#pragma once
#include <windows.h>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "soundpack.h"
#include "audio/audio_backend.h"
#include "audio/toggles.h"
#include "audio/trace.h"
#include "packs/registry.h"
using namespace std;
class PackPlayer {
    using Audio = keysound::AudioBackend;
    struct Event {
        KBDLLHOOKSTRUCT key{};
        bool interrupt = false;
        bool mouse = false;
        unsigned long generation = 0;
        bool warm = false;
        ULONGLONG submitted = 0;
        std::int64_t stamp = 0;
    };
    mutex queueMutex;
    condition_variable wake;
    deque<Event> queue;
    thread worker;
    atomic<bool> stopping{false};
    atomic<unsigned long> readyGeneration{0};

    bool cancelled(unsigned long generation) const {
        return stopping.load() || generation != packGeneration.load() || !packActive.load();
    }
    void observe(Audio& audio) {
        cachedBytes.store(audio.cached_bytes());
        cachedCount.store(audio.cached_count());
        voiceCount.store(audio.voice_count());
    }
    HRESULT reset(Audio& audio) {
        HRESULT hr = audio.reset();
        observe(audio);
        return hr;
    }
    std::int64_t warmStarted = 0;
    void finishWarm(unsigned long generation, HRESULT hr, bool loaded) {
        lock_guard<mutex> lock(queueMutex);
        if (keysound::trace::enabled()) keysound::trace::record(keysound::trace::Kind::WarmEnd, 0, 0, static_cast<unsigned int>(generation), warmStarted, keysound::trace::now());
        if (stopping.load() || generation != packGeneration.load()) return;
        lastError.store(hr);
        if (FAILED(hr)) errorCount.fetch_add(1);
        readyGeneration.store(generation);
        readyState.store(loaded && packActive.load());
        loading.store(false);
    }
    void warm(Audio& audio, bool& initialized, soundpack::Pack& snapshot, unsigned long generation) {
        warmStarted = keysound::trace::now();
        if (keysound::trace::enabled()) keysound::trace::record(keysound::trace::Kind::WarmBegin, 0, 0, static_cast<unsigned int>(generation), warmStarted, 0);
        HRESULT hr = reset(audio);
        snapshot = soundpack::Pack{};
        if (FAILED(hr)) { finishWarm(generation, hr, false); return; }
        if (cancelled(generation)) { finishWarm(generation, S_OK, false); return; }
        {
            lock_guard<mutex> lock(packMutex);
            if (!cancelled(generation)) snapshot = activePack;
        }
        if (cancelled(generation)) { finishWarm(generation, S_OK, false); return; }
        vector<wstring> paths;
        const auto mode = soundpack::modeKind(snapshot.mode);
        if (mode == soundpack::Mode::Assign || mode == soundpack::Mode::Mixed)
            for (const auto& item : snapshot.assigned) paths.push_back(item.second);
        if (mode == soundpack::Mode::Mixed || mode == soundpack::Mode::Random)
            paths.insert(paths.end(), snapshot.sounds.begin(), snapshot.sounds.end());
        if (mode == soundpack::Mode::Repeat && !snapshot.repeat.empty())
            paths.push_back(soundpack::resourcePath(snapshot.directory, snapshot.repeat));
        if (paths.empty()) { finishWarm(generation, HRESULT_FROM_WIN32(ERROR_NOT_FOUND), false); return; }
        if (!initialized) {
            hr = audio.initialize();
            if (FAILED(hr)) { finishWarm(generation, hr, false); return; }
            initialized = true;
        }
        hr = audio.preload(paths, [this, generation] { return cancelled(generation); });
        observe(audio);
        if (FAILED(hr) || cancelled(generation)) {
            HRESULT resetHr = reset(audio);
            snapshot = soundpack::Pack{};
            if (FAILED(resetHr)) hr = resetHr;
            finishWarm(generation, hr, false);
            return;
        }
        finishWarm(generation, S_OK, true);
    }
    void run() {
        unique_ptr<Audio> ownedAudio = keysound::createAudioBackend(); Audio& audio = *ownedAudio;
        bool initialized = false;
        soundpack::Pack snapshot;
        for (;;) {
            Event event;
            {
                unique_lock<mutex> lock(queueMutex);
                wake.wait(lock, [this] { return stopping.load() || !queue.empty(); });
                if (stopping.load()) break;
                event = queue.front(); queue.pop_front();
                queuedCount.store(queue.size());
                if (keysound::trace::enabled()) keysound::trace::record(keysound::trace::Kind::Dequeue, event.key.vkCode, static_cast<unsigned short>(event.key.flags), event.generation, event.stamp, keysound::trace::now());
            }
            try {
                if (event.warm) { warm(audio, initialized, snapshot, event.generation); continue; }
                if (cancelled(event.generation) || !readyState.load() ||
                    event.generation != readyGeneration.load() ||
                    GetTickCount64() - event.submitted > maxEventAgeMs) {
                    if (keysound::trace::enabled()) keysound::trace::record(keysound::trace::Kind::DropStale, event.key.vkCode, static_cast<unsigned short>(event.key.flags), event.generation, event.stamp, keysound::trace::now());
                    droppedCount.fetch_add(1); continue;
                }
                wstring path;
                const auto mode = soundpack::modeKind(snapshot.mode);
                if (mode == soundpack::Mode::Assign || mode == soundpack::Mode::Mixed) {
                    wstring key = event.mouse ? (event.key.vkCode == VK_LBUTTON ? L"Mouse Left" : event.key.vkCode == VK_RBUTTON ? L"Mouse Right" : L"Mouse Middle") : packKey(event.key);
                    auto it = snapshot.assigned.find(key);
                    if (it != snapshot.assigned.end()) path = it->second;
                }
                if (!event.mouse && mode == soundpack::Mode::Repeat && !snapshot.repeat.empty())
                    path = soundpack::resourcePath(snapshot.directory, snapshot.repeat);
                if (!event.mouse && path.empty() && (mode == soundpack::Mode::Random || mode == soundpack::Mode::Mixed) && !snapshot.sounds.empty())
                    path = snapshot.sounds[(size_t)rand() % snapshot.sounds.size()];
                if (path.empty()) {
                    if (keysound::trace::enabled()) keysound::trace::record(keysound::trace::Kind::DropNoPath, event.key.vkCode, static_cast<unsigned short>(event.key.flags), event.generation, event.stamp, keysound::trace::now());
                    continue;
                }
                lock_guard<mutex> lock(queueMutex);
                if (cancelled(event.generation) || !readyState.load() ||
                    GetTickCount64() - event.submitted > maxEventAgeMs) {
                    if (keysound::trace::enabled()) keysound::trace::record(keysound::trace::Kind::DropStale, event.key.vkCode, static_cast<unsigned short>(event.key.flags), event.generation, event.stamp, keysound::trace::now());
                    droppedCount.fetch_add(1); continue;
                }
                const std::int64_t playStart = keysound::trace::now();
                HRESULT hr = audio.play(path, event.interrupt ? Audio::PlayMode::interrupt : Audio::PlayMode::overlap);
                observe(audio);
                if (keysound::trace::enabled()) keysound::trace::record(SUCCEEDED(hr) ? keysound::trace::Kind::PlayOk : keysound::trace::Kind::PlayFail, event.key.vkCode, static_cast<unsigned short>(event.key.flags), event.generation, playStart, keysound::trace::now());
                if (keysound::trace::enabled()) keysound::trace::record(keysound::trace::Kind::Voices, event.key.vkCode, static_cast<unsigned short>(event.key.flags), event.generation, static_cast<std::int64_t>(audio.voice_count()), 0);
                if (SUCCEEDED(hr)) playCount.fetch_add(1);
                else {
                    errorCount.fetch_add(1);
                    if (!cancelled(event.generation)) lastError.store(hr);
                }
            } catch (const std::bad_alloc&) {
                reset(audio);
                finishWarm(event.generation, E_OUTOFMEMORY, false);
            } catch (...) {
                reset(audio);
                finishWarm(event.generation, E_FAIL, false);
            }
        }
        HRESULT hr = audio.shutdown();
        cachedBytes.store(0); cachedCount.store(0); voiceCount.store(0);
        if (FAILED(hr)) { lastError.store(hr); errorCount.fetch_add(1); }
        loading.store(false); readyState.store(false);
    }
public:
    static constexpr size_t maxQueue = 64;
    static constexpr ULONGLONG maxEventAgeMs = 100;
    atomic<size_t> cachedBytes{0}, cachedCount{0}, voiceCount{0}, queuedCount{0};
    atomic<bool> loading{false}, readyState{false};
    atomic<HRESULT> lastError{S_OK};
    atomic<unsigned long long> playCount{0}, errorCount{0}, droppedCount{0}, enqueueCount{0};

    void start() {
        lock_guard<mutex> lock(queueMutex);
        if (!stopping.load() && !worker.joinable()) worker = thread(&PackPlayer::run, this);
    }
    void submit(const KBDLLHOOKSTRUCT& key, bool interrupt, bool mouse = false) {
        const std::int64_t stamp = keysound::trace::now();
        const bool traced = keysound::trace::enabled();
        if (traced) keysound::trace::record(keysound::trace::Kind::Submit, key.vkCode, static_cast<unsigned short>(key.flags), packGeneration.load(), stamp, mouse ? 1 : 0);
        if (!readyState.load() || loading.load() || stopping.load() || !packActive.load()) {
            if (traced) keysound::trace::record(loading.load() ? keysound::trace::Kind::DropLoading : keysound::trace::Kind::DropNotReady,
                key.vkCode, static_cast<unsigned short>(key.flags), packGeneration.load(), stamp, 0);
            droppedCount.fetch_add(1); return;
        }
        unique_lock<mutex> lock(queueMutex, try_to_lock);
        const unsigned long generation = packGeneration.load();
        if (!lock || stopping.load() || !readyState.load() || loading.load() ||
            generation != readyGeneration.load() || queue.size() >= maxQueue) {
            if (traced) keysound::trace::record(generation != readyGeneration.load() ? keysound::trace::Kind::DropStale : keysound::trace::Kind::DropQueueFull,
                key.vkCode, static_cast<unsigned short>(key.flags), generation, stamp, static_cast<std::int64_t>(queue.size()));
            droppedCount.fetch_add(1); return;
        }
        Event event{};
        event.key = key; event.interrupt = interrupt; event.mouse = mouse; event.generation = generation;
        event.stamp = stamp;
        event.submitted = GetTickCount64();
        try { queue.push_back(event); }
        catch (...) { droppedCount.fetch_add(1); return; }
        queuedCount.store(queue.size()); enqueueCount.fetch_add(1);
        lock.unlock();
        wake.notify_one();
    }
    void prewarm() {
        {
            lock_guard<mutex> lock(queueMutex);
            if (stopping.load()) return;
            Event event{}; event.generation = ++packGeneration; event.warm = true;
            readyState.store(false); loading.store(true); lastError.store(S_OK);
            unsigned int dropped = 0;
            for (const auto& pending : queue) if (!pending.warm) { droppedCount.fetch_add(1); ++dropped; }
            if (dropped && keysound::trace::enabled()) keysound::trace::record(keysound::trace::Kind::PrewarmDropped, 0, 0, static_cast<unsigned int>(event.generation), dropped, 0);
            queue.clear(); queuedCount.store(0);
            try { queue.push_back(event); }
            catch (...) {
                stopping.store(true); loading.store(false);
                lastError.store(E_OUTOFMEMORY); errorCount.fetch_add(1);
            }
        }
        wake.notify_one();
    }
    void shutdown() {
        stopping.store(true);
        {
            lock_guard<mutex> lock(queueMutex);
            readyState.store(false);
            unsigned int dropped = 0;
            for (const auto& pending : queue) if (!pending.warm) { droppedCount.fetch_add(1); ++dropped; }
            if (dropped && keysound::trace::enabled()) keysound::trace::record(keysound::trace::Kind::ShutdownDrop, 0, 0, 0, dropped, 0);
            queue.clear(); queuedCount.store(0);
        }
        wake.notify_one();
        if (worker.joinable()) worker.join();
        loading.store(false);
    }
    ~PackPlayer() { shutdown(); }
};
extern PackPlayer packPlayer;
