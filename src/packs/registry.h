#pragma once
#include <windows.h>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include "soundpack.h"
#include "core/paths.h"

extern std::mutex packMutex;
extern soundpack::Pack activePack;
extern std::vector<soundpack::Pack> availablePacks;
extern std::atomic<unsigned long> packGeneration;
extern std::atomic<bool> packActive;
extern const int IDR_PACK_BASE;
extern const int IDR_PACK_LIMIT;

void discoverPacks(const std::wstring& base = runPathW());
std::wstring packKey(const KBDLLHOOKSTRUCT& k);
