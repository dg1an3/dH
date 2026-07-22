;;; elisp-mcp-server.el --- MCP server exposing a live Emacs to an LLM -*- lexical-binding: t; -*-

;;; Commentary:

;; An MCP (Model Context Protocol) server, written in Emacs Lisp and run as a
;; batch Emacs process:
;;
;;   emacs --batch -Q -l emacs/elisp-mcp-server.el
;;
;; It speaks newline-delimited JSON-RPC 2.0 on stdin/stdout, and forwards every
;; request to your *live* Emacs session via `emacsclient', where
;; emacs/elisp-mcp-bridge.el does the actual work.
;;
;;   MCP client (Claude Code)
;;        | JSON-RPC over stdio
;;   this batch Emacs
;;        | emacsclient --eval
;;   your interactive Emacs   <- has your buffers, your config, your state
;;
;; Two rules govern this file:
;;
;;   1. stdout carries JSON-RPC and nothing else.  Anything diagnostic goes to
;;      stderr via `message', which batch Emacs already routes there.  A stray
;;      `princ' would corrupt the protocol stream.
;;
;;   2. This process never evaluates model-supplied code itself.  It only
;;      relays.  That keeps the trust boundary in one place -- the bridge --
;;      rather than smeared across both processes.

;;; Code:

(require 'json)
(require 'subr-x)

(defconst elisp-mcp-server-version "1.0.0")
(defconst elisp-mcp-server-protocol-version "2025-06-18"
  "MCP protocol version advertised when the client does not request one.")

(defvar elisp-mcp-server-server-file
  (or (getenv "ELISP_MCP_SERVER_FILE")
      (expand-file-name "server/server" user-emacs-directory))
  "Path to the Emacs server's authentication file.

Written by `server-start'.  Line 1 is \"HOST:PORT PID\", line 2 the auth key.

We talk to that socket directly rather than shelling out to emacsclient.
emacsclient.exe hangs indefinitely when invoked from a batch Emacs on Windows
-- it wants a console that a `call-process' child does not have -- while
`call-process' to any other program works fine.  Connecting to the socket
ourselves sidesteps that entirely, and avoids a process spawn per call.")

(defvar elisp-mcp-server-bridge
  (or (getenv "ELISP_MCP_BRIDGE")
      (expand-file-name "elisp-mcp-bridge.el"
                        (file-name-directory
                         (or load-file-name buffer-file-name default-directory))))
  "Path to elisp-mcp-bridge.el, loaded on demand into the live session.")

(defvar elisp-mcp-server-name (getenv "ELISP_MCP_SERVER_NAME")
  "Emacs server name to connect to, or nil for the default.")

(defvar elisp-mcp-server-timeout 30
  "Seconds to wait for the live Emacs to answer before giving up.")


;;; ---------------------------------------------------------------------------
;;; Talking to the live Emacs

(defun elisp-mcp-server--elisp-string (s)
  "Render S as an Emacs Lisp string literal, safe to embed in a form."
  (format "\"%s\"" (replace-regexp-in-string
                    "[\\\"]" "\\\\\\&"
                    ;; Backslashes in Windows paths would otherwise be read as
                    ;; escapes; forward slashes work fine on Windows Emacs.
                    (replace-regexp-in-string "\\\\" "/" s))))

(defun elisp-mcp-server--quote-arg (arg)
  "Apply the Emacs server protocol's &-quoting to ARG.
Mirrors `server-quote-arg': the server splits a request on spaces and
unquotes each field, so every field -- including the auth key, which may
itself contain `&' or `-' -- must be quoted before being sent."
  (replace-regexp-in-string
   "[-&\n ]"
   (lambda (s) (pcase (aref s 0) (?& "&&") (?- "&-") (?\n "&n") (_ "&_")))
   arg t t))

(defun elisp-mcp-server--unquote-arg (arg)
  "Undo the server protocol's &-quoting in ARG.  Mirrors `server-unquote-arg'."
  (replace-regexp-in-string
   "&." (lambda (s) (pcase (aref s 1) (?& "&") (?- "-") (?n "\n") (_ " ")))
   arg t t))

(defun elisp-mcp-server--server-endpoint ()
  "Return (HOST PORT AUTH) parsed from the Emacs server file.
Signals a user-facing error if the file is missing or malformed."
  (unless (file-exists-p elisp-mcp-server-server-file)
    (error "No Emacs server file at %s -- the live Emacs is not running a server. Add (server-start) to your init, or M-x server-start"
           elisp-mcp-server-server-file))
  (with-temp-buffer
    (insert-file-contents elisp-mcp-server-server-file)
    (goto-char (point-min))
    (let ((first (buffer-substring-no-properties
                  (point) (line-end-position)))
          auth)
      (forward-line 1)
      (setq auth (buffer-substring-no-properties
                  (point) (line-end-position)))
      (unless (string-match "\\`\\(.+\\):\\([0-9]+\\)[ \t]" first)
        (error "Malformed Emacs server file: %s" first))
      (list (match-string 1 first)
            (string-to-number (match-string 2 first))
            auth))))

(defun elisp-mcp-server--eval-remote (form-string)
  "Evaluate FORM-STRING in the live Emacs over the server socket.
Return the printed result as a string.  Signals on connection or protocol
failure."
  (pcase-let* ((`(,host ,port ,auth) (elisp-mcp-server--server-endpoint))
               (buf (generate-new-buffer " *elisp-mcp-conn*"))
               (proc nil))
    (unwind-protect
        (progn
          (setq proc (open-network-stream "elisp-mcp" buf host port
                                          :type 'plain
                                          :coding 'utf-8-unix))
          (set-process-query-on-exit-flag proc nil)
          ;; The auth key goes RAW, not quoted.  `server--process-filter-1'
          ;; checks it with a plain `string-match' of "-auth \\([!-~]+\\)"
          ;; against the undecoded request, before any &-unquoting happens, so
          ;; a quoted key never matches.  Getting this wrong is expensive: the
          ;; failure path calls `sit-for' inside a process filter, which wedges
          ;; the whole daemon for anyone else trying to connect.
          (process-send-string
           proc (format "-auth %s -eval %s\n"
                        auth
                        (elisp-mcp-server--quote-arg form-string)))
          ;; Wait for the server's reply, or for it to hang up.  The bridge
          ;; writes its real payload to a file and returns a short token, so
          ;; a single -print line is all we expect here.
          (let ((deadline (+ (float-time) elisp-mcp-server-timeout))
                (done nil))
            (while (and (not done) (< (float-time) deadline))
              (accept-process-output proc 0.1)
              (with-current-buffer buf
                (when (or (save-excursion
                            (goto-char (point-min))
                            (re-search-forward "^-\\(print\\|error\\) " nil t))
                          (not (process-live-p proc)))
                  (setq done t))))
            (unless done
              (error "Timed out after %ss waiting for the live Emacs"
                     elisp-mcp-server-timeout)))
          (with-current-buffer buf
            (goto-char (point-min))
            (cond
             ((re-search-forward "^-error \\(.*\\)$" nil t)
              (error "Live Emacs reported: %s"
                     (elisp-mcp-server--unquote-arg (match-string 1))))
             ((progn (goto-char (point-min))
                     (re-search-forward "^-print \\(.*\\)$" nil t))
              (elisp-mcp-server--unquote-arg (match-string 1)))
             (t ""))))
      (when (and proc (process-live-p proc)) (delete-process proc))
      (when (buffer-live-p buf) (kill-buffer buf)))))

(defun elisp-mcp-server--call (op params)
  "Ask the live Emacs to run OP with PARAMS.  Return a result alist.

PARAMS is an alist; it is serialized to JSON and handed over in a temp file.
The reply comes back in a second temp file.  Going through files rather than
emacsclient's stdout avoids a real problem: emacsclient prints return values
through the Lisp reader, which mangles multi-line strings -- and source code
is nothing but multi-line strings."
  (let* ((req (make-temp-file "elisp-mcp-req" nil ".json"))
         (resp (make-temp-file "elisp-mcp-resp" nil ".json"))
         (payload (json-serialize
                   (let ((h (make-hash-table :test 'equal)))
                     (puthash "op" op h)
                     (puthash "params"
                              (let ((p (make-hash-table :test 'equal)))
                                ;; Keys arrive as symbols from JSON decoding;
                                ;; `json-serialize' insists on strings.
                                (dolist (cell params p)
                                  (puthash (elisp-mcp-server--key (car cell))
                                           (elisp-mcp-server--jsonify (cdr cell))
                                           p)))
                              h)
                     h))))
    (unwind-protect
        (progn
          (with-temp-file req (insert payload))
          (let ((form
                 (format "(progn (unless (featurep 'elisp-mcp-bridge) (load %s nil t)) (elisp-mcp-bridge-serve %s %s))"
                         (elisp-mcp-server--elisp-string elisp-mcp-server-bridge)
                         (elisp-mcp-server--elisp-string req)
                         (elisp-mcp-server--elisp-string resp))))
            (condition-case e
                (progn
                  (elisp-mcp-server--eval-remote form)
                  (if (not (file-exists-p resp))
                      (list (cons 'ok nil)
                            (cons 'error "The live Emacs produced no response file."))
                    (let ((raw (with-temp-buffer
                                 (insert-file-contents resp)
                                 (buffer-string))))
                      (if (string-empty-p (string-trim raw))
                          (list (cons 'ok nil)
                                (cons 'error "Empty response from the live Emacs."))
                        (json-parse-string raw :object-type 'alist
                                           :array-type 'list)))))
              (error
               (list (cons 'ok nil)
                     (cons 'error (error-message-string e)))))))
      (ignore-errors (delete-file req))
      (ignore-errors (delete-file resp)))))


;;; ---------------------------------------------------------------------------
;;; Tool definitions

(defun elisp-mcp-server--jsonify (value)
  "Recursively convert alists in VALUE into hash tables with string keys.

`json-serialize' will not take an alist whose keys are strings, nor a hash
table whose keys are symbols -- and JSON decoding here produces exactly those
shapes.  Normalising in one place keeps that mismatch from having to be
remembered at every call site."
  (cond
   ;; An alist: a cons whose car is itself a cons.
   ((and (consp value) (consp (car value)))
    (let ((h (make-hash-table :test 'equal)))
      (dolist (cell value h)
        (puthash (elisp-mcp-server--key (car cell))
                 (elisp-mcp-server--jsonify (cdr cell))
                 h))))
   (t value)))

(defun elisp-mcp-server--key (k)
  "Return K as a string suitable for a JSON object key."
  (cond ((symbolp k) (symbol-name k))
        ((stringp k) k)
        (t (format "%s" k))))

(defun elisp-mcp-server--schema (props required)
  "Build a JSON Schema object from PROPS and REQUIRED."
  (let ((h (make-hash-table :test 'equal)))
    (puthash "type" "object" h)
    (puthash "properties"
             (let ((p (make-hash-table :test 'equal)))
               (dolist (spec props p)
                 (puthash (car spec)
                          (elisp-mcp-server--jsonify (cdr spec))
                          p)))
             h)
    (puthash "required" (or required []) h)
    h))

(defconst elisp-mcp-server-tools
  `(("elisp_eval"
     "Evaluate an Emacs Lisp form in the live Emacs session and return its value, any printed output, and any error. State persists between calls -- this is the same Emacs the user is editing in. Use this to run code; use elisp_source to read code."
     ,(elisp-mcp-server--schema
       '(("form" . (("type" . "string")
                    ("description" . "A single Emacs Lisp form, e.g. (+ 1 2) or (buffer-name)"))))
       ["form"])
     "eval")

    ("elisp_source"
     "Read the actual source code of a function or variable: the defining form, its file, and its line number. Prefer this over guessing at an implementation. For built-ins with no Lisp source, returns the signature, docstring and internal definition instead."
     ,(elisp-mcp-server--schema
       '(("symbol" . (("type" . "string")
                      ("description" . "Name of the function or variable, e.g. brimstone-build"))))
       ["symbol"])
     "source")

    ("elisp_describe"
     "Describe a symbol as both function and variable: signature, docstring, defining file, current value, whether it is interactive or buffer-local. The fast way to orient before reading source."
     ,(elisp-mcp-server--schema
       '(("symbol" . (("type" . "string")
                      ("description" . "Symbol name"))))
       ["symbol"])
     "describe")

    ("elisp_macroexpand"
     "Expand a macro form to see what code it actually produces. Essential for reasoning about anything built on macros -- cl-lib, use-package, pcase, or a project's own macros -- where the surface syntax hides the real control flow."
     ,(elisp-mcp-server--schema
       '(("form" . (("type" . "string")
                    ("description" . "Form to expand, e.g. (cl-loop for x in l collect x)")))
         ("all" . (("type" . "boolean")
                   ("description" . "Expand fully and recursively (macroexpand-all) rather than one step. Default false."))))
       ["form"])
     "macroexpand")

    ("elisp_disassemble"
     "Disassemble a function to byte-code. Use when reasoning about what a function really does at execution level, or about performance."
     ,(elisp-mcp-server--schema
       '(("symbol" . (("type" . "string")
                      ("description" . "Function name"))))
       ["symbol"])
     "disassemble")

    ("elisp_trace"
     "Trace one or more functions while evaluating a form, then return the call trace: every invocation with its arguments and return value, nested by depth. This is the strongest tool for understanding how code actually runs -- what gets called, in what order, with what values. Reach for it instead of speculating about control flow."
     ,(elisp-mcp-server--schema
       '(("symbols" . (("type" . "array")
                       ("items" . (("type" . "string")))
                       ("description" . "Function names to trace")))
         ("form" . (("type" . "string")
                    ("description" . "Form to evaluate while tracing is active"))))
       ["symbols" "form"])
     "trace")

    ("elisp_apropos"
     "Search all symbols in the live session by regexp, returning names, signatures and one-line docs. Use to discover what exists before assuming a function's name."
     ,(elisp-mcp-server--schema
       '(("pattern" . (("type" . "string")
                       ("description" . "Regexp matched against symbol names, e.g. \"^brimstone-\"")))
         ("kind" . (("type" . "string")
                    ("enum" . ["any" "function" "variable" "command"])
                    ("description" . "Restrict results. Default any.")))
         ("limit" . (("type" . "integer")
                     ("description" . "Maximum matches to return. Default 100."))))
       ["pattern"])
     "apropos")

    ("elisp_session"
     "Inspect the live session: with no argument, list buffers with their major modes and files; with a buffer name, return that buffer's contents, mode and point."
     ,(elisp-mcp-server--schema
       '(("buffer" . (("type" . "string")
                      ("description" . "Buffer name to read. Omit to list all buffers."))))
       [])
     "session")

    ("elisp_load"
     "Load an Emacs Lisp file into the live session, so edits to it take effect without restarting Emacs."
     ,(elisp-mcp-server--schema
       '(("file" . (("type" . "string")
                    ("description" . "Path to a .el file"))))
       ["file"])
     "load"))
  "Tool definitions: (NAME DESCRIPTION SCHEMA BRIDGE-OP).")

(defun elisp-mcp-server--tools-list ()
  "Return the MCP tools/list payload."
  (let ((h (make-hash-table :test 'equal)))
    ;; `json-serialize' reads a Lisp list as an alist, so JSON arrays must be
    ;; vectors -- hence `vconcat' here and around every content block below.
    (puthash "tools"
             (vconcat
              (mapcar (lambda (tool)
                        (let ((t- (make-hash-table :test 'equal)))
                          (puthash "name" (nth 0 tool) t-)
                          (puthash "description" (nth 1 tool) t-)
                          (puthash "inputSchema" (nth 2 tool) t-)
                          t-))
                      elisp-mcp-server-tools))
             h)
    h))

(defun elisp-mcp-server--format-result (result)
  "Render RESULT, an alist from the bridge, as readable text for the model."
  (mapconcat
   (lambda (cell)
     (let ((key (symbol-name (car cell)))
           (val (cdr cell)))
       (cond
        ((eq val :false) (format "%s: false" key))
        ((eq val t) (format "%s: true" key))
        ;; Multi-line fields read better as labelled blocks than as key: value.
        ((and (stringp val) (string-match-p "\n" val))
         (format "%s:\n%s" key val))
        (t (format "%s: %s" key val)))))
   result
   "\n"))

(defun elisp-mcp-server--call-tool (name arguments)
  "Invoke tool NAME with ARGUMENTS, returning the MCP tools/call payload."
  (let* ((tool (seq-find (lambda (tl) (equal (nth 0 tl) name))
                         elisp-mcp-server-tools))
         (h (make-hash-table :test 'equal)))
    (if (not tool)
        (progn
          (puthash "isError" t h)
          (puthash "content"
                   (vector (elisp-mcp-server--text
                            (format "Unknown tool: %s" name)))
                   h))
      (let* ((response (elisp-mcp-server--call (nth 3 tool) arguments))
             (ok (cdr (assq 'ok response))))
        (if (and ok (not (eq ok :false)))
            (puthash "content"
                     (vector (elisp-mcp-server--text
                              (elisp-mcp-server--format-result
                               (cdr (assq 'result response)))))
                     h)
          (puthash "isError" t h)
          (puthash "content"
                   (vector (elisp-mcp-server--text
                            (or (cdr (assq 'error response))
                                "Unknown error")))
                   h))))
    h))

(defun elisp-mcp-server--text (s)
  "Wrap S as an MCP text content block."
  (let ((h (make-hash-table :test 'equal)))
    (puthash "type" "text" h)
    (puthash "text" s h)
    h))


;;; ---------------------------------------------------------------------------
;;; JSON-RPC plumbing

(defun elisp-mcp-server--send (obj)
  "Write OBJ to stdout as one line of JSON.
`send-string-to-terminal' is used rather than `princ' because it writes
unbuffered, which matters for a protocol the client is blocking on."
  (send-string-to-terminal (concat (json-serialize obj) "\n")))

(defun elisp-mcp-server--reply (id result)
  "Send a JSON-RPC success reply carrying RESULT for ID."
  (let ((h (make-hash-table :test 'equal)))
    (puthash "jsonrpc" "2.0" h)
    (puthash "id" id h)
    (puthash "result" result h)
    (elisp-mcp-server--send h)))

(defun elisp-mcp-server--error (id code message)
  "Send a JSON-RPC error reply for ID with CODE and MESSAGE."
  (let ((h (make-hash-table :test 'equal))
        (e (make-hash-table :test 'equal)))
    (puthash "code" code e)
    (puthash "message" message e)
    (puthash "jsonrpc" "2.0" h)
    (puthash "id" id h)
    (puthash "error" e h)
    (elisp-mcp-server--send h)))

(defun elisp-mcp-server--handle (req)
  "Dispatch a single decoded JSON-RPC request REQ."
  (let* ((id (cdr (assq 'id req)))
         (method (cdr (assq 'method req)))
         (params (cdr (assq 'params req))))
    (cond
     ;; Notifications carry no id and must never be answered.
     ((null id)
      nil)

     ((equal method "initialize")
      (let ((h (make-hash-table :test 'equal))
            (caps (make-hash-table :test 'equal))
            (info (make-hash-table :test 'equal))
            (requested (cdr (assq 'protocolVersion params))))
        (puthash "tools" (make-hash-table :test 'equal) caps)
        (puthash "name" "emacs-elisp" info)
        (puthash "version" elisp-mcp-server-version info)
        ;; Echo the client's protocol version when it names one; disagreeing
        ;; here is a common cause of silent handshake failure.
        (puthash "protocolVersion"
                 (or requested elisp-mcp-server-protocol-version) h)
        (puthash "capabilities" caps h)
        (puthash "serverInfo" info h)
        (elisp-mcp-server--reply id h)))

     ((equal method "tools/list")
      (elisp-mcp-server--reply id (elisp-mcp-server--tools-list)))

     ((equal method "tools/call")
      (let ((name (cdr (assq 'name params)))
            (args (cdr (assq 'arguments params))))
        (elisp-mcp-server--reply
         id (elisp-mcp-server--call-tool name args))))

     ((equal method "ping")
      (elisp-mcp-server--reply id (make-hash-table :test 'equal)))

     ((member method '("resources/list" "prompts/list"))
      ;; Declared unsupported in capabilities, but some clients probe anyway.
      (let ((h (make-hash-table :test 'equal)))
        (puthash (if (equal method "resources/list") "resources" "prompts")
                 [] h)
        (elisp-mcp-server--reply id h)))

     (t
      (elisp-mcp-server--error id -32601 (format "Method not found: %s" method))))))

(defun elisp-mcp-server--strip-bom (line)
  "Remove a leading byte-order mark or whitespace from LINE.
Some launchers prepend a BOM to the first line written to a pipe, which would
otherwise make the initialize request unparseable."
  (string-trim-left line "[﻿￾[:space:]]+"))

(defun elisp-mcp-server-run ()
  "Read JSON-RPC requests from stdin until EOF."
  (let (line)
    (while (setq line (condition-case nil (read-string "") (error nil)))
      (setq line (elisp-mcp-server--strip-bom line))
      (unless (string-empty-p line)
        (condition-case e
            (elisp-mcp-server--handle
             ;; Arrays stay vectors: a decoded JSON array as a Lisp list would
             ;; be re-encoded as an alist when relayed onward, and fail.
             (json-parse-string line :object-type 'alist :array-type 'array))
          (error
           ;; Never die on one bad message; log to stderr and keep serving.
           (message "elisp-mcp-server: %s" (error-message-string e))
           (ignore-errors
             (elisp-mcp-server--error
              :null -32700 (error-message-string e)))))))))

;; Running as `emacs --batch -l this-file' starts the server immediately.
;; `noninteractive' is nil when the file is merely loaded for inspection.
;; ELISP_MCP_NO_AUTOSTART suppresses that so the individual functions can be
;; loaded and exercised from a test harness.
(when (and noninteractive (not (getenv "ELISP_MCP_NO_AUTOSTART")))
  (elisp-mcp-server-run))

(provide 'elisp-mcp-server)

;;; elisp-mcp-server.el ends here
