# Build workflow
> When to read: before running or modifying the tools/ scripts. Scope: Visual Studio compilation and regression.

`tools\build-keysound.bat` builds an unsigned executable, by default `keysound.exe` in the repository root. `tools\test-keysound.bat` invokes the same build.

For an isolated build, set `KEYSOUND_OBJDIR` to a separate directory under `build\` and `KEYSOUND_OUTPUT` to a separate output path under `build\`. Do not use an output path occupied by a running program.
The scripts locate the repository root from their own path and initialize the x64 compiler environment with `vcvars64.bat` from the installed Visual Studio.
Compilation uses C++17, `/utf-8`, `UNICODE`, and `_UNICODE`; intermediates and test executables go to `build/`.
The test programs use the root `sounds/`; never mistake `build/sounds/` for the resource root just because the test executables live under `build/`.
On a non-zero exit, inspect the specific compile or test output. Ask the user to exit the running target program before building; never terminate a user process on your own to overwrite a file.
