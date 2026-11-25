#include "audio_backend.h"
#include "memory_audio.h"

namespace keysound {

namespace {

class MemoryAudioBackend final : public AudioBackend {
public:
    HRESULT initialize(bool enable_playback = true) override {
        return audio_.initialize(enable_playback);
    }

    HRESULT preload(const std::vector<std::wstring>& paths, const Cancel& cancel) override {
        return audio_.preload(paths, cancel);
    }

    HRESULT play(const std::wstring& path, PlayMode mode, float volume) override {
        const MemoryAudio::PlayMode target = (mode == PlayMode::interrupt)
            ? MemoryAudio::PlayMode::interrupt
            : MemoryAudio::PlayMode::overlap;
        return audio_.play(path, target, volume);
    }

    HRESULT reset() override { return audio_.reset(); }
    HRESULT shutdown() override { return audio_.shutdown(); }
    size_t cached_bytes() const override { return audio_.cached_bytes(); }
    size_t cached_count() const override { return audio_.cached_count(); }
    size_t voice_count() const override { return audio_.voice_count(); }

private:
    MemoryAudio audio_;
};

}

std::unique_ptr<AudioBackend> createAudioBackend() {
    return std::unique_ptr<AudioBackend>(new MemoryAudioBackend());
}

}
