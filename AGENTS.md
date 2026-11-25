# Development rules

A Windows keyboard sound program: low-level keyboard/mouse hooks → mode mapping → in-memory sound playback, with a tray menu and sound pack (`index.json`) loading.
Product acceptance is confirmed by the user only; a feature without that confirmation must not be called complete.

## Hard rules

1. Preserve user sounds and pack configuration in `sounds/`, and user settings in the repository root. Do not delete or overwrite them without authorization.
2. Never perform file I/O, MCI calls, waits, or thread creation inside low-level input hooks; hooks may only decide and post.
3. One physical key press triggers exactly one sound; ignore auto-repeat presses until the key is released.
4. Use UTF-16 Windows path APIs and UTF-8 JSON; never depend on the system ANSI code page.
5. Distinguish compilation, simulated tests, and real desktop verification; the first two are not evidence of background playback or a working tray.
6. Stop on a permission denial; do not bypass it by changing shells, elevating, or switching to desktop input.
7. Build intermediates and test binaries go to `build/`; the release program goes to the repository root so it keeps resolving `sounds/` relatively.
8. Do not modify `_archive/`; new development touches only `src/`, `tests/`, `tools/`, and `_docs/`.
9. Write no comments in code. The ability to read code is more reliable than comments: comments go stale and lie. Express intent through naming, structure, and module boundaries.
10. Every new build unit must be added to `build/build-app.sh`, `build/build-tests.sh`, `tools/build-keysound.bat`, and `tools/test-keysound.bat` at the same time, or linking will fail with missing symbols.

## What to read before entering a scope

The host loads nested `AGENTS.md` files layer by layer (verified: subdirectory rules are injected automatically when entering `src/`, `tests/`, and `tools/`). Child files therefore carry only directory-specific details, while global rules stay in this file; the two never repeat each other.

| Directory | Responsibility | Read before working |
|---|---|---|
| `src/` | Production logic, layered as core/audio/input/packs/app | [src/AGENTS.md](src/AGENTS.md) |
| `tests/` | Regression verification; 7 tests cover hooks, sound packs, settings, window lifecycle, built-in modes | [tests/AGENTS.md](tests/AGENTS.md) |
| `tools/` | Build and diagnostics entry points | [tools/AGENTS.md](tools/AGENTS.md) |
| `sounds/` | Runtime sounds and packs (runtime data, not source) | Not needed |
| `build/` | Generated output; never the only copy of source or user resources | Not needed |
| `_archive/` | Original deliverables; not a build input | Not needed |
| `_docs/` | Process, verification records, change history | [_docs/development.md](_docs/development.md) |

Module and data-flow details, including failure modes, are in [_docs/development.md](_docs/development.md); build arguments are in [_docs/build.md](_docs/build.md); the test matrix is in [_docs/verification.md](_docs/verification.md); dates and migration records are in [_docs/changelog.md](_docs/changelog.md).

## Common commands

```bat
tools\build-keysound.bat
tools\test-keysound.bat
```

Both call `call "...\vcvars64.bat"` to enter the MSVC environment. Invoking a bat from MSYS bash must be written as `MSYS_NO_PATHCONV=1 cmd.exe /c call "tools\test-keysound.bat"`: `cmd //c` only opens an interactive cmd, and `/c tools/...` gets mangled by path conversion.

## Documentation responsibilities

README covers installation and usage only. Process lives in `_docs/`; change and verification records live in [_docs/changelog.md](_docs/changelog.md). The root file keeps only global hard rules and the routing table; directory details sink into child `AGENTS.md` files and `_docs/`.
