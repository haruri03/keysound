#!/usr/bin/env python3
import argparse
import csv
import statistics
import sys


def percentile(values, fraction):
    if not values:
        return 0.0
    ordered = sorted(values)
    index = int(round((len(ordered) - 1) * fraction))
    return ordered[max(0, min(index, len(ordered) - 1))]


def load(path):
    records = []
    header = {}
    with open(path, newline="", encoding="utf-8") as handle:
        for line in handle:
            if line.startswith("# keysound trace"):
                header["version"] = line.strip()
                continue
            if line.startswith("# t_ms="):
                header["frequency"] = float(line.split("=", 1)[1])
                continue
            if line.startswith("# lost="):
                header["lost"] = int(line.split("=", 1)[1])
                continue
            if line.startswith("#") or line.startswith("t_ms,"):
                continue
            if not line.strip():
                continue
            records.append(line.rstrip("\n"))
    rows = []
    for line in csv.reader(records):
        if len(line) < 7:
            continue
        rows.append({
            "t": float(line[0]),
            "kind": "".join(part.capitalize() for part in line[1].split("_")),
            "vk": int(line[2]),
            "flags": int(line[3]),
            "generation": int(line[4]),
            "a": float(line[5]),
            "b": float(line[6]),
        })
    frequency = header.get("frequency", 10000000.0) or 10000000.0
    for row in rows:
        row["ms"] = row["t"]
        row["a_ms"] = row["a"] / frequency * 1000.0
        row["b_ms"] = row["b"] / frequency * 1000.0
    return rows, header


def summarize(rows, header, slow_ms, heartbeat_ms=250.0):
    counts = {}
    for row in rows:
        counts[row["kind"]] = counts.get(row["kind"], 0) + 1

    submits = counts.get("Submit", 0)
    plays = counts.get("PlayOk", 0) + counts.get("PlayFail", 0)
    drops = {name: value for name, value in counts.items() if name.startswith("Drop")}
    drops["PrewarmDropped"] = counts.get("PrewarmDropped", 0)
    drops["ShutdownDrop"] = counts.get("ShutdownDrop", 0)
    dropped = sum(drops.values())

    plays_ok = [row for row in rows if row["kind"] == "PlayOk"]
    plays_fail = [row for row in rows if row["kind"] == "PlayFail"]
    queue_wait = [row["b_ms"] - row["a_ms"] for row in rows if row["kind"] == "Dequeue" and row["a_ms"]]
    backend = [row["b_ms"] - row["a_ms"] for row in plays_ok + plays_fail if row["b_ms"] and row["a_ms"]]
    warm = [row["b_ms"] - row["a_ms"] for row in rows if row["kind"] == "WarmEnd" and row["b_ms"] >= row["a_ms"]]

    print("keysound trace report")
    print("  file version :", header.get("version", "# keysound trace ?"))
    print("  lost records :", header.get("lost", 0))
    print(f"  records      : {len(rows)}")
    print(f"  submits      : {submits}")
    print(f"  dropped      : {dropped}")
    if submits:
        print(f"  drop rate    : {dropped / submits * 100.0:.2f}%")
    print(f"  play ok/fail : {counts.get('PlayOk', 0)} / {counts.get('PlayFail', 0)}")

    if drops and any(drops.values()):
        print("\n  drop reasons")
        for name, value in sorted(drops.items(), key=lambda item: -item[1]):
            if value:
                print(f"    {name:16s} {value}")

    def stats(label, values):
        if not values:
            print(f"\n  {label}: no samples")
            return
        print(f"\n  {label} (ms)")
        print(f"    n={len(values)} min={min(values):.2f} p50={percentile(values, 0.5):.2f} "
              f"p95={percentile(values, 0.95):.2f} p99={percentile(values, 0.99):.2f} "
              f"max={max(values):.2f} mean={statistics.fmean(values):.2f}")

    stats("queue wait (submit -> dequeue)", queue_wait)
    stats("backend play", backend)
    stats("pack warm", warm)

    heartbeats = [row for row in rows if row["kind"] == "Heartbeat"]
    if len(heartbeats) > 1:
        beats = [heartbeats[i]["ms"] - heartbeats[i - 1]["ms"] for i in range(1, len(heartbeats))]
        stuck = [gap for gap in beats if gap >= heartbeat_ms]
        print(f"\n  hook heartbeat n={len(heartbeats)} interval p50={percentile(beats, 0.5):.1f} "
              f"p99={percentile(beats, 0.99):.1f} max={max(beats):.1f} ms; "
              f"gaps >= {heartbeat_ms:.0f} ms: {len(stuck)}")
        for gap in sorted(stuck, reverse=True)[:5]:
            print(f"    gap {gap:8.1f} ms")
    else:
        print("\n  hook heartbeat: none recorded (hook thread never had a chance to tick)")

    voices = [row for row in rows if row["kind"] == "Voices"]
    if voices:
        counts = [int(row["a_ms"]) for row in voices]
        print(f"  voices at play n={len(counts)} p50={percentile(counts, 0.5):.0f} max={max(counts)}")

    starts = [row for row in rows if row["kind"] == "Submit"]
    stalls = []
    last_ok = None
    for row in plays_ok:
        if last_ok is not None:
            gap = row["ms"] - last_ok["ms"]
            if gap >= slow_ms:
                stalls.append((last_ok["ms"], gap))
        last_ok = row
    if stalls:
        print(f"\n  stalls between audible plays >= {slow_ms:.0f} ms")
        for timestamp, gap in sorted(stalls, key=lambda item: -item[1])[:10]:
            print(f"    at {timestamp / 1000.0:9.3f} s  gap {gap:8.1f} ms")
    else:
        print(f"\n  no stalls >= {slow_ms:.0f} ms between plays")

    first = {}
    worst = []
    for row in rows:
        if row["kind"] == "Submit":
            first[(row["vk"], row["flags"], row["generation"])] = row
        elif row["kind"] in ("PlayOk", "PlayFail"):
            key = (row["vk"], row["flags"], row["generation"])
            start = first.pop(key, None)
            if start is not None:
                worst.append(((row["b_ms"] - row["a_ms"]) if row["a_ms"] else 0.0, row["ms"] - start["ms"], row["kind"], row["vk"]))
    pending = len(first)
    if worst:
        worst.sort(reverse=True)
        print("\n  slowest key-to-play (end to end ms)")
        for play_ms, total, kind, vk in worst[:10]:
            print(f"    vk=0x{vk:02X} {kind:8s} total={total:8.1f} backend={play_ms:7.1f}")
    if pending:
        print(f"\n  submits without any play record: {pending}")
        for key in list(first)[:10]:
            row = first[key]
            print(f"    vk=0x{row['vk']:02X} at {row['ms'] / 1000.0:.3f} s")


def main():
    parser = argparse.ArgumentParser(description="Summarize a keysound-trace.csv timeline")
    parser.add_argument("path", help="path to keysound-trace.csv")
    parser.add_argument("--slow", type=float, default=400.0, help="stall threshold in ms (default 400)")
    parser.add_argument("--heartbeat", type=float, default=250.0, help="hook heartbeat gap threshold in ms (default 250)")
    args = parser.parse_args()
    try:
        rows, header = load(args.path)
    except OSError as error:
        print(f"cannot read {args.path}: {error}", file=sys.stderr)
        return 1
    summarize(rows, header, args.slow, args.heartbeat)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
