@echo off
REM CMake Build Script for Windows
REM This script helps build the Brimstone project on Windows with Visual Studio

echo =====================================
echo CMake Build Script for Windows
echo =====================================
echo.

REM Check if ITK_DIR is set
if "%ITK_DIR%"=="" (
    echo ERROR: ITK_DIR environment variable is not set
    echo Please set ITK_DIR to point to your ITK build directory
    echo Example: set ITK_DIR=C:\path\to\ITK-build
    echo.
    goto :error
)

echo ITK_DIR is set to: %ITK_DIR%
echo.

REM Create build directory
if not exist build mkdir build
cd build

echo Configuring CMake...
echo ---------------------
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DITK_DIR=%ITK_DIR% ^
    -DBUILD_BRIMSTONE_APP=ON

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed
    goto :error
)

echo.
echo Configuration successful!
echo.
echo To build the project, run:
echo   cmake --build . --config Release
echo.
echo Or open the generated solution file in Visual Studio:
echo   start Brimstone.sln
echo.

cd ..
goto :end

:error
echo.
echo Build configuration failed!
cd ..
exit /b 1

:end
echo =====================================
echo Configuration Complete
echo =====================================
