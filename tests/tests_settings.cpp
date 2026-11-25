#include "settings.h"
#include <windows.h>
#include <cassert>
#include <string>
#include <iostream>

int main() {
    const char v2[] = "keysound-settings:2\nmode=1\npack=0\npath=\ninterrupt=0\nmouse=1\nlang=1\n";
    settings::Settings fromV2;
    assert(settings::detail::parse(v2, fromV2));
    assert(fromV2.language == 1 && fromV2.volume == 100);
    const std::string v3 = "keysound-settings:3\nmode=1\npack=0\npath=\ninterrupt=0\nmouse=1\nlang=1\nvolume=";
    assert(!settings::detail::parse(v3 + "110\n", fromV2));
    assert(!settings::detail::parse(v3 + "50x\n", fromV2));

    wchar_t module[MAX_PATH] = {}; assert(GetModuleFileNameW(NULL, module, MAX_PATH));
    std::wstring unique = module;
    unique = unique.substr(0, unique.find_last_of(L"\\/")) + L"\\settings-test-" + std::to_wstring(GetCurrentProcessId());
    assert(CreateDirectoryW(unique.c_str(), NULL));
    std::wstring file = std::wstring(unique) + L"\\\u8bbe\u7f6e.ini";
    settings::Settings s;
    assert(settings::load(file, s) == settings::LoadResult::Missing);
    assert(!s.interrupt && s.mouse && !s.use_pack && s.volume == 100);
    s.interrupt = true; s.mouse = false;
    s.volume = 30;
    s.use_pack = true; s.pack_path = L"C:\\sounds\\\u6210\u6b65\u5802 \U0001f3b5";
    assert(settings::save(file, s) == settings::SaveResult::Saved);
    settings::Settings loaded;
    assert(settings::load(file, loaded) == settings::LoadResult::Loaded);
    assert(loaded.interrupt && !loaded.mouse && loaded.use_pack && loaded.pack_path == s.pack_path);
    assert(loaded.volume == 30);
    loaded.use_pack = false;
    settings::SaveResult second = settings::save(file, loaded);
    if (second != settings::SaveResult::Saved) std::cerr << "second save error " << GetLastError() << "\n";
    assert(second == settings::SaveResult::Saved);
    assert(settings::load(file, s) == settings::LoadResult::Loaded && !s.use_pack);
    HANDLE h = CreateFileW(file.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(h != INVALID_HANDLE_VALUE); DWORD written; const char bad[] = "not our config";
    assert(WriteFile(h, bad, sizeof(bad)-1, &written, NULL)); CloseHandle(h);
    assert(settings::load(file, s) == settings::LoadResult::Invalid);
    assert(!s.interrupt && s.mouse && !s.use_pack);
    assert(settings::save(file, loaded) == settings::SaveResult::Protected);
    h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    char check[32] = {}; DWORD got = 0; assert(h != INVALID_HANDLE_VALUE && ReadFile(h, check, sizeof(check), &got, NULL)); CloseHandle(h);
    assert(std::string(check, got) == bad);
    HANDLE legacy = CreateFileW(file.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(legacy != INVALID_HANDLE_VALUE);
    const char v1[] = "keysound-settings:1\nmode=9\npack=0\npath=\ninterrupt=1\nmouse=0\n";
    DWORD legacyWritten = 0;
    assert(WriteFile(legacy, v1, sizeof(v1) - 1, &legacyWritten, NULL)); CloseHandle(legacy);
    settings::Settings fromV1;
    assert(settings::load(file, fromV1) == settings::LoadResult::Loaded);
    assert(fromV1.interrupt && !fromV1.mouse && !fromV1.use_pack && fromV1.language == 0);
    assert(fromV1.volume == 100);
    fromV1.language = 2;
    fromV1.volume = 30;
    assert(settings::save(file, fromV1) == settings::SaveResult::Saved);
    settings::Settings roundTrip;
    assert(settings::load(file, roundTrip) == settings::LoadResult::Loaded);
    assert(roundTrip.language == 2 && roundTrip.interrupt && !roundTrip.mouse && roundTrip.volume == 30);
    h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    char raw[256] = {}; DWORD rawBytes = 0;
    assert(h != INVALID_HANDLE_VALUE && ReadFile(h, raw, sizeof(raw), &rawBytes, NULL)); CloseHandle(h);
    const std::string text(raw, rawBytes);
    assert(text.compare(0, 19, "keysound-settings:3") == 0);
    assert(text.find("\nmode=1\n") != std::string::npos);
    assert(text.find("\nlang=2\n") != std::string::npos);
    assert(text.find("\nvolume=30\n") != std::string::npos);
    fromV1.language = 7;
    assert(settings::save(file, fromV1) == settings::SaveResult::Invalid);
    roundTrip.volume = 110;
    assert(settings::save(file, roundTrip) == settings::SaveResult::Invalid);
    roundTrip.volume = 0;
    assert(settings::save(file, roundTrip) == settings::SaveResult::Saved);
    assert(settings::load(file, s) == settings::LoadResult::Loaded && s.volume == 0);
    assert(DeleteFileW(file.c_str())); assert(RemoveDirectoryW(unique.c_str()));
    std::cout << "settings tests passed\n";
}
