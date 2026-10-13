@echo off
setlocal
set "root=%~dp0"
set "viewer=%root%build\native\Release\deadlocked_native_scene_viewer.exe"
set "scene=%root%build\native-assets\dreadzone-mission-0-collision.dlscene"
set "python=%root%venv\Scripts\python.exe"
if not exist "%viewer%" (
    echo Native scene viewer is missing. Build deadlocked_native_scene_viewer from docs\NATIVE_PORT.md.
    exit /b 1
)
if not exist "%root%private\overlays\levels\campaign\1_dreadzone_station\chunks\0\collision\collision.dae" (
    echo Dreadzone extracted collision assets are missing. Prepare private\overlays from your game disc first.
    exit /b 1
)
if not exist "%scene%" (
    if not exist "%python%" set "python=python"
    "%python%" "%root%tools\prepare_native_collision.py" --level "%root%private\overlays\levels\campaign\1_dreadzone_station" -o "%scene%"
    if errorlevel 1 exit /b 1
)
echo Opening the Dreadzone mission 0 collision scene. This scene contains extracted collision triangles only.
echo W/A/S/D move, Q/E down/up, arrow keys look, Shift moves faster, Escape exits.
"%viewer%" "%scene%" %*
exit /b %errorlevel%
