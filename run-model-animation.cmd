@echo off
setlocal
set "viewer=%~dp0build\native\Release\deadlocked_native_scene_viewer.exe"
set "scene=%~dp0build\native-assets\armor-0-native-animation.dlscene"
set "poses=%~dp0build\native-assets\armor-0-native-animation.dlposes"
if not exist "%viewer%" goto missing
if not exist "%scene%" goto missing
if not exist "%poses%" goto missing
echo Original animation0 evaluated by native stages - presentation playback, no player controls or gameplay.
"%viewer%" "%scene%" --poses "%poses%" --camera 0 1.8 0.8 -1.5707963 -0.08 %*
if errorlevel 1 goto failed
exit /b 0

:missing
echo Build and prepare the animation diagnostic as described in docs\NATIVE_MODELS.md.
:failed
if "%~1"=="" pause
exit /b 1
