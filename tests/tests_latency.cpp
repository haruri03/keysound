#include <windows.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <cassert>
#include <functional>
#include <memory>
#include "audio/audio_backend.h"
#include "audio/pack_player.h"
#include "audio/toggles.h"
#include "input/hooks.h"
#include "packs/registry.h"

static std::atomic<int> played{0}, resets{0};
static std::atomic<bool> blockDecode{false}, decoding{false};
static std::mutex playedMutex;
static std::wstring playedPath;

namespace {

class FakeBackend final : public keysound::AudioBackend {
public:
    HRESULT initialize(bool = true) override { return S_OK; }
    HRESULT preload(const std::vector<std::wstring>&, const Cancel& cancel) override {
        decoding = true;
        while (blockDecode && !(cancel && cancel())) std::this_thread::yield();
        decoding = false;
        return (cancel && cancel()) ? E_ABORT : S_OK;
    }
    HRESULT play(const std::wstring& path, PlayMode) override {
        { std::lock_guard<std::mutex> lock(playedMutex); playedPath = path; }
        ++played; return S_OK;
    }
    HRESULT reset() override { ++resets; return S_OK; }
    HRESULT shutdown() override { return reset(); }
    size_t cached_bytes() const override { return 8; }
    size_t cached_count() const override { return 1; }
    size_t voice_count() const override { return 0; }
};

}

namespace keysound {

std::unique_ptr<AudioBackend> createAudioBackend() {
    return std::unique_ptr<AudioBackend>(new FakeBackend());
}

}

static bool waitFor(const std::function<bool()>& predicate) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (!predicate() && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return predicate();
}

int main() {
    activePack.mode = L"\u6307\u5b9a";
    activePack.assigned[L"A"] = L"C:\\sound.mp3";
    packActive = true;
    KBDLLHOOKSTRUCT key{}; key.vkCode = 'A'; key.scanCode = 0x1e;
    packPlayer.start(); packPlayer.prewarm();
    assert(waitFor([] { return packPlayer.readyState.load(); }));
    LowLevelKeyboardProc(HC_ACTION, WM_KEYUP, reinterpret_cast<LPARAM>(&key));
    assert(played == 0);
    LowLevelKeyboardProc(HC_ACTION, WM_KEYDOWN, reinterpret_cast<LPARAM>(&key));
    assert(waitFor([] { return played == 1; }));
    for (int i = 0; i < 100; ++i)
        LowLevelKeyboardProc(HC_ACTION, WM_SYSKEYDOWN, reinterpret_cast<LPARAM>(&key));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(played == 1);
    LowLevelKeyboardProc(HC_ACTION, WM_KEYUP, reinterpret_cast<LPARAM>(&key));
    LowLevelKeyboardProc(HC_ACTION, WM_SYSKEYDOWN, reinterpret_cast<LPARAM>(&key));
    assert(waitFor([] { return played == 2; }));
    const int before = resets;
    packActive = false; packPlayer.prewarm();
    assert(waitFor([before] { return resets > before && !packPlayer.loading; }));
    assert(!packPlayer.readyState);
    blockDecode = true; packActive = true; packPlayer.prewarm();
    assert(waitFor([] { return decoding.load(); }));
    for (int i=0; i<100; ++i) packPlayer.submit(key, false);
    packActive = false; packPlayer.prewarm();
    assert(waitFor([] { return !decoding && !packPlayer.loading; }));
    blockDecode = false;
    packActive = true; packPlayer.prewarm();
    assert(waitFor([] { return packPlayer.readyState.load(); }));
    assert(played == 2 && packPlayer.queuedCount == 0);
    {
        std::lock_guard<std::mutex> lock(packMutex);
        activePack.mode = L"mixed";
        activePack.assigned.clear();
        activePack.assigned[L"A"] = L"assigned.wav";
        activePack.assigned[L"Mouse Left"] = L"mouse.wav";
        activePack.sounds = {L"fallback.wav"};
    }
    packPlayer.prewarm();
    assert(waitFor([] { return packPlayer.readyState.load(); }));
    LowLevelKeyboardProc(HC_ACTION, WM_KEYUP, reinterpret_cast<LPARAM>(&key));
    LowLevelKeyboardProc(HC_ACTION, WM_KEYDOWN, reinterpret_cast<LPARAM>(&key));
    assert(waitFor([] { return played == 3; }));
    { std::lock_guard<std::mutex> lock(playedMutex); assert(playedPath == L"assigned.wav"); }
    key.vkCode = 'B'; key.scanCode = 0x30;
    LowLevelKeyboardProc(HC_ACTION, WM_KEYDOWN, reinterpret_cast<LPARAM>(&key));
    assert(waitFor([] { return played == 4; }));
    { std::lock_guard<std::mutex> lock(playedMutex); assert(playedPath == L"fallback.wav"); }
    flag_mouse = true;
    MSLLHOOKSTRUCT mouse{};
    LowLevelMouseProc(HC_ACTION, WM_LBUTTONUP, reinterpret_cast<LPARAM>(&mouse));
    assert(waitFor([] { return played == 5; }));
    { std::lock_guard<std::mutex> lock(playedMutex); assert(playedPath == L"mouse.wav"); }
    LowLevelMouseProc(HC_ACTION, WM_RBUTTONUP, reinterpret_cast<LPARAM>(&mouse));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(played == 5);
    packPlayer.shutdown();
    assert(played == 5 && packPlayer.cachedBytes == 0);
    return 0;
}
