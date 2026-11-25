#pragma once
#include <windows.h>
#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

namespace keysound::trace {

enum class Kind : unsigned char {
    Submit = 1,
    Dequeue,
    DropNotReady,
    DropLoading,
    DropQueueFull,
    DropStale,
    DropNoPath,
    PlayBegin,
    PlayOk,
    PlayFail,
    VoiceCreate,
    BufferSubmit,
    VoiceStart,
    WarmBegin,
    WarmEnd,
    PrewarmDropped,
    ShutdownDrop,
    Heartbeat,
    Voices
};

struct Record {
    std::int64_t qpc = 0;
    std::int64_t a = 0;
    std::int64_t b = 0;
    unsigned int generation = 0;
    unsigned short vk = 0;
    unsigned short flags = 0;
    Kind kind = Kind::Submit;
};

constexpr unsigned ringSize = 1u << 16;

void start(const std::wstring& directory);
void stop();
bool enabled();
std::int64_t now();

void record(Kind kind, unsigned short vk, unsigned short flags, unsigned int generation,
            std::int64_t a = 0, std::int64_t b = 0);

const char* kindName(Kind kind);

}
