# tools rules

`tools/` has exactly four entry points; the scripts make no business decisions themselves.

| File | Purpose |
|---|---|
| `build-keysound.bat` | Enters the MSVC environment via `vcvars64.bat`, compiles the 13 .cpp files under `src/` into `build\obj\`, and links them into `keysound.exe` in the repository root |
| `test-keysound.bat` | Calls build first, then compiles and runs the 7 tests under `tests/`, emitting `build\bin\*.exe` |
| `runtime_probe.cpp` | Prints the current process integrity level, session, and desktop name (for troubleshooting tray failures) |
| `tray_probe.cpp` / `tray_test.cpp` | Verify independently whether `Shell_NotifyIconW` is accepted |

## Hard constraints

- Scripts invoke `cl.exe` and `vcvars64.bat` with absolute Windows paths; never rely on MSVC happening to be on `%PATH%`.
- Compiler intermediates always go to `build\obj\` and `build\`; the release exe goes to the repository root (root `AGENTS.md`, rule 7).
- Adding a source file or a test requires updating three places at once: the `build\obj` compile list, the object link list, and the test run list; otherwise the bat and bash paths diverge.
- No comments in scripts (root `AGENTS.md`, rule 9). Variable names carry the meaning of parameters.

## Correct invocation from MSYS bash

```bash
MSYS_NO_PATHCONV=1 cmd.exe /c call "tools\\build-keysound.bat"
MSYS_NO_PATHCONV=1 cmd.exe /c call "tools\\test-keysound.bat"
```

- `cmd //c ...` only opens an interactive cmd, prints the banner, and exits without running the script.
- `cmd.exe /c tools/build-keysound.bat` is mangled by path conversion and fails with `'tools' is not recognized`.
- Passing switches such as `/FI` to Windows programs also requires `MSYS_NO_PATHCONV=1`, or MSYS rewrites `/FI` as a path.

## Exit status

The bat scripts short-circuit with `if errorlevel 1 exit /b 1` after each stage. `test-keysound.bat` is all green only when it prints 7 `== <name>` lines and returns 0; a missing line means a failure halfway through.
