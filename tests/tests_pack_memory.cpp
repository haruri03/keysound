#include "memory_audio.h"
#include "soundpack.h"
#include <cassert>
#include <chrono>
#include <iostream>
int main() {
    soundpack::Pack pack;
    assert(soundpack::load(L"sounds\\\u6210\u6b65\u5802\u97f3\u6548", pack));
    keysound::MemoryAudio audio;
    assert(SUCCEEDED(audio.initialize()));
    std::vector<std::wstring> paths;
    for (auto& item : pack.assigned) paths.push_back(item.second);
    auto before = std::chrono::steady_clock::now();
    HRESULT hr = audio.preload(paths);
    std::cout << "preload HRESULT=" << std::hex << (unsigned long)hr << std::dec << " files=" << audio.cached_count() << " bytes=" << audio.cached_bytes() << "\n";
    assert(SUCCEEDED(hr));
    auto loaded = std::chrono::steady_clock::now();
    for (int i = 0; i < 100; ++i) assert(SUCCEEDED(audio.play(paths.front())));
    auto played = std::chrono::steady_clock::now();
    assert(SUCCEEDED(audio.reset()));
    assert(audio.cached_bytes() == 0 && audio.voice_count() == 0);
    assert(FAILED(audio.play(paths.front())));
    assert(SUCCEEDED(audio.preload(paths)));
    assert(SUCCEEDED(audio.reset()));
    std::cout << "preload_ms=" << std::chrono::duration<double,std::milli>(loaded-before).count()
        << " submit_100_ms=" << std::chrono::duration<double,std::milli>(played-loaded).count()
        << " reset/reload PASS (not end-to-end audible latency)\n";
}
