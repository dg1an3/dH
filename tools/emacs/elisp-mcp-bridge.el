;;; elisp-mcp-bridge.el --- Introspection endpoint for the elisp MCP server -*- lexical-binding: t; -*-

;;; Commentary:

;; This file is loaded into the *live* Emacs session -- the one you are editing
;; in.  The MCP server (emacs/elisp-mcp-server.el, running as a separate
;; `emacs --batch' process) talks to it through `emacsclient --eval'.
;;
;; The split matters: the batch process owns the JSON-RPC conversation and
;; never evaluates anything on the model's behalf, while this file does all the
;; real work inside the session that actually has your buffers, your loaded
;; libraries and your state.
;;
;; Request/response travel via temp files rather than emacsclient's stdout.
;; emacsclient re-prints values through the Lisp reader, which mangles
;; multi-line strings and anything with embedded quotes -- and function source
;; is nothing but multi-line strings with embedded quotes.  Files sidestep the
;; problem entirely.

;;; Code:

(require 'find-func)
(require 'help-fns)
(require 'trace)
(require 'apropos)
(require 'pp)
(require 'subr-x)
(require 'cl-lib)

(defgroup elisp-mcp nil
  "Bridge exposing Emacs Lisp introspection to an MCP client."
  :group 'tools
  :prefix "elisp-mcp-")

(defcustom elisp-mcp-max-output 60000
  "Maximum characters returned for any single field.
Longer values are truncated with a marker, so a stray `(buffer-string)' on a
large buffer cannot flood the model's context."
  :type 'integer
  :group 'elisp-mcp)

(defcustom elisp-mcp-eval-enabled t
  "When nil, refuse `eval' and `trace' operations.
Introspection (source, describe, macroexpand, apropos) still works.  Set to nil
for a session where you want the model reading but not running code."
  :type 'boolean
  :group 'elisp-mcp)

(defvar elisp-mcp-log nil
  "List of (TIME OP . DETAIL) for operations served, newest first.")

(defcustom elisp-mcp-log-limit 200
  "How many entries of `elisp-mcp-log' to retain."
  :type 'integer
  :group 'elisp-mcp)


;;; ---------------------------------------------------------------------------
;;; Helpers

(defun elisp-mcp--truncate (s)
  "Truncate S to `elisp-mcp-max-output', noting how much was dropped."
  (let ((s (if (stringp s) s (format "%s" s))))
    (if (<= (length s) elisp-mcp-max-output)
        s
      (concat (substring s 0 elisp-mcp-max-output)
              (format "\n\n[... truncated, %d characters total]" (length s))))))

(defun elisp-mcp--sym (name)
  "Coerce NAME to a symbol."
  (cond ((symbolp name) name)
        ((stringp name) (intern name))
        (t (error "Not a symbol name: %S" name))))

(defun elisp-mcp--read-form (string)
  "Read a single Lisp form from STRING, erroring if it is not well-formed."
  (let* ((form (car (read-from-string string))))
    form))

(defun elisp-mcp--pp (object)
  "Pretty-print OBJECT to a string."
  (let ((print-level nil)
        (print-length nil)
        (print-circle t))
    (string-trim-right (pp-to-string object))))

(defun elisp-mcp--fn-type (sym)
  "Describe what kind of function SYM is."
  (let ((fn (and (fboundp sym) (indirect-function sym))))
    (cond ((null fn) "not a function")
          ((special-form-p fn) "special form")
          ((macrop sym) "macro")
          ((subrp fn) (if (native-comp-function-p fn)
                          "natively-compiled function"
                        "built-in function (C)"))
          ((byte-code-function-p fn) "byte-compiled function")
          ((and (consp fn) (eq (car fn) 'closure)) "interpreted closure")
          ((and (consp fn) (eq (car fn) 'lambda)) "interpreted lambda")
          ((functionp fn) "function")
          (t "unknown"))))

(defun elisp-mcp--arglist (sym)
  "Return SYM's argument list as a string, or nil."
  (condition-case nil
      (let ((args (help-function-arglist sym t)))
        (if args (format "%S" args) "()"))
    (error nil)))


;;; ---------------------------------------------------------------------------
;;; Operations
;;
;; Each returns an alist.  `elisp-mcp-bridge-serve' turns that into JSON.

(defun elisp-mcp-op-eval (params)
  "Evaluate the form in PARAMS, capturing value, printed output and errors."
  (unless elisp-mcp-eval-enabled
    (error "Evaluation is disabled (elisp-mcp-eval-enabled is nil)"))
  (let* ((code (cdr (assq 'form params)))
         (form (elisp-mcp--read-form code))
         (out (generate-new-buffer " *elisp-mcp-out*"))
         value err)
    (unwind-protect
        (let ((standard-output out))
          (condition-case e
              (setq value (eval form t))
            (error (setq err (error-message-string e)))))
      (setq out (prog1 (with-current-buffer out (buffer-string))
                  (kill-buffer out))))
    (append
     (list (cons 'form code)
           (cons 'output (elisp-mcp--truncate out)))
     (if err
         (list (cons 'error err))
       (list (cons 'value (elisp-mcp--truncate (elisp-mcp--pp value)))
             (cons 'value_type (symbol-name (type-of value))))))))

(defun elisp-mcp-op-source (params)
  "Return the defining source text for the symbol in PARAMS."
  (let* ((sym (elisp-mcp--sym (cdr (assq 'symbol params))))
         (type (elisp-mcp--fn-type sym)))
    (unless (or (fboundp sym) (boundp sym))
      (error "No such function or variable: %s" sym))
    (condition-case e
        (let* ((loc (if (fboundp sym)
                        (find-function-noselect sym t)
                      (find-variable-noselect sym)))
               (buf (car loc))
               (pos (cdr loc)))
          (unless (and buf pos)
            (error "No Lisp source available"))
          (with-current-buffer buf
            (save-excursion
              (goto-char pos)
              ;; Both finders land on the defining form's opening paren, so
              ;; point is already where we want it.  `beginning-of-defun-raw'
              ;; seeks a defun start strictly *before* point -- calling it
              ;; here would silently return the preceding top-level form,
              ;; with a line number to match.  Only back up if the finder put
              ;; us somewhere unexpected.
              (let* ((start (progn (unless (looking-at-p "\\s-*(")
                                     (beginning-of-defun-raw))
                                   (point)))
                     (end (progn (goto-char start) (end-of-defun) (point))))
                (list (cons 'symbol (symbol-name sym))
                      (cons 'type type)
                      (cons 'file (or (buffer-file-name) "<no file>"))
                      (cons 'line (line-number-at-pos start))
                      (cons 'source
                            (elisp-mcp--truncate
                             (buffer-substring-no-properties start end))))))))
      (error
       ;; Built-ins and functions defined by C, eval, or a macro with no
       ;; recorded location land here.  Say why, and give what we do have.
       (list (cons 'symbol (symbol-name sym))
             (cons 'type type)
             (cons 'source_unavailable (error-message-string e))
             (cons 'arglist (or (elisp-mcp--arglist sym) "unknown"))
             (cons 'documentation
                   (or (ignore-errors (documentation sym)) "none"))
             (cons 'definition
                   (elisp-mcp--truncate
                    (elisp-mcp--pp (and (fboundp sym)
                                        (indirect-function sym))))))))))

(defun elisp-mcp-op-describe (params)
  "Describe the symbol in PARAMS as both function and variable."
  (let* ((sym (elisp-mcp--sym (cdr (assq 'symbol params))))
         (res (list (cons 'symbol (symbol-name sym)))))
    (when (fboundp sym)
      (setq res (append res
                        (list (cons 'function_type (elisp-mcp--fn-type sym))
                              (cons 'arglist (or (elisp-mcp--arglist sym) "unknown"))
                              (cons 'function_doc
                                    (or (ignore-errors (documentation sym))
                                        "none"))
                              (cons 'defined_in
                                    (let ((f (ignore-errors (symbol-file sym 'defun))))
                                      (or f "unknown")))
                              (cons 'interactive
                                    (if (commandp sym) "yes" "no"))))))
    (when (boundp sym)
      (setq res (append res
                        (list (cons 'variable_doc
                                    (or (ignore-errors
                                          (documentation-property
                                           sym 'variable-documentation))
                                        "none"))
                              (cons 'value
                                    (elisp-mcp--truncate
                                     (elisp-mcp--pp (symbol-value sym))))
                              (cons 'buffer_local
                                    (if (local-variable-if-set-p sym) "yes" "no"))))))
    (unless (or (fboundp sym) (boundp sym))
      (setq res (append res (list (cons 'note "Symbol is neither bound nor fbound")))))
    res))

(defun elisp-mcp-op-macroexpand (params)
  "Macro-expand the form in PARAMS, one step or fully."
  (let* ((code (cdr (assq 'form params)))
         (all (cdr (assq 'all params)))
         (form (elisp-mcp--read-form code)))
    (list (cons 'form code)
          (cons 'mode (if all "macroexpand-all" "macroexpand-1"))
          (cons 'expansion
                (elisp-mcp--truncate
                 (elisp-mcp--pp (if all
                                    (macroexpand-all form)
                                  (macroexpand-1 form))))))))

(defun elisp-mcp-op-disassemble (params)
  "Disassemble the function in PARAMS."
  (let* ((sym (elisp-mcp--sym (cdr (assq 'symbol params)))))
    (unless (fboundp sym) (error "Not a function: %s" sym))
    (list (cons 'symbol (symbol-name sym))
          (cons 'type (elisp-mcp--fn-type sym))
          (cons 'disassembly
                (elisp-mcp--truncate
                 ;; `disassemble' writes to its BUFFER argument, not to
                 ;; `standard-output' -- binding the latter yields an empty
                 ;; string rather than an error.
                 (with-temp-buffer
                   (disassemble sym (current-buffer))
                   (buffer-string)))))))

(defun elisp-mcp-op-trace (params)
  "Trace functions while evaluating a form -- see what actually runs.
PARAMS has `symbols' (a vector or list of names) and `form'."
  (unless elisp-mcp-eval-enabled
    (error "Evaluation is disabled (elisp-mcp-eval-enabled is nil)"))
  (let* ((names (append (cdr (assq 'symbols params)) nil))
         (syms (mapcar #'elisp-mcp--sym names))
         (code (cdr (assq 'form params)))
         (form (elisp-mcp--read-form code))
         value err)
    (dolist (s syms)
      (unless (fboundp s) (error "Not a function: %s" s)))
    (let ((buf (get-buffer-create trace-buffer)))
      (with-current-buffer buf (erase-buffer))
      (unwind-protect
          (progn
            (dolist (s syms) (trace-function-background s))
            (condition-case e
                (setq value (eval form t))
              (error (setq err (error-message-string e)))))
        (dolist (s syms) (ignore-errors (untrace-function s))))
      (append
       (list (cons 'traced (mapconcat #'symbol-name syms ", "))
             (cons 'form code)
             (cons 'trace
                   (elisp-mcp--truncate
                    (with-current-buffer buf (buffer-string)))))
       (if err
           (list (cons 'error err))
         (list (cons 'value (elisp-mcp--truncate (elisp-mcp--pp value)))))))))

(defun elisp-mcp-op-apropos (params)
  "Search symbols matching a pattern in PARAMS."
  (let* ((pattern (cdr (assq 'pattern params)))
         (kind (or (cdr (assq 'kind params)) "any"))
         (limit (or (cdr (assq 'limit params)) 100))
         (hits '()))
    (mapatoms
     (lambda (s)
       (when (and (string-match-p pattern (symbol-name s))
                  (cond ((equal kind "function") (fboundp s))
                        ((equal kind "variable") (boundp s))
                        ((equal kind "command") (commandp s))
                        (t (or (fboundp s) (boundp s)))))
         (push s hits))))
    (setq hits (sort hits (lambda (a b) (string< (symbol-name a) (symbol-name b)))))
    (list (cons 'pattern pattern)
          (cons 'kind kind)
          (cons 'total (length hits))
          (cons 'matches
                (elisp-mcp--truncate
                 (mapconcat
                  (lambda (s)
                    (format "%s%s  %s"
                            (symbol-name s)
                            (if (fboundp s)
                                (format " %s" (or (elisp-mcp--arglist s) ""))
                              "")
                            (let ((d (or (and (fboundp s)
                                              (ignore-errors (documentation s)))
                                         (and (boundp s)
                                              (documentation-property
                                               s 'variable-documentation))
                                         "")))
                              (car (split-string (or d "") "\n")))))
                  (seq-take hits limit)
                  "\n"))))))

(defun elisp-mcp-op-session (params)
  "Report on the live session: buffers, modes, and optionally one buffer's text."
  (let ((want (cdr (assq 'buffer params))))
    (if want
        (let ((buf (get-buffer want)))
          (unless buf (error "No such buffer: %s" want))
          (with-current-buffer buf
            (list (cons 'buffer (buffer-name))
                  (cons 'file (or (buffer-file-name) "<none>"))
                  (cons 'major_mode (symbol-name major-mode))
                  (cons 'point (point))
                  (cons 'size (buffer-size))
                  (cons 'content
                        (elisp-mcp--truncate
                         (buffer-substring-no-properties
                          (point-min) (point-max)))))))
      (list (cons 'emacs_version emacs-version)
            (cons 'default_directory default-directory)
            (cons 'buffers
                  (mapconcat
                   (lambda (b)
                     (with-current-buffer b
                       (format "%-40s %-20s %s"
                               (buffer-name b)
                               (symbol-name major-mode)
                               (or (buffer-file-name b) ""))))
                   (seq-remove (lambda (b)
                                 (string-prefix-p " " (buffer-name b)))
                               (buffer-list))
                   "\n"))
            (cons 'features (format "%d libraries loaded" (length features)))))))

(defun elisp-mcp-op-load (params)
  "Load the file named in PARAMS into the live session."
  (unless elisp-mcp-eval-enabled
    (error "Evaluation is disabled (elisp-mcp-eval-enabled is nil)"))
  (let ((file (cdr (assq 'file params))))
    (load (expand-file-name file) nil t)
    (list (cons 'loaded (expand-file-name file))
          (cons 'status "ok"))))

(defconst elisp-mcp-operations
  '(("eval"         . elisp-mcp-op-eval)
    ("source"       . elisp-mcp-op-source)
    ("describe"     . elisp-mcp-op-describe)
    ("macroexpand"  . elisp-mcp-op-macroexpand)
    ("disassemble"  . elisp-mcp-op-disassemble)
    ("trace"        . elisp-mcp-op-trace)
    ("apropos"      . elisp-mcp-op-apropos)
    ("session"      . elisp-mcp-op-session)
    ("load"         . elisp-mcp-op-load))
  "Dispatch table mapping operation name to handler.")


;;; ---------------------------------------------------------------------------
;;; Entry point

;;;###autoload
(defun elisp-mcp-bridge-serve (request-file response-file)
  "Read a JSON request from REQUEST-FILE and write a JSON reply to RESPONSE-FILE.
Always writes a well-formed reply, including for internal errors, so the
server process never blocks waiting on a file that will not appear.
Returns the symbol `ok'."
  (let ((reply (make-hash-table :test 'equal)))
    (condition-case e
        (let* ((json (with-temp-buffer
                       (let ((coding-system-for-read 'utf-8-unix))
                         (insert-file-contents request-file))
                       (buffer-string)))
               (req (json-parse-string json :object-type 'alist
                                       :array-type 'list))
               (op (cdr (assq 'op req)))
               (params (cdr (assq 'params req)))
               (handler (cdr (assoc op elisp-mcp-operations))))
          (unless handler (error "Unknown operation: %s" op))
          (push (cons (current-time-string) op) elisp-mcp-log)
          (when (> (length elisp-mcp-log) elisp-mcp-log-limit)
            (setq elisp-mcp-log (seq-take elisp-mcp-log elisp-mcp-log-limit)))
          (let ((result (funcall handler params)))
            (puthash "ok" t reply)
            (puthash "result"
                     (let ((h (make-hash-table :test 'equal)))
                       (dolist (cell result h)
                         (puthash (symbol-name (car cell))
                                  (let ((v (cdr cell)))
                                    (cond ((stringp v) v)
                                          ((numberp v) v)
                                          ((eq v t) t)
                                          ((null v) :false)
                                          (t (format "%s" v))))
                                  h)))
                     reply)))
      (error
       (puthash "ok" :false reply)
       (puthash "error" (error-message-string e) reply)))
    ;; Pin the coding system.  Emacs error messages contain curly quotes
    ;; ("Symbol's function definition is void" uses U+2019), and writing
    ;; non-ASCII without an explicit coding system can raise a "Select coding
    ;; system" prompt.  In a daemon, whose only frame is not displayed
    ;; anywhere, that prompt is unanswerable and hangs the whole session --
    ;; every later client connection blocks behind it.
    (let ((coding-system-for-write 'utf-8-unix))
      (with-temp-file response-file
        (insert (json-serialize reply))))
    'ok))

(provide 'elisp-mcp-bridge)

;;; elisp-mcp-bridge.el ends here
