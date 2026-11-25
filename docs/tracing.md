# Latency tracing

**English** · [简体中文](tracing_cn.md)

> Read this when a key press is late, silent, or repeats as a stutter. Scope:
> `src/audio/trace.*`, the pack player, and `tools/trace-report.py`.

The player normally runs with tracing off: `trace::record` returns
immediately, so the hook thread pays one atomic read. Turning tracing on adds
one atomic increment plus a few stores per event and never performs file I/O
on the hook thread; a separate writer thread flushes the ring to disk every
200 ms.

## Enabling

Either create `<install directory>\keysound-trace.on`, or set the environment
variable `KEYSOUND_TRACE=1` before starting the program. Tracing starts in
`main()` after `packPlayer.start()` and stops before shutdown, which writes
the final `# lost=` line.

The output is `<install directory>\keysound-trace.csv`. Delete the marker file
or the variable to return to zero overhead.

## Format

```
# keysound trace v1
# t_ms=10000000
t_ms,kind,vk,flags,generation,a,b
```

`t_ms` is a QPC timestamp in milliseconds. `# t_ms=<value>` is the QPC
frequency, so raw values can be converted back to seconds. `kind` is one of:

| kind | meaning |
|---|---|
| `Submit` | a physical press reached `PackPlayer::submit` (`a`=hook time, `b`=1 for mouse) |
| `Dequeue` | the worker took the event (`a`=hook time, `b`=dequeue time) |
| `DropNotReady` / `DropLoading` | the press arrived before the pack was warmed |
| `DropQueueFull` | more than 64 presses queued (`b`=queue length) |
| `DropStale` | the event was superseded by a pack switch |
| `DropNoPath` | the resolved sound path was empty; no play was attempted |
| `PlayBegin` / `PlayOk` / `PlayFail` | backend call span (`a`=start, `b`=duration) |
| `WarmBegin` / `WarmEnd` | pack decode span (`a`=start, `b`=duration) |
| `PrewarmDropped` / `ShutdownDrop` | queued events discarded by a pack switch or shutdown |
| `VoiceCreate` / `BufferSubmit` / `VoiceStart` | reserved for XAudio2 level timing |

`vk` is the virtual key code, `flags` the hook flags, `generation` the pack
generation the event belonged to.

## Reading it

```
python tools/trace-report.py keysound-trace.csv
python tools/trace-report.py keysound-trace.csv --slow 250
```

The report prints submit/drop counts and drop reasons, then percentile
latency for queue wait (submit to dequeue), backend play, and pack warm,
plus the widest gaps between audible plays and the slowest key-to-play pairs.
A press that is slow but present shows up in `queue wait` or `backend play`;
a press that never plays shows up as a drop reason or in
`submits without any play record`.

## Reading the result

- `DropNotReady`/`DropLoading` clustered right after a pack switch means the
  warm-up is still running; the first presses after startup are expected to
  drop.
- `DropQueueFull` means the worker cannot keep up: check `backend play`
  before assuming the hook is slow.
- Non-zero `submits without any play record` with no drop rows means the
  worker consumed the event and produced no sound, which points at
  `DropNoPath` (a mapping problem in `index.json`, not latency).
- A large `backend play` tail with normal `queue wait` points at the audio
  device or at voice creation, not at the keyboard hook.


## Hook heartbeat and voice count

Two extra events make a hang visible even when no key is pressed:

| kind | meaning | columns |
| --- | --- | --- |
| `heartbeat` | the hook thread's own 100 ms timer fired | `a` = tick time |
| `voices` | source voices alive right after a play call | `a` = voice count |

`heartbeat` gaps over 250 ms mean the hook thread was starved, so keys pressed in that
window never reached the player. `voices` near 32 means the 32-voice ceiling forced the
oldest voice to be destroyed, which can cut a sound or make it click.

`python tools/trace-report.py keysound-trace.csv --heartbeat 250` prints both.
