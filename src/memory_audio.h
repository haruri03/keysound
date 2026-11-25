#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <xaudio2.h>
#include <wrl/client.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <new>
#include <string>
#include <vector>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "xaudio2.lib")

namespace keysound {
class MemoryAudio final {
public:
    using SoundId = std::uint64_t;
    using Cancel = std::function<bool()>;
    enum class PlayMode { overlap, interrupt };
    struct Limits {
        size_t max_file_bytes = 16u * 1024u * 1024u;
        size_t max_total_bytes = 128u * 1024u * 1024u;
        size_t max_files = 4096;
        size_t max_voices = 32;
        size_t max_read_calls = 100000;
        unsigned max_decode_seconds = 30;
    };
    MemoryAudio() = default;
    explicit MemoryAudio(Limits limits) : limits_(limits) {}
    MemoryAudio(const MemoryAudio&) = delete;
    MemoryAudio& operator=(const MemoryAudio&) = delete;
    MemoryAudio(MemoryAudio&&) = delete;
    MemoryAudio& operator=(MemoryAudio&&) = delete;
    ~MemoryAudio() { shutdown(); }

    HRESULT initialize(bool enable_playback = true) {
        if (owner_ && owner_ != GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
        if (ready_) return S_FALSE;
        if (!limits_.max_file_bytes || !limits_.max_total_bytes || !limits_.max_files ||
            !limits_.max_voices || limits_.max_voices > 1024 || !limits_.max_read_calls ||
            !limits_.max_decode_seconds) return E_INVALIDARG;
        owner_ = GetCurrentThreadId();
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hr)) { owner_ = 0; return hr; }
        com_ = true;
        hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
        if (FAILED(hr)) { shutdown(); return hr; }
        mf_ = true;
        format_.wFormatTag = WAVE_FORMAT_PCM; format_.nChannels = 2;
        format_.nSamplesPerSec = 48000; format_.wBitsPerSample = 16;
        format_.nBlockAlign = 4; format_.nAvgBytesPerSec = 192000;
        if (enable_playback) {
            hr = XAudio2Create(engine_.GetAddressOf());
            if (SUCCEEDED(hr)) hr = engine_->CreateMasteringVoice(&master_);
            if (FAILED(hr)) { shutdown(); return hr; }
        }
        ready_ = true;
        return S_OK;
    }

    HRESULT load(const std::wstring& path, SoundId& id, const Cancel& cancel = {}) {
        id = 0;
        HRESULT hr = check(); if (FAILED(hr)) return hr;
        try {
            if (cancel && cancel()) return cancelled();
            std::wstring key; hr = normalize(path, key); if (FAILED(hr)) return hr;
            auto found = cache_.find(key);
            if (found != cache_.end()) { id = found->second.id; return S_OK; }
            if (cache_.size() >= limits_.max_files || bytes_ >= limits_.max_total_bytes)
                return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            const size_t budget = (std::min)({limits_.max_file_bytes,
                limits_.max_total_bytes - bytes_, static_cast<size_t>(XAUDIO2_MAX_BUFFER_BYTES)});
            Clip clip; hr = decode(key, budget, cancel, clip.pcm); if (FAILED(hr)) return hr;
            if (cancel && cancel()) return cancelled();
            if (next_id_ == (std::numeric_limits<SoundId>::max)()) return E_UNEXPECTED;
            clip.id = ++next_id_;
            const size_t allocated = clip.pcm.capacity();
            auto inserted = cache_.emplace(std::move(key), std::move(clip));
            bytes_ += allocated; id = inserted.first->second.id;
            return S_OK;
        } catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
          catch (...) { return E_FAIL; }
    }
    HRESULT preload(const std::vector<std::wstring>& paths, const Cancel& cancel = {}) {
        HRESULT hr = check(); if (FAILED(hr)) return hr;
        for (const auto& path : paths) { SoundId id; hr = load(path, id, cancel); if (FAILED(hr)) return hr; }
        return S_OK;
    }
    HRESULT find(const std::wstring& path, SoundId& id) const {
        id = 0; HRESULT hr = check(); if (FAILED(hr)) return hr;
        try {
            std::wstring key; hr = normalize(path, key); if (FAILED(hr)) return hr;
            auto i = cache_.find(key); if (i == cache_.end()) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            id = i->second.id; return S_OK;
        } catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    }
    HRESULT play(const std::wstring& path, PlayMode mode = PlayMode::overlap, float volume = 1.0f) {
        SoundId id; HRESULT hr = find(path, id); return FAILED(hr) ? hr : play(id, mode, volume);
    }
    HRESULT play(SoundId id, PlayMode mode = PlayMode::overlap, float volume = 1.0f) {
        HRESULT hr = check(); if (FAILED(hr)) return hr;
        if (!engine_) return HRESULT_FROM_WIN32(ERROR_NOT_READY);
        if (!std::isfinite(volume) || volume < 0 || volume > 1) return E_INVALIDARG;
        auto clip = std::find_if(cache_.begin(), cache_.end(), [id](const auto& p) { return p.second.id == id; });
        if (clip == cache_.end()) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        if (mode == PlayMode::interrupt) clear_voices();
        IXAudio2SourceVoice* voice = nullptr;
        for (auto i = voices_.begin(); i != voices_.end(); ++i) {
            XAUDIO2_VOICE_STATE state{}; i->voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
            if (!state.BuffersQueued) { voice = i->voice; voices_.erase(i); break; }
        }
        if (!voice && voices_.size() >= limits_.max_voices) {
            voices_.front().voice->DestroyVoice(); voices_.erase(voices_.begin());
        }
        try { voices_.reserve(limits_.max_voices); } catch (const std::bad_alloc&) {
            if (voice) voice->DestroyVoice(); return E_OUTOFMEMORY;
        }
        if (!voice) { hr = engine_->CreateSourceVoice(&voice, &format_); if (FAILED(hr)) return hr; }
        XAUDIO2_BUFFER buffer{}; buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.AudioBytes = static_cast<UINT32>(clip->second.pcm.size()); buffer.pAudioData = clip->second.pcm.data();
        hr = voice->SetVolume(volume);
        if (SUCCEEDED(hr)) hr = voice->SubmitSourceBuffer(&buffer);
        if (SUCCEEDED(hr)) hr = voice->Start();
        if (FAILED(hr)) { voice->DestroyVoice(); return hr; }
        voices_.push_back({voice}); return S_OK;
    }
    HRESULT reset() noexcept {
        if (owner_ && owner_ != GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
        clear_voices();
        cache_.clear(); bytes_ = 0; return S_OK;
    }
    HRESULT shutdown() noexcept {
        if (owner_ && owner_ != GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
        reset();
        if (master_) { master_->DestroyVoice(); master_ = nullptr; }
        engine_.Reset();
        if (mf_) { MFShutdown(); mf_ = false; }
        if (com_) { CoUninitialize(); com_ = false; }
        ready_ = false; owner_ = 0; return S_OK;
    }
    size_t cached_bytes() const noexcept { return bytes_; }
    size_t cached_count() const noexcept { return cache_.size(); }
    size_t voice_count() const noexcept { return voices_.size(); }
private:
    template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
    struct PathLess {
        bool operator()(const std::wstring& a, const std::wstring& b) const noexcept {
            return CompareStringOrdinal(a.data(), static_cast<int>(a.size()), b.data(), static_cast<int>(b.size()), TRUE) == CSTR_LESS_THAN;
        }
    };
    struct Clip { SoundId id = 0; std::vector<BYTE> pcm; };
    struct Voice { IXAudio2SourceVoice* voice; };
    HRESULT check() const noexcept {
        if (owner_ && owner_ != GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
        return ready_ ? S_OK : HRESULT_FROM_WIN32(ERROR_NOT_READY);
    }
    static HRESULT cancelled() noexcept { return HRESULT_FROM_WIN32(ERROR_CANCELLED); }
    static HRESULT normalize(const std::wstring& path, std::wstring& key) {
        if (path.empty() || path.find(L'\0') != std::wstring::npos || path.size() > 32767) return E_INVALIDARG;
        DWORD count = GetFullPathNameW(path.c_str(), 0, nullptr, nullptr);
        if (!count || count > 32768) return E_INVALIDARG;
        std::vector<wchar_t> storage(count);
        DWORD length = GetFullPathNameW(path.c_str(), count, storage.data(), nullptr);
        if (!length || length >= count) return E_INVALIDARG;
        key.assign(storage.data(), length); std::replace(key.begin(), key.end(), L'/', L'\\');
        if (key.compare(0, 4, L"\\\\.\\") == 0 || key.compare(0, 4, L"\\\\?\\") == 0 ||
            key.find(L':', 2) != std::wstring::npos) return E_INVALIDARG;
        return S_OK;
    }
    static HRESULT decode(const std::wstring& path, size_t budget, const Cancel& cancel,
                          std::vector<BYTE>& pcm, const Limits& limits) {
        const auto started = std::chrono::steady_clock::now();
        auto guard = [&]() -> HRESULT {
            if (cancel && cancel()) return cancelled();
            if (std::chrono::steady_clock::now() - started > std::chrono::seconds(limits.max_decode_seconds))
                return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
            return S_OK;
        };
        HRESULT hr = guard(); if (FAILED(hr)) return hr;
        DWORD attrs = GetFileAttributesW(path.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) return HRESULT_FROM_WIN32(GetLastError());
        if (attrs & FILE_ATTRIBUTE_DIRECTORY) return E_INVALIDARG;
        ComPtr<IMFSourceReader> reader;
        hr = MFCreateSourceReaderFromURL(path.c_str(), nullptr, reader.GetAddressOf()); if (FAILED(hr)) return hr;
        hr = guard(); if (FAILED(hr)) return hr;
        hr = reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE); if (FAILED(hr)) return hr;
        hr = reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), TRUE); if (FAILED(hr)) return hr;
        ComPtr<IMFMediaType> type; hr = MFCreateMediaType(type.GetAddressOf()); if (FAILED(hr)) return hr;
        if (FAILED(hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio)) ||
            FAILED(hr = type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM)) ||
            FAILED(hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2)) ||
            FAILED(hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 48000)) ||
            FAILED(hr = type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16)) ||
            FAILED(hr = type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 4)) ||
            FAILED(hr = type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 192000))) return hr;
        hr = reader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, type.Get()); if (FAILED(hr)) return hr;
        auto valid_format = [&]() -> HRESULT {
            ComPtr<IMFMediaType> actual;
            HRESULT result = reader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), actual.GetAddressOf());
            if (FAILED(result)) return result;
            GUID subtype{}; UINT32 rate = 0, channels = 0, bits = 0, block = 0;
            if (FAILED(actual->GetGUID(MF_MT_SUBTYPE, &subtype)) || subtype != MFAudioFormat_PCM ||
                FAILED(actual->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &rate)) || rate != 48000 ||
                FAILED(actual->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels)) || channels != 2 ||
                FAILED(actual->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bits)) || bits != 16 ||
                FAILED(actual->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &block)) || block != 4) return MF_E_INVALIDMEDIATYPE;
            return S_OK;
        };
        hr = valid_format(); if (FAILED(hr)) return hr;
        for (size_t calls = 0; calls < limits.max_read_calls; ++calls) {
            hr = guard(); if (FAILED(hr)) return hr;
            DWORD flags = 0; ComPtr<IMFSample> sample;
            hr = reader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr, &flags, nullptr, sample.GetAddressOf());
            if (FAILED(hr)) return hr;
            hr = guard(); if (FAILED(hr)) return hr;
            if (flags & MF_SOURCE_READERF_ERROR) return E_FAIL;
            if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) { hr = valid_format(); if (FAILED(hr)) return hr; }
            if (sample) {
                DWORD sample_bytes = 0;
                hr = sample->GetTotalLength(&sample_bytes); if (FAILED(hr)) return hr;
                if (sample_bytes > budget - pcm.size()) return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                ComPtr<IMFMediaBuffer> buffer; hr = sample->ConvertToContiguousBuffer(buffer.GetAddressOf()); if (FAILED(hr)) return hr;
                BYTE* data = nullptr; DWORD length = 0; hr = buffer->Lock(&data, nullptr, &length); if (FAILED(hr)) return hr;
                struct Unlock { IMFMediaBuffer* b; ~Unlock() { b->Unlock(); } } unlock{buffer.Get()};
                if (length % 4) return MF_E_INVALIDMEDIATYPE;
                if (length > budget - pcm.size()) return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                if (length) {
                    const size_t needed = pcm.size() + length;
                    if (needed > pcm.capacity()) {
                        const size_t growth = (std::min)(budget, (std::max)(needed, pcm.capacity() * 2));
                        pcm.reserve(growth);
                        if (pcm.capacity() > budget) return E_OUTOFMEMORY;
                    }
                    pcm.insert(pcm.end(), data, data + length);
                }
            }
            if (flags & MF_SOURCE_READERF_ENDOFSTREAM) return pcm.empty() ? MF_E_INVALID_FILE_FORMAT : S_OK;
        }
        return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
    }
    HRESULT decode(const std::wstring& path, size_t budget, const Cancel& cancel, std::vector<BYTE>& pcm) {
        return decode(path, budget, cancel, pcm, limits_);
    }
    void clear_voices() noexcept { for (auto& v : voices_) v.voice->DestroyVoice(); voices_.clear(); }
    Limits limits_;
    DWORD owner_ = 0;
    bool com_ = false, mf_ = false, ready_ = false;
    WAVEFORMATEX format_{};
    ComPtr<IXAudio2> engine_;
    IXAudio2MasteringVoice* master_ = nullptr;
    std::vector<Voice> voices_;
    std::map<std::wstring, Clip, PathLess> cache_;
    size_t bytes_ = 0;
    SoundId next_id_ = 0;
};
}
