@echo off
setlocal
set "viewer=%~dp0build\native\Release\deadlocked_native_scene_viewer.exe"
set "scene=%~dp0build\native-assets\armor-0-native-topology.dlscene"
if not exist "%viewer%" goto missing
if not exist "%scene%" goto missing
echo Native selected-body blend and topology diagnostic - static pose, no animation or gameplay.
"%viewer%" "%scene%" %*
if errorlevel 1 goto failed
exit /b 0

:missing
echo Build and prepare the topology diagnostic as described in docs\NATIVE_MODELS.md.
:failed
if "%~1"=="" pause
exit /b 1
