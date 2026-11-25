# Development workflow
> When to read: before changing `src/`. Scope: module layering, data flow, and failure modes.

## Layers and responsibilities

| Location | Responsibility |
|---|---|
| `src/core/paths.cpp` | `getRunPath` (ANSI), `runPathW` (UTF-16, the executable's directory) |
| `src/core/diagnostics.cpp` | `integrityLevel`, `processIntegrity`, `restrictedIntegrity`, `reportWin32` |
| `src/audio/toggles.cpp` | Two global switches: `flag_fast` (sound interruption), `flag_mouse` (mouse sounds) |
| `src/audio/audio_backend.h` | The `keysound::AudioBackend` abstraction and the `createAudioBackend()` factory declaration |
| `src/audio/memory_audio_backend.cpp` | Factory implementation that adapts `keysound::MemoryAudio` (MF decode + XAudio2) to `AudioBackend` |
| `src/audio/pack_player.cpp/.h` | `PackPlayer`: event queue, audio worker thread, warm-up and cancellation, diagnostic counters |
| `src/input/hooks.cpp/.h` | `WH_KEYBOARD_LL` / `WH_MOUSE_LL` hooks; forwards presses to `PackPlayer` |
| `src/packs/registry.cpp/.h` | `activePack`, `availablePacks`, `packActive`, `discoverPacks`, `packKey` |
| `src/app/window.cpp` | `WndProc`: menus, tray, status labels, settings restore and save |
| `src/app/icon.cpp` | `SetIco`: register the window class, create the window, run the message loop |
| `src/app/main.cpp` | Process startup and shutdown order |
| `src/memory_audio.h` | Self-contained header: MF pre-decode to PCM16/48 kHz/stereo, XAudio2 in-memory playback |
| `src/soundpack.h` | Self-contained header: UTF-8 JSON parsing and sound pack loading (`index.json`) |
| `src/settings.h` | Self-contained header: read/write the versioned ASCII settings file |

`src/main-updated.cpp` is the pre-split single-file original; it is **not a build input**.

## Data flow

Startup: `main` → `packPlayer.start()` → create the `keyboardReady` event → `thread key_hook` installs the hooks → wait until the hooks are ready → `SetIco()` creates the window and runs the message loop. On exit: `PostThreadMessage(keyboardThreadId, WM_QUIT)` → wait for the hook thread → `packPlayer.shutdown()`.

Key press: the hook thread only decides and posts. `LowLevelKeyboardProc` filters auto-repeat with a "scan code + extended bit" table, so one physical press posts exactly once, then calls `packPlayer.submit`. `LowLevelMouseProc` submits left/right/middle releases the same way when `flag_mouse` is set.

Sound packs: `submit` enqueues with `try_to_lock`; when the queue is full the event is dropped and counted. The worker validates each event against the generation and drops events older than 100 ms. Warm-up runs reset → snapshot `activePack` under `packMutex` → resolve the path for the mode → `initialize` → `preload` (with a cancellation callback) → set `readyState` only after everything succeeds. Key presses during warm-up are dropped outright, never replayed.

Menus: `discoverPacks()` scans `sounds/*` and keeps only directories whose `index.json` loads. Packs are the only sound source; picking one sets `packActive` and prewarms it.

## Failure modes

- Hook installation fails: recorded in `keyboardHookError` / `mouseHookError`; the process keeps running.
- Tray registration fails: `trayAvailable` is false, retry every 5 seconds, and show the "Restart as administrator" button (command 101).
- Warm-up fails or is cancelled: `readyState` is false and `lastError` records the HRESULT; there is no fallback playback path.
- Queue full or the hook thread cannot take the lock: `droppedCount` increments; the hook thread never waits.
- A synchronous Media Foundation decode cannot be force-cancelled: switching packs and exiting wait for the worker to return from the current call.
- Settings write denied: returns `Protected` or `IoError` and leaves the existing file untouched.

## Local hard constraints

Write no comments in code. Headers must be self-contained (they may not rely on the includer providing `using namespace std;`). Playback goes only through `keysound::AudioBackend`; a new backend provides `createAudioBackend()` in its own translation unit. A new source file must be added to `build/build-app.sh`, `build/build-tests.sh`, and `tools/build-keysound.bat` at the same time. Work is complete only when all tests in `build/build-tests.sh` pass.
