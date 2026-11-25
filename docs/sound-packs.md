# Custom sound pack guide

**English** · [简体中文](sound-packs_cn.md)

This page explains how to build a sound pack from scratch and how each `index.json` field actually behaves.
For installing and using the program itself, see the [README](../README.md).

## 1. Where sound packs live

Sound packs live under `sounds\` in the program directory; one subdirectory is one pack:

```
keysound.exe
sounds\
  我的音效包\
    index.json
    sounds\
      a1.wav
      a2.wav
```

On startup the program scans every subdirectory under `sounds\`; any directory with a readable
`index.json` appears in the **Sound packs** submenu. Directories without an `index.json` are not listed.

After editing a pack you do not need to reinstall the program or change anything in it; just select it
again in the menu. After adding or removing a directory, restart the program to see the change (the
program scans only at startup and when switching packs).

## 2. Writing index.json

`index.json` must be encoded as **UTF-8**. The minimal working file has just two fields:

```json
{
  "name": "我的音效包",
  "mode": "assign"
}
```

Full example (map every key to an audio file, with an extra set of random sounds):

```json
{
  "name": "打字机",
  "mode": "assign",
  "repeat_sound": "",
  "assigned_sounds": [
    { "key": "1", "sound": "打字声 01.mp3" },
    { "key": "2", "sound": "打字声 02.mp3" },
    { "key": "A", "sound": "打字声 03.mp3" }
  ],
  "soundsList": ["打字声 01.mp3", "打字声 02.mp3", "打字声 03.mp3"]
}
```

Field reference:

| Field | Type | Required | Purpose |
|---|---|---|---|
| `name` | string | yes | Name shown in the menu. A pack without this field is rejected. |
| `mode` | string | yes | Playback mode: `assign`, `repeat`, `random`, or `mixed`. The Chinese aliases `指定`, `重复`, `随机`, and `混合` are also accepted. A pack without this field is rejected. |
| `assigned_sounds` | array | no | One file per key; each element is `{ "key": "...", "sound": "..." }`. |
| `soundsList` | string array | no | Candidate sound files; `random` mode draws from this list. |
| `repeat_sound` | string | no | Single file played on keyboard presses in `repeat` mode; empty means no repeat sound. |

`key` accepts ordinary characters (`1`, `A`) and function key names (for example `L Shift`, `R Ctrl`, arrow keys).
If you are unsure, choose "key-name mode" (键名模式) in the program, press the key you want, and read the name shown in the window.

`sound` contains **only a file name** — no path and no `sounds\` prefix. The program looks it up
first under the pack's `sounds\` subdirectory, then directly under the pack directory:

```
<pack directory>\sounds\<file name written in sound>
```

File names may contain spaces and Chinese characters, for example `打字声 03.mp3`.

## 3. How the four modes differ

- **`mixed`** (`混合`): an assigned keyboard key takes priority; an unassigned keyboard key draws from `soundsList`, or is silent if the list is empty. Mouse releases use only explicit `Mouse Left`, `Mouse Right`, or `Mouse Middle` assignments when mouse sounds are enabled; they never draw from the random pool.
- Sound filenames are looked up first in the pack's `sounds` subdirectory, then in the pack directory itself. Do not include directory separators in filenames.

- **`assign`** (`指定`): matches `assigned_sounds` exactly. A key with no entry plays nothing.
  Best for packs where every key has a distinct sound.
- **`random`** (`随机`): picks one entry from `soundsList` at random, regardless of which key was pressed.
  Best for packs where any key plays something different each time.
- **`repeat`** (`重复`): plays `repeat_sound` for keyboard presses; an empty field leaves the pack without a playable repeat sound.

Use `assign` for a key-name list, `random` for all keys, and `repeat` for a single long sound — pick whichever fits.

## 4. Audio file requirements and limits

- `.wav` and `.mp3` are supported; other extensions are not loaded.
- After selecting a pack, the program **decodes all audio into memory at once** before playback starts, so
  wait until the window status line reads "Memory sounds ready" (Chinese UI: 「内存音效就绪」) before
  expecting key presses to respond.
- The per-file limit is 16 MiB and the whole-pack retained-audio limit is 128 MiB. Files over the limit
  are skipped, and the window status line reports the error code (`HRESULT`).
- A pack that fails to decode does not fall back to disk playback: verify that the format is intact and
  the file is not locked by another program, then select the pack again.

## 5. Listening and troubleshooting

1. Select a pack → check the window status line: "Memory sounds ready" with a plausible file count and
   memory usage means it loaded successfully.
2. Press a few keys → the "submissions" (提交) counter should increase; if it does not, key presses are
   not reaching the sound pack.
3. The counter rises but there is no sound → check system volume, the output device, and this app's
   volume in the volume mixer.
4. No sound inside an elevated program → Windows does not allow a lower-privilege program to send input
   into a higher-privilege window; run this program as administrator too.
5. No sound after moving the whole directory → pack paths are relative to the program directory, so
   `sounds\` must move together with `keysound.exe`.

## 6. Minimal self-check list

- [ ] The directory is under `sounds\`, and its name may differ from `name`
- [ ] `index.json` is UTF-8 and contains both `name` and `mode`
- [ ] `mode` is `assign`, `repeat`, `random`, or `mixed` (or `指定`, `重复`, `随机`, `混合`)
- [ ] Every `sound` is only a file name, and the file exists under `<pack directory>\sounds\` or directly under `<pack directory>\`
- [ ] Files are `.wav` or `.mp3`, each at most 16 MiB and the pack at most 128 MiB total
- [ ] Restarting the program shows the pack name in the menu

If everything above is satisfied and there is still no sound, check the error code in the window status
line and compare it with the troubleshooting steps above.
