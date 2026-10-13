@echo off
setlocal
set "root=%~dp0"
set "viewer=%root%build\native\Release\deadlocked_native_scene_viewer.exe"
set "scene=%root%build\native-assets\dreadzone-mission-0-placement-audited-v2.dlscene"
set "python=%root%venv\Scripts\python.exe"
if not exist "%viewer%" (
    echo Native scene viewer is missing. Build deadlocked_native_scene_viewer from docs\NATIVE_PORT.md.
    exit /b 1
)
if not exist "%root%private\overlays\levels\campaign\1_dreadzone_station\gameplay.instances" (
    echo Dreadzone extracted assets are missing. Prepare private\overlays from your game disc first.
    exit /b 1
)
if not exist "%scene%" (
    if not exist "%python%" set "python=python"
    "%python%" "%root%tools\prepare_native_level.py" --extracted "%root%private\overlays" --level "%root%private\overlays\levels\campaign\1_dreadzone_station" --mission 0 --include-dynamic-previews --include-native-bind-previews --native-bind-class 8554 --include-unresolved-moby-markers -o "%scene%"
    if errorlevel 1 exit /b 1
)
echo Opening the Dreadzone mission 0 placement audit. Markers show omitted Moby positions only.
echo W/A/S/D move, Q/E down/up, arrow keys look, Shift moves faster, Escape exits.
"%viewer%" "%scene%" %*
exit /b %errorlevel%
