@echo off
setlocal
cd /d "%~dp0"
set "controllerProbe=build\native\Release\deadlocked_native_controller_probe.exe"
if not exist "%controllerProbe%" (
    echo The native controller diagnostic has not been built.
    echo Run powershell -ExecutionPolicy Bypass -File tools\check-native.ps1 first.
    if "%~1"=="" pause
    exit /b 1
)
if "%~1"=="" (
    "%controllerProbe%" --frames 10
) else (
    "%controllerProbe%" %*
)
set "probeResult=%ERRORLEVEL%"
if "%~1"=="" pause
exit /b %probeResult%
