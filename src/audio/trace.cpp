#include "audio/trace.h"
#include <cstdio>
#include <cstring>
#include <string>

namespace keysound::trace {

namespace {

struct Slot {
    std::atomic<unsigned> seq{0};
    Record r{};
};

Slot slots[ringSize];
std::atomic<unsigned> produced{0};
std::atomic<bool> running{false};
std::atomic<bool> closing{false};
std::thread writer;
std::FILE* file = nullptr;
std::int64_t ticksPerSecond = 0;
std::int64_t lostRecords = 0;

unsigned drainFrom = 0;

std::int64_t counter() {
    LARGE_INTEGER value{};
    QueryPerformanceCounter(&value);
    return value.QuadPart;
}

std::int64_t frequency() {
    LARGE_INTEGER value{};
    QueryPerformanceFrequency(&value);
    return value.QuadPart > 0 ? value.QuadPart : 1;
}

bool requested(const std::wstring& directory) {
    wchar_t buffer[8]{};
    const DWORD length = GetEnvironmentVariableW(L"KEYSOUND_TRACE", buffer, 8);
    if (length > 0 && length < 8 && buffer[0] != L'0') return true;
    const std::wstring marker = directory + L"\\keysound-trace.on";
    return GetFileAttributesW(marker.c_str()) != INVALID_FILE_ATTRIBUTES;
}

void emit(const Record& r) {
    if (!file) return;
    const double milliseconds = static_cast<double>(r.qpc) * 1000.0 / static_cast<double>(ticksPerSecond);
    std::fprintf(file, "%.3f,%s,%u,%u,%u,%lld,%lld\n", milliseconds, kindName(r.kind),
                 static_cast<unsigned>(r.vk), static_cast<unsigned>(r.flags),
                 static_cast<unsigned>(r.generation),
                 static_cast<long long>(r.a), static_cast<long long>(r.b));
}

void drain() {
    const unsigned target = produced.load(std::memory_order_acquire);
    if (target - drainFrom > ringSize) {
        lostRecords += static_cast<std::int64_t>(target - drainFrom - ringSize);
        drainFrom = target - ringSize;
    }
    while (drainFrom < target) {
        Slot& slot = slots[drainFrom & (ringSize - 1)];
        const unsigned first = slot.seq.load(std::memory_order_acquire);
        if (first & 1u) break;
        Record copy{};
        std::memcpy(&copy, &slot.r, sizeof(Record));
        const unsigned second = slot.seq.load(std::memory_order_acquire);
        if (first != second) break;
        emit(copy);
        ++drainFrom;
    }
    if (file) std::fflush(file);
}

void loop() {
    while (!closing.load()) {
        drain();
        Sleep(200);
    }
    drain();
    if (lostRecords > 0 && file)
        std::fprintf(file, "# lost=%lld\n", static_cast<long long>(lostRecords));
    if (file) {
        std::fflush(file);
        std::fclose(file);
        file = nullptr;
    }
}

}

const char* kindName(Kind kind) {
    switch (kind) {
    case Kind::Submit: return "submit";
    case Kind::Dequeue: return "dequeue";
    case Kind::DropNotReady: return "drop_not_ready";
    case Kind::DropLoading: return "drop_loading";
    case Kind::DropQueueFull: return "drop_queue_full";
    case Kind::DropStale: return "drop_stale";
    case Kind::DropNoPath: return "drop_no_path";
    case Kind::PlayBegin: return "play_begin";
    case Kind::PlayOk: return "play_ok";
    case Kind::PlayFail: return "play_fail";
    case Kind::VoiceCreate: return "voice_create";
    case Kind::BufferSubmit: return "buffer_submit";
    case Kind::VoiceStart: return "voice_start";
    case Kind::WarmBegin: return "warm_begin";
    case Kind::WarmEnd: return "warm_end";
    case Kind::PrewarmDropped: return "prewarm_dropped";
    case Kind::ShutdownDrop: return "shutdown_drop";
    case Kind::Heartbeat: return "heartbeat";
    case Kind::Voices: return "voices";
    }
    return "unknown";
}

std::int64_t now() { return counter(); }

bool enabled() { return running.load(); }

void start(const std::wstring& directory) {
    if (running.load()) return;
    ticksPerSecond = frequency();
    if (!requested(directory)) return;
    const std::wstring path = directory + L"\\keysound-trace.csv";
    file = _wfopen(path.c_str(), L"wb");
    if (!file) return;
    std::fprintf(file, "# keysound trace v1\n");
    std::fprintf(file, "# t_ms=%lld\n", static_cast<long long>(ticksPerSecond));
    std::fprintf(file, "t_ms,kind,vk,flags,generation,a,b\n");
    produced.store(0);
    drainFrom = 0;
    closing.store(false);
    running.store(true);
    writer = std::thread(loop);
}

void stop() {
    if (!running.load()) return;
    closing.store(true);
    if (writer.joinable()) writer.join();
    running.store(false);
}

void record(Kind kind, unsigned short vk, unsigned short flags, unsigned int generation,
            std::int64_t a, std::int64_t b) {
    if (!running.load(std::memory_order_relaxed)) return;
    const unsigned index = produced.fetch_add(1, std::memory_order_relaxed);
    Slot& slot = slots[index & (ringSize - 1)];
    const unsigned sequence = slot.seq.load(std::memory_order_relaxed);
    slot.seq.store(sequence + 1, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_release);
    slot.r.qpc = counter();
    slot.r.a = a;
    slot.r.b = b;
    slot.r.generation = generation;
    slot.r.vk = vk;
    slot.r.flags = flags;
    slot.r.kind = kind;
    slot.seq.store(sequence + 2, std::memory_order_release);
}

}
