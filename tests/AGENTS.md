# tests rules

Each test is a standalone `.cpp` with its own `main`. A failed assertion is a failure; nothing but "pass" is printed on success.

| Test | Behavior it locks down |
|---|---|
| `tests_latency.cpp` | One physical press plays once, release-then-press plays again, switching modes cancels decoding, the queue is cleared; uses the injected `FakeBackend` |
| `tests_window_lifecycle.cpp` | With `trayAvailable=true`, WM_CLOSE hides the window; with `false`, it minimizes |
| `tests_soundpack.cpp` | UTF-8/surrogate-pair parsing of `index.json`, the 85 mappings in the 成步堂音效 pack, `packKey` key names, and real files playable through MCI |
| `tests_builtin_modes.cpp` | The 12 built-in modes have unique command IDs, correct directory tables, and are unavailable when the directory is missing |
| `tests_memory_audio.cpp` | The Media Foundation decode contract and decoding a real WAV file |
| `tests_pack_memory.cpp` | Pre-decoded byte count and submission time ceiling for a real pack |
| `tests_settings.cpp` | Round-trip of the 6-line settings file, invalid files not overwritten, fallback to defaults |

## Linking rules

Tests never compile sources from `src/`; they always link object files from `build/obj/`:

- Common set `COMMON` = `diagnostics paths playback_mode builtin_modes registry`
- Add `legacy_playback pack_player keyname hooks` when hooks/playback are needed
- Add `memory_audio_backend window` when the tray window is needed
- `tests_memory_audio`, `tests_pack_memory`, and `tests_settings` link no objects, only libraries

`tests_latency` ships its own fake `createAudioBackend()`, so it **must not** link `memory_audio_backend.obj`, or the symbol would be duplicated.

## Hard constraints

- Tests provide their own factories or fixtures and do not change the observable behavior of production code. Loosening an assertion to make a test pass is cheating.
- Do not use `%TEMP%`: on this machine the session sandbox denies child processes creating files in the temp directory (`GetTempFileNameW` returns `ERROR_ACCESS_DENIED`). Always create a `<name>-test-<pid>` directory next to the executable and delete it afterward.
- Do not assume the executable sits in the repository root when locating resources: walk up from `runPathW()` until you find a directory containing a `sounds` subdirectory.
- Write no comments in test files (root `AGENTS.md`, rule 9).
- A new test must be added to both `build/build-tests.sh` and `tools/test-keysound.bat` (compile list, link line, run list — three places), or running the scripts as documented will skip it.

## Running

```bat
tools\test-keysound.bat
```

From MSYS bash: `MSYS_NO_PATHCONV=1 cmd.exe /c call "tools\test-keysound.bat"`.
The script aborts on the first failed assertion, so "all 7 `== <name>` lines printed and exit code 0" is the only all-green result.
