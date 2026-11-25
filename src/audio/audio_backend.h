#pragma once
#include <windows.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace keysound {

class AudioBackend {
public:
    using Cancel = std::function<bool()>;
    enum class PlayMode { overlap, interrupt };
    virtual ~AudioBackend() = default;
    virtual HRESULT initialize(bool enable_playback = true) = 0;
    virtual HRESULT preload(const std::vector<std::wstring>& paths, const Cancel& cancel) = 0;
    virtual HRESULT play(const std::wstring& path, PlayMode mode, float volume) = 0;
    virtual HRESULT reset() = 0;
    virtual HRESULT shutdown() = 0;
    virtual size_t cached_bytes() const = 0;
    virtual size_t cached_count() const = 0;
    virtual size_t voice_count() const = 0;
};

std::unique_ptr<AudioBackend> createAudioBackend();

}
