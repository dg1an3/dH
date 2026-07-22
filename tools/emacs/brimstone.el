;;; brimstone.el --- Build and debug support for the dH/Brimstone tree -*- lexical-binding: t; -*-

;; Copyright (c) 2007-2021, Derek G. Lane. All rights reserved.

;;; Commentary:

;; Project-local Emacs support for building `Brimstone_src.sln' with MSBuild
;; and debugging the resulting binaries with cdb.exe (the Windows SDK console
;; debugger), which is the only debugger on this machine that reads MSVC PDBs.
;;
;; The host is Windows on ARM64, so ARM64 is the default target platform.
;;
;; Entry points:
;;   M-x brimstone-build          build the solution (compilation-mode)
;;   M-x brimstone-build-project  build one project
;;   M-x brimstone-rebuild        clean + build
;;   M-x brimstone-clean          clean
;;   M-x brimstone-cdb            start cdb under gud
;;
;; See emacs/README.md for keybindings.

;;; Code:

(require 'compile)
(require 'gud)
(require 'subr-x)

(defgroup brimstone nil
  "Build and debug support for the Brimstone radiotherapy planning tree."
  :group 'tools
  :prefix "brimstone-")


;;; ---------------------------------------------------------------------------
;;; Locations

(defcustom brimstone-msbuild
  "C:/Program Files/Microsoft Visual Studio/18/Community/MSBuild/Current/Bin/arm64/MSBuild.exe"
  "Absolute path to MSBuild.exe.
The arm64 build is used so MSBuild itself runs natively rather than emulated."
  :type 'file
  :group 'brimstone)

(defcustom brimstone-cdb
  "C:/Program Files (x86)/Windows Kits/10/Debuggers/arm64/cdb.exe"
  "Absolute path to cdb.exe, the Windows SDK console debugger.
This is installed by the Windows SDK feature `OptionId.WindowsDesktopDebuggers'."
  :type 'file
  :group 'brimstone)

(defcustom brimstone-solution "Brimstone_src.sln"
  "Solution file, relative to the project root."
  :type 'string
  :group 'brimstone)

(defcustom brimstone-configuration "Debug"
  "MSBuild Configuration to build.  Debug or Release."
  :type '(choice (const "Debug") (const "Release") string)
  :group 'brimstone)

(defcustom brimstone-platform "ARM64"
  "MSBuild Platform to build.
ARM64 is the native platform here; the x64 configurations are known to be
incomplete (they link no ITK or DCMTK).  See CLAUDE.md."
  :type '(choice (const "ARM64") (const "x64") (const "Win32") string)
  :group 'brimstone)

(defcustom brimstone-msbuild-extra-args '("/m" "/nologo")
  "Extra arguments always passed to MSBuild.
`/m' enables parallel project builds; `/nologo' suppresses the banner."
  :type '(repeat string)
  :group 'brimstone)

(defun brimstone-root ()
  "Return the project root directory, or nil if it cannot be located.
Looks upward for the directory containing `brimstone-solution'."
  (when-let* ((dir (locate-dominating-file
                    (or buffer-file-name default-directory)
                    brimstone-solution)))
    (expand-file-name dir)))

(defun brimstone--root-or-error ()
  "Return the project root, signalling an error if it cannot be found."
  (or (brimstone-root)
      (user-error "Cannot find %s above %s"
                  brimstone-solution default-directory)))


;;; ---------------------------------------------------------------------------
;;; Building

;; Emacs knows how to parse MSVC diagnostics, but the `msft' rule is not
;; enabled by default.  Without it, C2065 and friends are not clickable.
(add-to-list 'compilation-error-regexp-alist 'msft)

;; MSBuild prefixes diagnostics with a project number ("3>foo.cpp(10,4): error
;; C2065: ...") under /m.  The stock `msft' rule already tolerates that prefix.

(defun brimstone--build-command (target &optional project)
  "Return the MSBuild command line for TARGET, optionally scoped to PROJECT."
  (string-join
   (append
    (list (shell-quote-argument brimstone-msbuild)
          (shell-quote-argument brimstone-solution))
    (when target
      (list (concat "/t:" (if project
                              (concat (brimstone--msbuild-target-name project)
                                      ":" target)
                            target))))
    (list (concat "/p:Configuration=" brimstone-configuration)
          (concat "/p:Platform=" brimstone-platform)
          ;; Emacs cannot drive MSBuild's terminal logger; force plain output.
          "/v:minimal"
          "-tl:off")
    brimstone-msbuild-extra-args)
   " "))

(defun brimstone--msbuild-target-name (project)
  "Escape PROJECT for use inside an MSBuild /t: target specification.
MSBuild uses `.' as a target-path separator, so dots must be escaped."
  (replace-regexp-in-string "\\." "_" project))

(defun brimstone--compile (command)
  "Run COMMAND in `compilation-mode' from the project root."
  (let ((default-directory (brimstone--root-or-error)))
    (compile command)))

;;;###autoload
(defun brimstone-build ()
  "Build the whole solution."
  (interactive)
  (brimstone--compile (brimstone--build-command "Build")))

;;;###autoload
(defun brimstone-rebuild ()
  "Clean and rebuild the whole solution."
  (interactive)
  (brimstone--compile (brimstone--build-command "Rebuild")))

;;;###autoload
(defun brimstone-clean ()
  "Clean the solution."
  (interactive)
  (brimstone--compile (brimstone--build-command "Clean")))

(defconst brimstone-projects '("RtModel" "Graph" "Brimstone")
  "Projects in `Brimstone_src.sln', in dependency order.")

;;;###autoload
(defun brimstone-build-project (project)
  "Build a single PROJECT from the solution."
  (interactive (list (completing-read "Project: " brimstone-projects nil t)))
  (brimstone--compile (brimstone--build-command "Build" project)))

;;;###autoload
(defun brimstone-set-configuration (config platform)
  "Set the build CONFIG and PLATFORM for subsequent builds."
  (interactive
   (list (completing-read "Configuration: " '("Debug" "Release") nil t)
         (completing-read "Platform: " '("ARM64" "x64" "Win32") nil t)))
  (setq brimstone-configuration config
        brimstone-platform platform)
  (message "Brimstone: building %s|%s" config platform))


;;; ---------------------------------------------------------------------------
;;; Debugging with cdb
;;
;; Emacs' gud.el ships support for gdb, dbx, xdb, sdb, jdb, pdb and perldb --
;; but not cdb.  What follows is a gud "driver" for cdb: a marker filter that
;; recognises cdb's source-position announcements and feeds them to gud so the
;; overlay arrow tracks execution, plus the usual stepping commands bound to
;; cdb's command set.

(defcustom brimstone-cdb-symbol-path
  "srv*C:\\Symbols*https://msdl.microsoft.com/download/symbols"
  "Symbol path handed to cdb via `-y'.
The default caches the Microsoft public symbol server under C:\\Symbols, which
makes call stacks through system DLLs readable.  Set to just a local path to
work entirely offline."
  :type 'string
  :group 'brimstone)

(defcustom brimstone-cdb-use-symbol-server t
  "When non-nil, pass `brimstone-cdb-symbol-path' to cdb.
Disable to avoid all network access during debugging."
  :type 'boolean
  :group 'brimstone)

;; cdb announces a source position whenever it stops, in the form:
;;
;;   RtModel!KLDivTerm::Eval+0x2a [c:\...\rtmodel\kldivterm.cpp @ 142]:
;;
;; and, for `.lines'-resolved frames without a symbol offset, as a bare:
;;
;;   c:\...\rtmodel\kldivterm.cpp(142)
;;
;; Both forms are matched below.  The bracketed form is by far the common one.
(defconst brimstone-cdb-position-regexp
  (rx "["
      (group (one-or-more (not (any "]" "\n" "@"))))
      " @ "
      (group (one-or-more digit))
      "]")
  "Regexp matching cdb's bracketed `[file @ line]' source annotation.")

(defconst brimstone-cdb-position-regexp-alt
  (rx bol
      (group (any "a-zA-Z") ":" (one-or-more (not (any "(" ")" "\n"))))
      "("
      (group (one-or-more digit))
      ")")
  "Regexp matching cdb's alternate `file(line)' source annotation.")

(defvar brimstone-cdb--partial ""
  "Buffered tail of cdb output that may contain a split source annotation.")

(defun brimstone-cdb--scan-position (string)
  "Scan STRING for a cdb source annotation and update `gud-last-frame'.
Returns nothing useful; called for effect."
  (let ((start 0))
    ;; Use the *last* match in the chunk: when cdb prints a stack or several
    ;; steps at once, the final annotation is the current position.
    (while (string-match brimstone-cdb-position-regexp string start)
      (setq gud-last-frame
            (cons (match-string 1 string)
                  (string-to-number (match-string 2 string))))
      (setq start (match-end 0)))
    (when (zerop start)
      (setq start 0)
      (while (string-match brimstone-cdb-position-regexp-alt string start)
        (setq gud-last-frame
              (cons (match-string 1 string)
                    (string-to-number (match-string 2 string))))
        (setq start (match-end 0))))))

(defun brimstone-cdb-marker-filter (string)
  "Process cdb output STRING for gud, returning the text to display.
cdb output is passed through unmodified; the annotations it contains are
genuinely useful to read, so unlike gdb's marker filter this one does not
strip what it matches."
  ;; Hold back a trailing partial line so a source annotation split across two
  ;; process chunks is still matched.
  (let* ((text (concat brimstone-cdb--partial string))
         (nl (string-match-p "\n[^\n]*\\'" text)))
    (if nl
        (setq brimstone-cdb--partial (substring text (1+ nl))
              text (substring text 0 (1+ nl)))
      ;; No newline at all -- keep buffering unless it is getting silly.
      (if (> (length text) 4096)
          (setq brimstone-cdb--partial "")
        (setq brimstone-cdb--partial text
              text "")))
    (brimstone-cdb--scan-position (concat text brimstone-cdb--partial))
    ;; Return everything, including the held-back tail, so the prompt appears.
    (prog1 (concat text brimstone-cdb--partial)
      (setq brimstone-cdb--partial ""))))

(defun brimstone--output-dir ()
  "Return the directory holding build output for the current configuration."
  (expand-file-name (format "%s/%s/" brimstone-platform brimstone-configuration)
                    (brimstone--root-or-error)))

(defun brimstone--default-exe ()
  "Return a plausible default executable to debug, or nil."
  (let ((exe (expand-file-name "Brimstone.exe" (brimstone--output-dir))))
    (and (file-exists-p exe) exe)))

;;;###autoload
(defun brimstone-cdb (command-line)
  "Run cdb on COMMAND-LINE, with gud source tracking.

The overlay arrow follows execution in the source buffer.  cdb's own command
set applies -- `p' step over, `t' step into, `gu' step out, `g' go, `k' stack,
`bp' breakpoint, `dv' locals, `?? expr' evaluate -- and the gud keybindings
below are bound to those commands."
  (interactive
   (list (gud-query-cmdline
          'brimstone-cdb
          (or (brimstone--default-exe)
              (expand-file-name "Brimstone.exe" (brimstone--output-dir))))))
  (unless (file-exists-p brimstone-cdb)
    (user-error "cdb.exe not found at %s -- install the Windows SDK \
\"Debugging Tools for Windows\" feature" brimstone-cdb))
  (setq brimstone-cdb--partial "")
  ;; `gud-query-cmdline' hands back "<program> <args>"; cdb wants the program
  ;; last, after its own switches, so rebuild the line here.
  (let* ((root (brimstone--root-or-error))
         (switches (append
                    ;; -o  debug child processes too (the optimizer spawns none
                    ;;     today, but OptThread may in future)
                    ;; -g  ignore the initial breakpoint
                    ;; -srcpath  so cdb resolves source next to the PDB
                    (list "-lines" "-srcpath" (expand-file-name root))
                    (when brimstone-cdb-use-symbol-server
                      (list "-y" brimstone-cdb-symbol-path))))
         (full (string-join
                (append (list (shell-quote-argument brimstone-cdb))
                        switches
                        (list command-line))
                " ")))
    (gud-common-init full nil 'brimstone-cdb-marker-filter))

  (setq-local gud-minor-mode 'brimstone-cdb)

  ;; cdb's stepping commands.  `f' is not a cdb command; `gu' (go up) is the
  ;; step-out equivalent.
  (gud-def gud-break "bp `%d%f:%l`" "\C-b" "Set breakpoint at current line.")
  (gud-def gud-remove "bc *"        "\C-d" "Clear ALL breakpoints.")
  (gud-def gud-step   "t"           "\C-s" "Step one source line, into calls.")
  (gud-def gud-next   "p"           "\C-n" "Step one source line, over calls.")
  (gud-def gud-finish "gu"          "\C-f" "Run until the current frame returns.")
  (gud-def gud-cont   "g"           "\C-r" "Continue.")
  (gud-def gud-stepi  "th"          "\C-t" "Step one instruction.")
  (gud-def gud-print  "?? %e"       "\C-p" "Evaluate C++ expression at point.")
  (gud-def gud-where  "k"           "\C-w" "Show the call stack.")

  (setq comint-prompt-regexp "^[0-9]+:[0-9]+\\(:x86\\)?> ")
  (setq paragraph-start comint-prompt-regexp)
  (run-hooks 'brimstone-cdb-mode-hook))

(defvar brimstone-cdb-mode-hook nil
  "Hook run when a cdb session starts.")

;;;###autoload
(defun brimstone-cdb-attach (process-name)
  "Attach cdb to a running process named PROCESS-NAME.
Useful for catching the optimizer mid-run without restarting the GUI."
  (interactive "sProcess name (e.g. Brimstone.exe): ")
  (brimstone-cdb (concat "-pn " process-name)))


;;; ---------------------------------------------------------------------------
;;; Code navigation

;;;###autoload
(defun brimstone-regenerate-compile-commands ()
  "Regenerate compile_commands.json so clangd sees current build flags.
Run this after changing include paths, defines, or the source file list in
any .vcxproj."
  (interactive)
  (let ((default-directory (brimstone--root-or-error)))
    (compile (string-join
              (list "powershell" "-NoProfile" "-ExecutionPolicy" "Bypass"
                    "-File" (shell-quote-argument
                             (expand-file-name "emacs/gen-compile-commands.ps1"
                                               default-directory))
                    "-Configuration" brimstone-configuration
                    "-Platform" brimstone-platform)
              " "))))

(provide 'brimstone)

;;; brimstone.el ends here
