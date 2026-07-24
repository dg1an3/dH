@echo off
REM ============================================================
REM  Brimstone -- separable entropy at the knee (W = 2e-4)
REM  Launches the app with the regularizer configured; build the
REM  plan (Import DICOM -> Plan Setup: 7 beams -> prescriptions ->
REM  Optimize) yourself in the GUI. The env vars below are read by
REM  the C++ side (Prescription.cpp / OptThread.cpp) at startup.
REM ============================================================
setlocal

set "BRIMSTONE_ENTROPY_SEPARABLE=1"
set "BRIMSTONE_ENTROPY_WEIGHT=2e-4"
set "BRIMSTONE_DETERMINISTIC=1"

set "EXE=C:\dev_dH_and_friends\dH\x64\Release\Brimstone.exe"

REM /D sets the working dir so the app finds its *.dat kernels
start "" /D "C:\dev_dH_and_friends\dH\Brimstone" "%EXE%"

endlocal
