@echo off
setlocal
set "root=%~dp0"
set "viewer=%root%build\native\Release\deadlocked_native_scene_viewer.exe"
set "scene=%root%build\native-assets\dreadzone-mission-0-sky-review.dlscene"
set "python=%root%venv\Scripts\python.exe"
set "tools=%root%tools"
set "extracted=%root%private\overlays"
if not exist "%tools%\prepare_native_level.py" (
    set "tools=%root%..\rac-deadlocked-decomp\tools"
)
if not exist "%extracted%\levels\campaign\1_dreadzone_station\gameplay.instances" (
    set "extracted=%root%..\rac-deadlocked-decomp\private\overlays"
)
if not exist "%viewer%" set "viewer=%root%build\native-sky-viewer\Release\deadlocked_native_scene_viewer.exe"
if not exist "%viewer%" (
    echo Native scene viewer is missing. Build the native-sky-viewer target from native\CMakeLists.txt.
    exit /b 1
)
if not exist "%extracted%\levels\campaign\1_dreadzone_station\gameplay.instances" (
    echo Dreadzone extracted assets are missing. Prepare private\overlays from your game disc first.
    exit /b 1
)
if not exist "%python%" set "python=python"
if not exist "%tools%\prepare_native_level.py" (
    echo Native level preparation tools are missing.
    exit /b 1
)
"%python%" "%tools%\prepare_native_level.py" --extracted "%extracted%" --level "%extracted%\levels\campaign\1_dreadzone_station" --mission 0 --include-dynamic-previews --include-native-bind-previews --include-sky -o "%scene%"
if errorlevel 1 exit /b 1
echo Opening Dreadzone mission 0 with a camera-following alpha-composited sky diagnostic.
echo W/A/S/D move, Q/E down/up, arrow keys look, Shift moves faster, Escape exits.
"%viewer%" "%scene%" --sky-shell-count 4 %*
exit /b %errorlevel%
