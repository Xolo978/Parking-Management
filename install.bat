@echo off
setlocal enabledelayedexpansion

echo   Parking Management System - Installer
echo.

REM Check for administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo This script requires administrator privileges.
    echo Please right-click and select "Run as administrator"
    pause
    exit /b 1
)

REM Check if chocolatey is installed
where choco >nul 2>&1
if %errorLevel% neq 0 (
    echo Chocolatey package manager not found.
    echo Installing Chocolatey...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"
    
    if %errorLevel% neq 0 (
        echo Failed to install Chocolatey.
        echo Please install manually from: https://chocolatey.org/install
        pause
        exit /b 1
    )
    
    REM Refresh environment
    call refreshenv
)

echo Installing dependencies...
choco install -y cmake --installargs 'ADD_CMAKE_TO_PATH=System'
choco install -y mingw
choco install -y git

echo.
echo Downloading and installing raylib...

REM Create temp directory
if not exist "%TEMP%\raylib_build" mkdir "%TEMP%\raylib_build"
cd /d "%TEMP%\raylib_build"

REM Clone raylib
if exist raylib (
    echo Removing old raylib directory...
    rmdir /s /q raylib
)

git clone --depth 1 https://github.com/raysan5/raylib.git
if %errorLevel% neq 0 (
    echo Failed to clone raylib repository
    pause
    exit /b 1
)

cd raylib
mkdir build
cd build

REM Build raylib
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
if %errorLevel% neq 0 (
    echo CMake configuration failed
    pause
    exit /b 1
)

cmake --build . --config Release
if %errorLevel% neq 0 (
    echo Build failed
    pause
    exit /b 1
)

REM Install raylib
cmake --install .

echo.
echo   Building Parking Management System
echo.

cd /d "%~dp0"

if exist build (
    echo Cleaning previous build...
    rmdir /s /q build
)

mkdir build
cd build

echo Running CMake...
cmake -G "MinGW Makefiles" ..
if %errorLevel% neq 0 (
    echo CMake configuration failed
    pause
    exit /b 1
)

echo Compiling...
cmake --build . --config Release
if %errorLevel% neq 0 (
    echo Build failed
    pause
    exit /b 1
)

echo.
echo   Installation Complete!
echo.
echo To run the application:
echo   cd build
echo   parking_lot.exe
echo.

pause
