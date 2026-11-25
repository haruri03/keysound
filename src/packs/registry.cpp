#include <io.h>
#include <direct.h>
#include <string>
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <time.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <algorithm>
#include <comdef.h>
using namespace std;

#include "packs/registry.h"

mutex packMutex;
soundpack::Pack activePack;
vector<soundpack::Pack> availablePacks;
const int IDR_PACK_BASE = 1000;
const int IDR_PACK_LIMIT = 30000;
atomic<bool> packActive(false);

void discoverPacks(const wstring& base) {
    availablePacks.clear();
    wstring root = base + L"\\sounds";
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((root + L"\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || wcscmp(fd.cFileName,L".")==0 || wcscmp(fd.cFileName,L"..")==0) continue;
        soundpack::Pack p;
        if (soundpack::load(root + L"\\" + fd.cFileName, p) && availablePacks.size() < IDR_PACK_LIMIT-IDR_PACK_BASE) availablePacks.push_back(p);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}
wstring packKey(const KBDLLHOOKSTRUCT &k) {
    DWORD v=k.vkCode;
    if (v >= 'A' && v <= 'Z') return wstring(1, (wchar_t)v);
    if (v >= '0' && v <= '9') return wstring(1, (wchar_t)v);
    if (v >= VK_NUMPAD0 && v <= VK_NUMPAD9) return wstring(1, (wchar_t)(L'0'+v-VK_NUMPAD0));
    if (v >= VK_F1 && v <= VK_F24) return L"F" + to_wstring(v-VK_F1+1);
    switch(v) {
    case VK_LSHIFT: return L"L Shift"; case VK_RSHIFT: return L"R Shift";
    case VK_SHIFT: return k.scanCode==0x36 ? L"R Shift" : L"L Shift";
    case VK_LCONTROL: return L"L Ctrl"; case VK_RCONTROL: return L"R Ctrl";
    case VK_CONTROL: return (k.flags & LLKHF_EXTENDED) ? L"R Ctrl" : L"L Ctrl";
    case VK_LMENU: return L"L Alt"; case VK_RMENU: return L"R Alt";
    case VK_MENU: return (k.flags & LLKHF_EXTENDED) ? L"R Alt" : L"L Alt";
    case VK_LWIN: case VK_RWIN: return L"Win";
    case VK_ESCAPE:return L"Esc"; case VK_SCROLL:return L"SrcLk"; case VK_PAUSE:return L"Pause";
    case VK_DELETE:return L"Del"; case VK_BACK:return L"Backspace"; case VK_INSERT:return L"Ins";
    case VK_HOME:return L"Home"; case VK_PRIOR:return L"PgUp"; case VK_NEXT:return L"PgDn"; case VK_END:return L"End";
    case VK_TAB:return L"Tab"; case VK_CAPITAL:return L"Caps"; case VK_SPACE:return L"Space"; case VK_RETURN:return L"Enter";
    case VK_LEFT:return L"\u2190"; case VK_UP:return L"\u2191"; case VK_DOWN:return L"\u2193"; case VK_RIGHT:return L"\u2192";
    case VK_OEM_3:return L"~"; case VK_OEM_MINUS:return L"-"; case VK_OEM_PLUS:return L"=";
    case VK_OEM_5:return L"\\"; case VK_OEM_6:return L"] }"; case VK_OEM_4:return L"[ {";
    case VK_OEM_7:return L"'"; case VK_OEM_1:return L"; :"; case VK_OEM_PERIOD:return L". >";
    case VK_OEM_COMMA:return L", <"; case VK_OEM_2:return L"/ ?";
    default:return L"";
    }
}
#ifndef KEYSOUND_MEMORY_AUDIO_TYPE
#include "memory_audio.h"
#define KEYSOUND_MEMORY_AUDIO_TYPE keysound::MemoryAudio
#endif

atomic<unsigned long> packGeneration(0);
