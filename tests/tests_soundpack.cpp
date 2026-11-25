#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <cassert>
#include "soundpack.h"
#include "core/paths.h"
#include "packs/registry.h"

using namespace std;

static void testResourcePaths() {
    wchar_t module[MAX_PATH] = {};
    assert(GetModuleFileNameW(nullptr, module, MAX_PATH));
    wstring root = module;
    root = root.substr(0, root.find_last_of(L"\\/")) + L"\\resource-path-test-" + to_wstring(GetCurrentProcessId());
    assert(CreateDirectoryW(root.c_str(), nullptr));
    const wstring nested = root + L"\\sounds";
    const wstring directFile = root + L"\\音效.wav";
    const wstring nestedFile = nested + L"\\音效.wav";
    assert(CreateDirectoryW(nested.c_str(), nullptr));
    assert(soundpack::resourcePath(root, L"音效.wav") == directFile);
    HANDLE direct = CreateFileW(directFile.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    assert(direct != INVALID_HANDLE_VALUE);
    assert(CloseHandle(direct));
    assert(soundpack::resourcePath(root, L"音效.wav") == directFile);
    HANDLE child = CreateFileW(nestedFile.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    assert(child != INVALID_HANDLE_VALUE);
    assert(CloseHandle(child));
    assert(soundpack::resourcePath(root, L"音效.wav") == nestedFile);
    assert(DeleteFileW(nestedFile.c_str()));
    assert(soundpack::resourcePath(root, L"音效.wav") == directFile);
    assert(DeleteFileW(directFile.c_str()));
    assert(RemoveDirectoryW(nested.c_str()));
    assert(RemoveDirectoryW(root.c_str()));
}

int main() {
    testResourcePaths();
    soundpack::Json value;
    const std::string escaped = "{\"s\":\"\\u6210\\u6b65\\u5802 \\ud83c\\udfb5\"}";
    assert(soundpack::Parser(escaped).parse(value));
    std::wstring decoded;
    assert(soundpack::wide(value.object["s"].text, decoded));
    assert(decoded == L"\u6210\u6b65\u5802 \U0001f3b5");
    const std::string broken = "{\"s\":\"\\ud800\"}";
    assert(!soundpack::Parser(broken).parse(value));
    assert(soundpack::modeKind(L"\u6307\u5b9a") == soundpack::Mode::Assign);
    assert(soundpack::modeKind(L"assign") == soundpack::Mode::Assign);
    assert(soundpack::modeKind(L"Assign") == soundpack::Mode::Assign);
    assert(soundpack::modeKind(L"\u91cd\u590d") == soundpack::Mode::Repeat);
    assert(soundpack::modeKind(L"repeat") == soundpack::Mode::Repeat);
    assert(soundpack::modeKind(L"REPEAT") == soundpack::Mode::Repeat);
    assert(soundpack::modeKind(L"\u968f\u673a") == soundpack::Mode::Random);
    assert(soundpack::modeKind(L"random") == soundpack::Mode::Random);
    assert(soundpack::modeKind(L"Random") == soundpack::Mode::Random);
    assert(soundpack::modeKind(L"mixed") == soundpack::Mode::Mixed);
    assert(soundpack::modeKind(L"混合") == soundpack::Mode::Mixed);
    assert(soundpack::modeKind(L"nonsense") == soundpack::Mode::Unknown);
    assert(soundpack::modeKind(L"") == soundpack::Mode::Unknown);
    wstring root = runPathW();
    for (int depth = 0; depth < 5 && !root.empty(); ++depth) {
        if (GetFileAttributesW((root + L"\\sounds").c_str()) != INVALID_FILE_ATTRIBUTES) break;
        const size_t slash = root.find_last_of(L"\\/");
        root = slash == wstring::npos ? wstring() : root.substr(0, slash);
    }
    assert(!root.empty());
    discoverPacks(root);
    assert(!availablePacks.empty());
    const soundpack::Pack* pack = nullptr;
    for (const auto& p : availablePacks) if (p.name == L"\u6210\u6b65\u5802\u97f3\u6548") pack = &p;
    assert(pack && pack->assigned.size() >= 80);
    for (const wchar_t* name : {L"mahjong", L"piano"}) {
        soundpack::Pack mixed;
        assert(soundpack::load(root + L"\\sounds\\" + name, mixed));
        assert(soundpack::modeKind(mixed.mode) == soundpack::Mode::Mixed);
        assert(!mixed.assigned.empty() && !mixed.sounds.empty());
        for (const auto& entry : mixed.assigned)
            assert(GetFileAttributesW(entry.second.c_str()) != INVALID_FILE_ATTRIBUTES);
        for (const auto& path : mixed.sounds)
            assert(GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES);
    }
    KBDLLHOOKSTRUCT key{};
    key.vkCode = VK_LSHIFT; assert(packKey(key) == L"L Shift");
    key.vkCode = VK_RCONTROL; assert(packKey(key) == L"R Ctrl");
    key.vkCode = VK_LEFT; assert(packKey(key) == L"\u2190");
    key.vkCode = VK_OEM_4; assert(packKey(key) == L"[ {");
    key.vkCode = '1';
    auto found = pack->assigned.find(packKey(key));
    assert(found != pack->assigned.end());
    assert(GetFileAttributesW(found->second.c_str()) != INVALID_FILE_ATTRIBUTES);
    std::wstring cmd = L"open \"" + found->second + L"\" alias soundpack_test";
    MCIERROR error = mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
    assert(error == 0);
    assert(mciSendStringW(L"play soundpack_test wait", nullptr, 0, nullptr) == 0);
    assert(mciSendStringW(L"close soundpack_test", nullptr, 0, nullptr) == 0);
    return 0;
}
