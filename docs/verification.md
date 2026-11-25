# Verification flow

> Read before changing a test or accepting a feature. Scope: `tests/` and the real Windows desktop.

Tests use a dedicated temporary settings path and must never rewrite the user's `keysound-settings.dat`.

Build-simulation tests confirm the result of press, auto-repeat, release and press-again; holding a key must not pile up playback events.

Real desktop acceptance: select a sound pack, switch to another medium-integrity application, press keys and check the input and playback diagnostics; repeat the minimized-window scenario.

Test tray icon visibility, menu usability and recovery after an Explorer restart separately; a fallback window does not count as a passing tray.

Change the pack and the toggles, exit normally, restart and confirm the restored state; a first successful write does not prove that overwriting works.

Record errors from restricted environments separately from ordinary desktop results; when authorization is needed, request approval for that exact operation.
