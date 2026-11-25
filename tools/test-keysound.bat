@echo off
setlocal
cd /d "%~dp0.."
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b %errorlevel%

call tools\build-keysound.bat
if errorlevel 1 exit /b 1

set LIBS=winmm.lib user32.lib shell32.lib gdi32.lib ole32.lib oleaut32.lib advapi32.lib mfplat.lib mfreadwrite.lib mfuuid.lib xaudio2.lib
set CORE=build\obj\diagnostics.obj build\obj\paths.obj build\obj\toggles.obj build\obj\registry.obj
set PACK=%CORE% build\obj\pack_player.obj build\obj\hooks.obj

if not exist build\bin mkdir build\bin
for %%T in (tests_latency tests_window_lifecycle tests_soundpack tests_i18n tests_memory_audio tests_pack_memory tests_settings) do (
  cl /nologo /c /EHsc /std:c++17 /utf-8 /DUNICODE /D_UNICODE /Isrc /Fo:build\%%T.obj tests\%%T.cpp
  if errorlevel 1 exit /b 1
)

cl /nologo build\tests_latency.obj %PACK% /Fe:build\bin\tests_latency.exe /link %LIBS%
if errorlevel 1 exit /b 1
cl /nologo build\tests_window_lifecycle.obj %PACK% build\obj\memory_audio_backend.obj build\obj\window.obj /Fe:build\bin\tests_window_lifecycle.exe /link %LIBS%
if errorlevel 1 exit /b 1
cl /nologo build\tests_soundpack.obj %CORE% /Fe:build\bin\tests_soundpack.exe /link %LIBS%
if errorlevel 1 exit /b 1
cl /nologo build\tests_i18n.obj /Fe:build\bin\tests_i18n.exe /link %LIBS%
if errorlevel 1 exit /b 1
cl /nologo build\tests_memory_audio.obj /Fe:build\bin\tests_memory_audio.exe /link %LIBS%
if errorlevel 1 exit /b 1
cl /nologo build\tests_pack_memory.obj /Fe:build\bin\tests_pack_memory.exe /link %LIBS%
if errorlevel 1 exit /b 1
cl /nologo build\tests_settings.obj /Fe:build\bin\tests_settings.exe /link %LIBS%
if errorlevel 1 exit /b 1

for %%T in (tests_latency tests_window_lifecycle tests_soundpack tests_i18n tests_memory_audio tests_pack_memory tests_settings) do (
  echo == %%T
  start "" /wait build\bin\%%T.exe
  if errorlevel 1 exit /b 1
)
exit /b 0
