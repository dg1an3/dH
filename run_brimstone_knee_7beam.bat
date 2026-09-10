@echo off
REM ==================================================================
REM  Brimstone -- separable entropy at the knee (W = 2e-4), 7-beam plan
REM  Fully automated: launches the app, imports the DICOM micro series,
REM  builds a 7-beam plan, sets the 5-structure prescription, and runs
REM  the optimizer -- then leaves the window open for DVH inspection.
REM
REM  Entropy/knee params below are read by the C++ (Prescription.cpp,
REM  OptThread.cpp). BRIMSTONE_BEAM_COUNT is read by the pywinauto
REM  driver. Requires the 3.11 venv that has pywinauto installed.
REM ==================================================================
setlocal

REM --- regularizer configuration -----------------------------------
set "BRIMSTONE_ENTROPY_SEPARABLE=1"
set "BRIMSTONE_ENTROPY_WEIGHT=2e-4"
set "BRIMSTONE_DETERMINISTIC=1"
set "BRIMSTONE_BEAM_COUNT=7"

REM --- write the converged objective next to this .bat -------------
set "BRIMSTONE_OBJECTIVE_FILE=%~dp0knee_run_7beam.txt"
if exist "%BRIMSTONE_OBJECTIVE_FILE%" del "%BRIMSTONE_OBJECTIVE_FILE%"

REM --- interpreter + driver ---------------------------------------
set "PY=C:\Users\Derek\.pyenv\pyenv-win\versions\3.11.9\python.exe"
set "DRIVER=C:\dev_dH_and_friends\dH\python\run_knee.py"

"%PY%" "%DRIVER%"

endlocal
