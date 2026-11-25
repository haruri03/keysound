@echo off
setlocal
cd /d "%~dp0.."
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
if not defined KEYSOUND_OBJDIR set "KEYSOUND_OBJDIR=build\obj"
if not defined KEYSOUND_OUTPUT set "KEYSOUND_OUTPUT=keysound.exe"
if not exist "%KEYSOUND_OBJDIR%" mkdir "%KEYSOUND_OBJDIR%"
"%VCToolsInstallDir%bin\Hostx64\x64\cl.exe" /nologo /c /EHsc /std:c++17 /utf-8 /DUNICODE /D_UNICODE /Isrc /Fo"%KEYSOUND_OBJDIR%\\" ^
  src\core\diagnostics.cpp src\core\paths.cpp ^
  src\audio\toggles.cpp src\audio\trace.cpp src\audio\pack_player.cpp src\audio\memory_audio_backend.cpp ^
  src\input\hooks.cpp ^
  src\packs\registry.cpp ^
  src\app\window.cpp src\app\icon.cpp src\app\main.cpp
if errorlevel 1 exit /b 1
"%VCToolsInstallDir%bin\Hostx64\x64\cl.exe" /nologo ^
  "%KEYSOUND_OBJDIR%\diagnostics.obj" "%KEYSOUND_OBJDIR%\paths.obj" ^
  "%KEYSOUND_OBJDIR%\toggles.obj" "%KEYSOUND_OBJDIR%\pack_player.obj" "%KEYSOUND_OBJDIR%\memory_audio_backend.obj" ^
  "%KEYSOUND_OBJDIR%\hooks.obj" ^
  "%KEYSOUND_OBJDIR%\registry.obj" ^
  "%KEYSOUND_OBJDIR%\window.obj" "%KEYSOUND_OBJDIR%\icon.obj" "%KEYSOUND_OBJDIR%\main.obj" ^
  /Fe:"%KEYSOUND_OBJDIR%\keysound-staged.exe" ^
  /link winmm.lib user32.lib shell32.lib gdi32.lib ole32.lib oleaut32.lib advapi32.lib mfplat.lib mfreadwrite.lib mfuuid.lib xaudio2.lib
if errorlevel 1 exit /b 1
copy /y "%KEYSOUND_OBJDIR%\keysound-staged.exe" "%KEYSOUND_OUTPUT%" >nul
if errorlevel 1 exit /b 1
exit /b 0
