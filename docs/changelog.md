# Changelog

## Mouse press sounds and volume (2026-11-25)

Left, right, and middle mouse buttons now trigger their mapped sounds on press. The tray and fallback sound menus offer a Volume submenu with 0% to 100% in 10% increments. The selection applies to new keyboard and mouse sounds and is saved across restarts. Older settings files load at 100%.

The MSVC build and six regression executables passed. The existing sound pack test stopped at the legacy MCI open call. Desktop tray and audible playback verification are pending.

Source: the user-approved directory reorganization plan and development verification; date: 2026-11-10.

## Directory reorganization
Maintenance code moved to src/, tests to tests/, scripts and probes to tools/, the original main program and binaries to _archive/, and generated output to build/. sounds/, user settings, and the release program's location in the root were preserved.
At the time, layered loading of subdirectory rules by the host was not yet confirmed, so only the root AGENTS.md was created; that conclusion was overturned by later testing, see "Modularization and documentation layering".
debug.log was held open by another process and could not be moved; it stays in place and was not force-unlocked.
The original README carried a 2024/11/11 note about updating MFC2.exe; that history moved into this record and is not evidence for the new version.

## In-memory sound implementation and verification
JSON sound packs switched to Media Foundation pre-decoding to PCM16/48 kHz/stereo with XAudio2 in-memory playback; built-in modes still use the legacy player.
Buffers are held only on the audio worker thread. Switching packs cancels the old warm-up, clears the event queue, destroys the old source voice, and only then frees the PCM; input is accepted only after the entire warm-up succeeds. Key presses are dropped during warm-up and never replayed. Queue limit 64; events older than 100 ms are dropped.
Kept limits: 16 MiB per file and 128 MiB total retained PCM, 32 voices; this excludes memory internal to system decoders and transient growth during expansion. Synchronous system decode calls cannot be force-cancelled; cancellation and the 30-second timeout are checked between calls.
The real 成步堂音效 pack deduplicated 9 files and retained 1,892,728 bytes of PCM; the most recent test measured 39.1159 ms of warm-up and 0.6843 ms for 100 playback submissions. Submission time is not speaker end-to-end latency; no before/after acoustic comparison was made.
Passed: MSVC build; long-press repeat filtering and release-then-press; input dropped during warm-up; fast-switch cancellation simulation; real WAV decode / voice limit / reset; real MP3 pack decode / reload after reset; the original JSON mapping tests.
Desktop startup observation: the JSON pack was ready in memory with HRESULT 0; the tray still reported registration failure, so it cannot be claimed fixed. The standalone foreground/background input probe never received foreground focus and sent no test keys, so background playback still awaits a real user test.

## Subsequent runtime environment diagnosis
Read-only security-context probe: the tool process integrity is 0x1000 (Low) and Explorer is 0x2000 (Medium), both in session 1 on the Default desktop. The danger-full-access file policy does not mean the tool process becomes Medium. Do not work around this by modifying the Explorer message filter or auto-elevating.
A user-launched keysound process, PID 21136, was then observed: the window showed "tray registration accepted", memory sounds ready, 90 submissions, queue 0, HRESULT=0. That is evidence that a normal launch environment is better than the tool's launch environment, but not full acceptance of a visible icon or actual background audio.
Added a low-integrity notice that only queries its own token, without changing privileges. Because the user program was still running, linking the original file failed with LNK1104; the user process was left alone and a diagnostic build was produced as keysound-next.exe. The final regression passed; the running keysound-updated.exe was not covered.

## Historical fixes awaiting verification
The user still reports no sound when unfocused, tray registration failure, and a long-press backlog. Earlier builds and simulated tests passing are not enough to confirm real desktop behavior; overwriting settings was once blocked by ERROR_ACCESS_DENIED (5) in the restricted environment.

## Modularization and documentation layering
Date: 2026-11-11.

`src/main-updated.cpp` (2674 lines) was split into 13 translation units by core / audio / input / packs / app; `build/split-sources.sh` only performed a one-time extraction — the module tree is the only source from now on, and changes are no longer synced by re-running the script.
`PackPlayer`'s audio backend became the `keysound::AudioBackend` abstraction plus a `createAudioBackend()` factory; `memory_audio_backend.cpp` adapts `keysound::MemoryAudio`; `tests_latency` ships its own fake factory, so it does not link `memory_audio_backend.obj`.
Three tests that previously used `#include "main-updated.cpp"` (tests_latency, tests_window_lifecycle, tests_soundpack) now include module headers and link object files.
Build entry points became multi-translation-unit: `build/build-app.sh`, `build/build-tests.sh`, `tools/build-keysound.bat`, `tools/test-keysound.bat`.

## Removing hard-coded built-in modes
`init_res` no longer pre-creates the fourteen legacy subdirectories random / keyname / keyname_default / mouse / piano / yasuo / zhixia / majiang / fengmingqi / muyu / qifengle / cedar / cedar_default / yin; it only ensures that the `sounds/` root exists, so directories the user deleted do not reappear on the next start.
The built-in mode menu is now generated dynamically from `builtinModes()` and directory existence; this added `src/audio/builtin_modes.h`, `src/audio/builtin_modes.cpp`, and `tests/tests_builtin_modes.cpp`; the regression suite grew from 6 to 7 tests, all green.

## Documentation layering
Testing confirmed that the host does load nested `AGENTS.md` files layer by layer (subdirectory rules are injected automatically when entering src/, tests/, and tools/); the "layered loading unconfirmed" conclusion in the "Directory reorganization" section is void.
The root `AGENTS.md` keeps only the 10 hard rules and the routing table; `src/AGENTS.md`, `tests/AGENTS.md`, and `tools/AGENTS.md` were added; `docs/development.md` gained module layering, data flow, and failure modes.

## Comment cleanup
After rule 9 of the root `AGENTS.md` became "write no comments in code", comments in `src/` and `tests/` were removed as well, 289 lines in total.
`build/strip-comments.py` uses a line-by-line state machine to handle `//`, `/* */`, string and character literals (including escapes), and multi-line block comments, preserving the original CRLF/LF line endings; a comment-only line is deleted whole, and a trailing comment is truncated in place; `src/main-updated.cpp` is the pre-split historical original, not a build input, so it was excluded and left unchanged.
After cleanup, `src/` and `tests/` contain no comment markers except the `"/*"` wildcard string passed to `_findfirst` in `src/audio/legacy_playback.cpp`. Backups from before cleanup are in `build/src-before-comments/` and `build/tests-before-comments/`.
Rebuilt with zero errors and all 7 regression tests green.
