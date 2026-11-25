# 延迟追踪

[English](tracing.md) · **简体中文**

> 按键延迟、没声音、或者一串按键挤在一起播放时读这份文档。范围：
> `src/audio/trace.*`、音效包播放器和 `tools/trace-report.py`。

平时追踪是关闭的：`trace::record` 立刻返回，钩子线程只多一次原子读。
打开后每个事件多一次原子自增和几次赋值，**绝不在钩子线程做文件 IO**；
另一个写线程每 200 ms 把环形缓冲刷到磁盘。

## 打开方式

二选一：在程序目录新建 `keysound-trace.on` 文件，或者启动前设置环境变量
`KEYSOUND_TRACE=1`。追踪在 `main()` 里 `packPlayer.start()` 之后开始，
退出前停止，并写出最后的 `# lost=` 行。

结果写到程序目录下的 `keysound-trace.csv`。删掉标记文件或环境变量即可恢复零开销。

## 格式

```
# keysound trace v1
# t_ms=10000000
t_ms,kind,vk,flags,generation,a,b
```

`t_ms` 是 QPC 时间戳（毫秒），`# t_ms=<值>` 是 QPC 频率，可换算回秒。`kind` 取值：

| kind | 含义 |
|---|---|
| `Submit` | 一次物理按下到达 `PackPlayer::submit`（`a`=钩子时刻，`b`=1 表示鼠标） |
| `Dequeue` | 工作线程取出事件（`a`=钩子时刻，`b`=出队时刻） |
| `DropNotReady` / `DropLoading` | 音效包还没预热完成就按了键 |
| `DropQueueFull` | 队列超过 64 条（`b`=当时队列长度） |
| `DropStale` | 事件被切换音效包作废 |
| `DropNoPath` | 解析出的音频路径为空，根本没尝试播放 |
| `PlayBegin` / `PlayOk` / `PlayFail` | 后端播放耗时（`a`=开始，`b`=持续） |
| `WarmBegin` / `WarmEnd` | 音效包解码耗时（`a`=开始，`b`=持续） |
| `PrewarmDropped` / `ShutdownDrop` | 切换音效包或退出时丢弃的排队事件 |
| `VoiceCreate` / `BufferSubmit` / `VoiceStart` | 预留给 XAudio2 层的细分耗时 |

`vk` 是虚拟键码，`flags` 是钩子标志，`generation` 是事件所属的音效包代次。

## 生成报告

```
python tools/trace-report.py keysound-trace.csv
python tools/trace-report.py keysound-trace.csv --slow 250
```

报告先给出提交/丢弃数量和丢弃原因分布，再给出队列等待（提交→出队）、
后端播放、音效包预热三项的百分位耗时，以及两次可听播放之间最长的停顿和
最慢的「按下→出声」配对。

## 怎么读

- `DropNotReady`/`DropLoading` 集中在切换音效包之后，说明预热还没跑完；
  刚启动的头几次按键本来就会丢。
- `DropQueueFull` 说明工作线程跟不上：先看 `backend play`，别急着怪钩子。
- 有「submits without any play record」但没有丢弃记录，说明事件被消费却没出声，
  多半是 `DropNoPath`（`index.json` 的映射问题，不是延迟）。
- `backend play` 尾巴很长而 `queue wait` 正常，问题在音频设备或 voice 创建，
  不在键盘钩子。


## 钩子心跳与 voice 数

多两种事件，让「没按键时也可能卡住」变得可见：

| kind | 含义 | 列 |
| --- | --- | --- |
| `heartbeat` | 钩子线程自己的 100 ms 定时器触发 | `a` = 触发时刻 |
| `voices` | 每次播放调用后存活的 source voice 数 | `a` = voice 数 |

`heartbeat` 间隔超过 250 ms 说明钩子线程被饿住，那段时间按下的键根本没进播放器。
`voices` 接近 32 说明撞上 32 路上限、最旧的 voice 被销毁，可能截断声音或产生爆音。

`python tools/trace-report.py keysound-trace.csv --heartbeat 250` 会打印这两项。
