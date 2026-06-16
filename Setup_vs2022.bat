@echo off
setlocal enabledelayedexpansion

set "ROOT_DIR=%~dp0"
set "ROOT_DIR=%ROOT_DIR:~0,-1%"
cd /d "%ROOT_DIR%"

if not exist "build" mkdir build
if not exist "intermediate" mkdir intermediate
if not exist "binaries" mkdir binaries

cmake -G "Visual Studio 17 2022" -A x64 ^
    -S "%ROOT_DIR%" ^
    -B "%ROOT_DIR%\build" ^
    -DCMAKE_CONFIGURATION_TYPES="Release;Debug"

pause
