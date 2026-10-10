@echo off
setlocal
set "viewer=%~dp0build\native\Release\deadlocked_native_scene_viewer.exe"
set "scene=%~dp0build\native-assets\dreadzone-static.dlscene"
if not exist "%viewer%" (
    echo Native viewer is missing. Build the Release target described in docs\NATIVE_PORT.md.
    goto failed
)
if not exist "%scene%" (
    echo Local Dreadzone assets are missing. Follow docs\NATIVE_PORT.md to prepare them from your disc.
    goto failed
)
echo Dreadzone native scenery viewer - game logic is not implemented yet.
echo W/A/S/D: move. Q/E: down/up. Arrow keys: look. Shift: faster. Escape: exit.
"%viewer%" "%scene%" --camera 600 480 203 -1.5707963 -0.05 %*
if errorlevel 1 goto failed
exit /b 0

:failed
if "%~1"=="" pause
exit /b 1
