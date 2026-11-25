# src rules

Program logic is split by layer, and **one file has exactly one responsibility**. Before adding a file, decide which layer it belongs to, then choose the name.

| Layer | File | Responsibility |
|---|---|---|
| core | `core/paths.{h,cpp}` | `getRunPath()` / `runPathW()`; the only place allowed to assemble install paths |
| core | `core/diagnostics.{h,cpp}` | `integrityLevel()`, `processIntegrity`, `restrictedIntegrity`, `reportWin32()` |
| audio | `audio/toggles.{h,cpp}` | Global `flag_fast` (interruption) and `flag_mouse` (mouse sounds) |
| audio | `audio/audio_backend.h` | Abstract interface `keysound::AudioBackend` plus the `createAudioBackend()` factory declaration |
| audio | `audio/memory_audio_backend.cpp` | The only TU implementing the factory; wraps the header-only `memory_audio.h` |
| audio | `audio/pack_player.{h,cpp}` | Worker thread, bounded queue, pre-decode, generation-based cancellation |
| input | `input/hooks.{h,cpp}` | Low-level keyboard/mouse hooks, auto-repeat filtering, non-blocking dispatch to `PackPlayer` |
| packs | `packs/registry.{h,cpp}` | `discoverPacks()`, `packKey()`, `activePack` / `availablePacks` / `packActive` / `packGeneration` |
| app | `app/window.cpp` | Window procedure, tray, menus, settings read/write entry points |
| app | `app/icon.cpp` | Window class registration and message loop |
| app | `app/main.cpp` | Process lifetime and thread teardown |
| root | `memory_audio.h`, `soundpack.h`, `settings.h` | Self-contained utility headers that depend on no other module |

## Hard constraints

- Sound packs are the only sound source. Do not reintroduce built-in mode tables, MCI playback, or a second input queue; a new mode belongs in the pack format (`index.json`), not in the executable.
- `src/main-updated.cpp` is the historical single-file original and **no longer a build input**. `build/split-sources.sh` only performed a one-time split; do not re-run it to sync changes — the module tree is the only source.
- No comments inside `src/` (root `AGENTS.md`, rule 9). Do not explain control flow with comments; change names and structure so the code explains itself.
- The hook thread only decides and posts. Any file I/O, decode, lock wait, or thread creation must happen on the `PackPlayer` worker.
- Any variable shared across modules is declared `extern` in a header and defined in exactly one .cpp. A common build failure is the same symbol being defined in two layers.
- Headers must be self-sufficient: include every standard library header they need. Historically generated headers relied on the includer providing `using namespace std;`; never introduce that kind of implicit contract again.
- Playback goes only through `keysound::AudioBackend`. A new backend implements `audio_backend.h` and provides `createAudioBackend()` in its own TU; tests may supply a different factory, so never put the implementation into `pack_player.cpp`.

## Verification after changes

`bash build/build-tests.sh` must compile the app and show every test green before you report completion. A successful compile alone does not count.
