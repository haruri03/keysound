# keysound

**English** · [简体中文](readme_cn.md)

A keyboard and mouse sound player for Windows. Every key press can play a sound effect — pick a built-in set, or load your own sound pack.

## Quick start

1. Keep `keysound.exe` and the `sounds` folder together in the same directory.
2. Double-click `keysound.exe`.
3. A small window opens and a tray icon appears. Right-click the tray icon to pick a sound set and toggle the options.
4. Type anywhere — you should hear the sounds.

Closing the window keeps the program running in the tray. Use **Exit** in the menu to stop it.

## Sound packs

Put a folder under `sounds\` with an `index.json` and an audio subfolder, and it shows up in the **Sound packs** menu on the next start. The full guide — file layout, every `index.json` field, the three playback modes, size limits and troubleshooting — is in [the sound pack guide](docs/sound-packs.md).

## Settings

Modes, the selected pack and both toggles are saved automatically to `keysound-settings.dat` next to the executable. Delete that file to return to defaults.

## Requirements

Windows 10 or 11, 64-bit. If the program does not start, install the Visual C++ runtime included as `Visual_C  RuntimePack_3.0.22.0317.exe`.

## Troubleshooting

- **No tray icon** — the window shows the tray state and retries every few seconds; you can always use the button in the window instead of the tray.
- **No sound** — check the volume mixer for this app, and watch the status line in the window: it reports how many sounds are loaded and whether playback was accepted.
- **Silent inside an elevated app** — Windows does not deliver input across integrity levels; run this program as administrator too.
- **Holding a key repeats the sound** — it should not: one physical press plays once. If it does, report it as a bug.

## Credits

Refactored from [yearn19/keysound](https://github.com/yearn19/keysound).
