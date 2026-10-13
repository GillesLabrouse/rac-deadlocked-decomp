@echo off
setlocal
set "root=%~dp0"
set "viewer=%root%build\native\Release\deadlocked_native_scene_viewer.exe"
set "scene=%root%build\native-assets\dreadzone-mission-0-all-placement-audit.dlscene"
set "python=%root%venv\Scripts\python.exe"
set "tools=%root%tools"
set "sibling=%root%..\rac-deadlocked-decomp"
set "extracted=%root%private\overlays"
if not exist "%tools%\prepare_native_level.py" set "tools=%sibling%\tools"
findstr /C:"--include-unresolved-moby-markers" "%tools%\prepare_native_level.py" >nul
if errorlevel 1 set "tools=%sibling%\tools"
if not exist "%extracted%\levels\campaign\1_dreadzone_station\gameplay.instances" (
    set "extracted=%sibling%\private\overlays"
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
"%python%" "%tools%\prepare_native_level.py" --extracted "%extracted%" --level "%extracted%\levels\campaign\1_dreadzone_station" --mission 0 --include-dynamic-previews --include-native-bind-previews --include-unresolved-moby-markers --include-sky -o "%scene%"
if errorlevel 1 exit /b 1
echo Diagnostic scene includes Moby editor meshes and native bind-pose previews without game animation.
echo Unresolved Moby markers show position only; pose and visibility are not implied.
echo W/A/S/D move, Q/E down/up, arrow keys look, Shift moves faster, Escape exits.
"%viewer%" "%scene%" --sky-shell-count 4 %*
exit /b %errorlevel%
