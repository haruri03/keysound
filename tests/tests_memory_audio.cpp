#include "memory_audio.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

static void wav(const std::filesystem::path& path, unsigned frames = 4800) {
    std::ofstream out(path, std::ios::binary);
    auto u16 = [&](unsigned n) { char b[2] = {char(n), char(n >> 8)}; out.write(b, 2); };
    auto u32 = [&](unsigned n) { u16(n); u16(n >> 16); };
    out.write("RIFF", 4); u32(36 + frames * 4); out.write("WAVEfmt ", 8);
    u32(16); u16(1); u16(2); u32(48000); u32(192000); u16(4); u16(16);
    out.write("data", 4); u32(frames * 4);
    for (unsigned i = 0; i < frames; ++i) { u16(0); u16(0); }
    assert(out.good());
}
int main(int argc, char** argv) {
    using namespace keysound;
    if (argc > 1 && std::string(argv[1]) == "--contracts-only") {
        MemoryAudio cold; MemoryAudio::SoundId id = 99;
        assert(FAILED(cold.play(1)));
        assert(FAILED(cold.load(L"unused.wav", id)) && id == 0);
        assert(SUCCEEDED(cold.reset())); assert(SUCCEEDED(cold.shutdown()));
        MemoryAudio::Limits invalid; invalid.max_voices = 0;
        MemoryAudio rejected(invalid); assert(rejected.initialize(false) == E_INVALIDARG);
        MemoryAudio owner; assert(SUCCEEDED(owner.initialize(false)));
        assert(owner.load(L"unused.wav", id, [] { return true; }) == HRESULT_FROM_WIN32(ERROR_CANCELLED));
        assert(owner.preload({L"unused.wav"}, [] { return true; }) == HRESULT_FROM_WIN32(ERROR_CANCELLED));
        assert(owner.cached_bytes() == 0 && owner.cached_count() == 0);
        HRESULT other = S_OK;
        std::thread wrong([&] { other = owner.reset(); }); wrong.join();
        assert(other == RPC_E_WRONG_THREAD);
        assert(SUCCEEDED(owner.shutdown()));
        assert(SUCCEEDED(owner.initialize(false))); assert(SUCCEEDED(owner.shutdown()));
        std::cout << "memory_audio non-I/O contracts passed\n"; return 0;
    }
    auto dir = std::filesystem::path(L"build") / (L"memory-audio-" + std::to_wstring(GetCurrentProcessId()));
    assert(std::filesystem::create_directory(dir));
    struct Cleanup { std::filesystem::path p; ~Cleanup() { std::error_code ec; std::filesystem::remove_all(p, ec); } } cleanup{dir};
    auto path = dir / L"音效.wav"; wav(path);
    MemoryAudio audio;
    assert(SUCCEEDED(audio.initialize(false)));
    MemoryAudio::SoundId id = 0, again = 0;
    HRESULT loaded = audio.load(path.wstring(), id); std::cerr << "load HRESULT=0x" << std::hex << static_cast<unsigned long>(loaded) << std::dec << '\n';
    assert(SUCCEEDED(loaded));
    assert(id != 0 && audio.cached_count() == 1 && audio.cached_bytes() >= 19200);
    assert(SUCCEEDED(audio.load((dir / L"." / L"音效.wav").wstring(), again)) && id == again);
    assert(audio.cached_count() == 1);
    assert(audio.load(path.wstring(), again, [] { return true; }) == HRESULT_FROM_WIN32(ERROR_CANCELLED));
    assert(FAILED(audio.play(id)));
    assert(FAILED(audio.load((dir / L"missing.mp3").wstring(), again)));
    assert(again == 0);
    HRESULT otherThread = S_OK;
    std::thread t([&] { MemoryAudio::SoundId unused; otherThread = audio.load(path.wstring(), unused); }); t.join();
    assert(otherThread == RPC_E_WRONG_THREAD);
    assert(SUCCEEDED(audio.reset()));
    assert(audio.cached_count() == 0 && audio.cached_bytes() == 0);
    assert(FAILED(audio.play(id)));
    assert(SUCCEEDED(audio.load(path.wstring(), again)) && again != id);
    assert(SUCCEEDED(audio.shutdown())); assert(SUCCEEDED(audio.shutdown()));
    MemoryAudio::Limits limits; limits.max_file_bytes = 1024; limits.max_total_bytes = 2048;
    MemoryAudio limited(limits); assert(SUCCEEDED(limited.initialize(false)));
    assert(FAILED(limited.load(path.wstring(), id))); assert(limited.cached_count() == 0);
    assert(limited.preload({path.wstring()}, [] { return true; }) == HRESULT_FROM_WIN32(ERROR_CANCELLED));
    assert(SUCCEEDED(limited.shutdown()));
    if (argc > 1 && std::string(argv[1]) == "--audio") {
        MemoryAudio live; assert(SUCCEEDED(live.initialize())); assert(SUCCEEDED(live.load(path.wstring(), id)));
        for (int i = 0; i < 100; ++i) assert(SUCCEEDED(live.play(id)));
        assert(live.voice_count() <= 32);
        assert(SUCCEEDED(live.play(id, MemoryAudio::PlayMode::interrupt)));
        assert(live.voice_count() == 1);
        assert(SUCCEEDED(live.reset())); assert(live.cached_bytes() == 0 && live.voice_count() == 0);
    }
    std::cout << "memory_audio tests passed (decode/lifecycle; audible output not asserted)\n";
}
