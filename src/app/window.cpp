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

#include "app/menu_ids.h"
#include "app/resources.h"
#include "app/strings.h"
#include "app/window.h"
#include "audio/pack_player.h"
#include "audio/toggles.h"
#include "core/diagnostics.h"
#include "core/paths.h"
#include "input/hooks.h"
#include "packs/registry.h"
#include "settings.h"
#include <shellapi.h>

LPCTSTR szAppClassName = TEXT("keysound");
LPCTSTR szAppWindowName = TEXT("keysound");
HMENU hmenu;
HMENU packMenu;
static HMENU languageMenu;
static settings::Settings savedSettings;
static constexpr int volumeOptions = settings::max_volume / settings::volume_step + 1;
static wstring settingsPath() { return runPathW() + L"\\keysound-settings.dat"; }

bool trayAvailable = false;
static DWORD trayLastError = 0;
static HWND trayStatusLabel = NULL;
static HWND audioStatusLabel = NULL;
static HWND elevationButton = NULL;
static HWND quitButton = NULL;

static i18n::Lang currentLang() { return i18n::resolve(savedSettings.language); }

static const wchar_t* languageLabel(int choice)
{
    const i18n::Lang lang = currentLang();
    if (choice == i18n::Chinese) return L"\u4e2d\u6587";
    if (choice == i18n::English) return L"English";
    return i18n::pick(lang, L"跟随系统", L"Follow system");
}

static const wchar_t* trayStatusText()
{
    const i18n::Lang lang = currentLang();
    if (restrictedIntegrity)
        return i18n::pick(lang, L"当前进程以低完整性运行，托盘无法注册，也收不到其它程序的按键。常见原因是程序所在目录或 exe 带有低完整性标签；在该目录执行 icacls . /setintegritylevel \"(OI)(CI)M\" 即可恢复，不需要管理员权限。",
                          L"Running at low integrity: the tray cannot register and keys from other programs are blocked. The usual cause is a Low integrity label on the program folder or exe; run icacls . /setintegritylevel \"(OI)(CI)M\" in that folder to restore it. Administrator rights are not required.");
    if (trayAvailable)
        return i18n::pick(lang, L"托盘注册已接受；请在系统托盘检查图标。", L"Tray icon registered; check the notification area.");
    return i18n::pick(lang, L"托盘注册失败；自动重试中，可使用下方菜单。", L"Tray registration failed; retrying automatically. Use the menu below.");
}

static void loadSettings()
{
    settings::load(settingsPath(), savedSettings);
    flag_fast = savedSettings.interrupt;
    flag_mouse = savedSettings.mouse;
    packPlayer.setVolumePercent(savedSettings.volume);
    if (availablePacks.empty()) return;
    size_t chosen = 0;
    if (savedSettings.use_pack)
    {
        bool matched = false;
        for (size_t i = 0; i < availablePacks.size(); ++i)
        {
            if (_wcsicmp(availablePacks[i].directory.c_str(), savedSettings.pack_path.c_str()) != 0) continue;
            chosen = i;
            matched = true;
            break;
        }
        if (!matched) chosen = 0;
    }
    {
        lock_guard<mutex> lock(packMutex);
        activePack = availablePacks[chosen];
        ++packGeneration;
    }
    packActive = true;
    packPlayer.prewarm();
}

static bool isActivePack(const soundpack::Pack& pack)
{
    if (!packActive.load()) return false;
    lock_guard<mutex> lock(packMutex);
    return _wcsicmp(activePack.directory.c_str(), pack.directory.c_str()) == 0;
}

static void buildMenu(HWND hwnd)
{
    if (hmenu)
    {
        DestroyMenu(hmenu);
        hmenu = NULL;
        packMenu = NULL;
        languageMenu = NULL;
    }
    const i18n::Lang lang = currentLang();
    hmenu = CreatePopupMenu();
    packMenu = CreatePopupMenu();
    for (size_t i = 0; i < availablePacks.size(); ++i)
    {
        const UINT state = isActivePack(availablePacks[i]) ? MF_CHECKED : MF_UNCHECKED;
        AppendMenuW(packMenu, MF_STRING | state, IDR_PACK_BASE + (UINT)i, availablePacks[i].name.c_str());
    }
    if (availablePacks.empty()) AppendMenuW(packMenu, MF_STRING | MF_GRAYED, 0, L"(empty)");
    AppendMenuW(hmenu, MF_POPUP, (UINT_PTR)packMenu, i18n::pick(lang, L"音效包", L"Sound packs"));

    languageMenu = CreatePopupMenu();
    for (int i = 0; i < 3; ++i)
    {
        const UINT state = savedSettings.language == i ? MF_CHECKED : MF_UNCHECKED;
        AppendMenuW(languageMenu, MF_STRING | state, IDR_LANG_BASE + i, languageLabel(i));
    }
    AppendMenuW(hmenu, MF_POPUP, (UINT_PTR)languageMenu, i18n::pick(lang, L"语言", L"Language"));

    AppendMenuW(hmenu, MF_STRING | (flag_fast.load() ? MF_CHECKED : MF_UNCHECKED), IDR_FAST,
                i18n::pick(lang, L"音效打断", L"Interrupt sounds"));
    AppendMenuW(hmenu, MF_STRING | (flag_mouse.load() ? MF_CHECKED : MF_UNCHECKED), IDR_MOUSE,
                i18n::pick(lang, L"鼠标音效", L"Mouse sounds"));
    HMENU volumeMenu = CreatePopupMenu();
    for (int i = 0; i < volumeOptions; ++i)
    {
        const int percent = i * settings::volume_step;
        const std::wstring label = std::to_wstring(percent) + L"%";
        AppendMenuW(volumeMenu, MF_STRING, IDR_VOLUME_BASE + i, label.c_str());
    }
    CheckMenuRadioItem(volumeMenu, IDR_VOLUME_BASE, IDR_VOLUME_BASE + volumeOptions - 1,
                       IDR_VOLUME_BASE + savedSettings.volume / settings::volume_step, MF_BYCOMMAND);
    AppendMenuW(hmenu, MF_POPUP, (UINT_PTR)volumeMenu, i18n::pick(lang, L"音量", L"Volume"));
    AppendMenuW(hmenu, MF_STRING, IDR_OPENDIR, i18n::pick(lang, L"打开目录", L"Open folder"));
    AppendMenuW(hmenu, MF_STRING, IDR_QUIT, i18n::pick(lang, L"退出", L"Exit"));
}

static void setTrayLabel()
{
    if (!trayStatusLabel) return;
    const i18n::Lang lang = currentLang();
    std::wstring text = trayStatusText();
    text += L"\n";
    text += i18n::pick(lang,
                       L"关闭与最小化都会收进托盘；退出请右键托盘图标 →「退出」。",
                       L"Closing or minimizing hides to the tray; to quit, right-click the tray icon and choose Exit.");
    SetWindowTextW(trayStatusLabel, text.c_str());
}

static void applyWindowText(HWND hwnd)
{
    const i18n::Lang lang = currentLang();
    setTrayLabel();
    if (elevationButton) SetWindowTextW(elevationButton, i18n::pick(lang, L"以管理员身份重新启动", L"Restart as administrator"));
    if (quitButton) SetWindowTextW(quitButton, i18n::pick(lang, L"退出", L"Exit"));
    HWND openMenuButton = GetDlgItem(hwnd, 100);
    if (openMenuButton) SetWindowTextW(openMenuButton, i18n::pick(lang, L"打开音效菜单", L"Open sound menu"));
}

static void refreshAudioStatus()
{
    if (!audioStatusLabel) return;
    const i18n::Lang lang = currentLang();
    const wchar_t* mode = packPlayer.loading.load()
        ? i18n::pick(lang, L"正在预解码", L"Decoding")
        : packPlayer.readyState.load()
            ? i18n::pick(lang, L"内存音效就绪", L"Memory sounds ready")
            : i18n::pick(lang, L"内存音效未就绪", L"Memory sounds not ready");
    const wchar_t* format = i18n::pick(lang,
        L"%s  PCM: %.2f MiB / %zu \u6587\u4ef6\n\u89e6\u53d1: %llu  \u961f\u5217: %zu  HRESULT: 0x%08lX",
        L"%s  PCM: %.2f MiB / %zu files\nEvents: %llu  Queue: %zu  HRESULT: 0x%08lX");
    wchar_t status[256]{};
    swprintf_s(status, format, mode,
               packPlayer.cachedBytes.load() / 1048576.0,
               (size_t)packPlayer.cachedCount.load(),
               (unsigned long long)packPlayer.playCount.load(),
               (size_t)packPlayer.queuedCount.load(),
               (unsigned long)packPlayer.lastError.load());
    const wchar_t* hookFormat = i18n::pick(lang, L"%s\n钩子: 0x%08lX / 0x%08lX", L"%s\nHooks: 0x%08lX / 0x%08lX");
    wchar_t hooked[384]{};
    swprintf_s(hooked, hookFormat, status,
               (unsigned long)keyboardHookError.load(),
               (unsigned long)mouseHookError.load());
    SetWindowTextW(audioStatusLabel, hooked);
}

static void saveSettings(HWND hwnd)
{
    savedSettings.interrupt = flag_fast.load() != 0;
    savedSettings.mouse = flag_mouse.load() != 0;
    savedSettings.use_pack = packActive.load();
    if (savedSettings.use_pack)
    {
        lock_guard<mutex> lock(packMutex);
        savedSettings.pack_path = activePack.directory;
    }
    if (settings::save(settingsPath(), savedSettings) == settings::SaveResult::Saved) return;
    const i18n::Lang lang = currentLang();
    MessageBoxW(hwnd,
                i18n::pick(lang, L"无法保存设置。请检查程序目录的写入权限及设置文件。",
                                L"Unable to save settings. Check the write permission of the program folder and the settings file."),
                L"keysound", MB_OK | MB_ICONWARNING);
}

static bool restartElevated(HWND owner)
{
    wchar_t path[32768]{};
    if (!GetModuleFileNameW(NULL, path, 32768)) return false;
    SHELLEXECUTEINFOW request{};
    request.cbSize = sizeof(request);
    request.fMask = SEE_MASK_NOCLOSEPROCESS;
    request.lpVerb = L"runas";
    request.lpFile = path;
    request.nShow = SW_SHOWNORMAL;
    if (ShellExecuteExW(&request))
    {
        if (request.hProcess) CloseHandle(request.hProcess);
        return true;
    }
    const DWORD error = GetLastError();
    if (error != ERROR_CANCELLED)
    {
        reportWin32(L"ShellExecuteExW", error);
        const i18n::Lang lang = currentLang();
        MessageBoxW(owner,
                    i18n::pick(lang, L"无法重新启动，当前程序会继续运行。", L"Restart failed; the current program keeps running."),
                    L"keysound", MB_OK | MB_ICONWARNING);
    }
    return false;
}

static const UINT taskbarCreatedMessage = RegisterWindowMessageW(L"TaskbarCreated");

static bool addTrayIcon(HWND hwnd)
{
    NOTIFYICONDATAW data{};
    data.cbSize = sizeof(data);
    data.hWnd = hwnd;
    data.uID = 0;
    data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    data.uCallbackMessage = WM_USER;
    data.hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_ICON1));
    if (!data.hIcon) data.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    lstrcpyW(data.szTip, L"keysound");
    trayAvailable = Shell_NotifyIconW(NIM_ADD, &data) != FALSE;
    if (!trayAvailable) trayLastError = GetLastError();
    if (trayStatusLabel) SetWindowTextW(trayStatusLabel, trayStatusText());
    if (elevationButton) ShowWindow(elevationButton, trayAvailable ? SW_HIDE : SW_SHOW);
    if (trayAvailable) KillTimer(hwnd, 1);
    else SetTimer(hwnd, 1, 5000, NULL);
    return trayAvailable;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    NOTIFYICONDATA nid{};
    POINT pt;
    int xx;
    if (taskbarCreatedMessage && message == taskbarCreatedMessage)
    {
        trayAvailable = false;
        addTrayIcon(hwnd);
        return 0;
    }
    switch (message)
    {
    case WM_CREATE:
        addTrayIcon(hwnd);
        discoverPacks();
        loadSettings();
        buildMenu(hwnd);
        {
            trayStatusLabel = CreateWindowW(L"STATIC", trayStatusText(), WS_CHILD | WS_VISIBLE,
                                            16, 16, 400, 44, hwnd, NULL, GetModuleHandle(NULL), NULL);
            HWND openMenuButton = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                                16, 66, 150, 32, hwnd, (HMENU)100, GetModuleHandle(NULL), NULL);
            elevationButton = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_PUSHBUTTON | (trayAvailable ? 0 : WS_VISIBLE),
                                            178, 66, 230, 32, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);
            audioStatusLabel = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE,
                                             16, 106, 400, 44, hwnd, NULL, GetModuleHandle(NULL), NULL);
            HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            if (trayStatusLabel) SendMessageW(trayStatusLabel, WM_SETFONT, (WPARAM)font, TRUE);
            if (openMenuButton) SendMessageW(openMenuButton, WM_SETFONT, (WPARAM)font, TRUE);
            if (elevationButton) SendMessageW(elevationButton, WM_SETFONT, (WPARAM)font, TRUE);
            quitButton = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       16, 158, 110, 30, hwnd, (HMENU)102, GetModuleHandle(NULL), NULL);
            if (audioStatusLabel) SendMessageW(audioStatusLabel, WM_SETFONT, (WPARAM)font, TRUE);
            if (quitButton) SendMessageW(quitButton, WM_SETFONT, (WPARAM)font, TRUE);
            applyWindowText(hwnd);
            if (taskbarCreatedMessage)
        ChangeWindowMessageFilterEx(hwnd, taskbarCreatedMessage, MSGFLT_ALLOW, nullptr);
    SetTimer(hwnd, 2, 500, NULL);
        }
        break;
    case WM_TIMER:
        if (wParam == 2) refreshAudioStatus();
        else if (wParam == 1 && !trayAvailable) addTrayIcon(hwnd);
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_MINIMIZE && trayAvailable)
        {
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);
    case WM_CLOSE:
        ShowWindow(hwnd, trayAvailable ? SW_HIDE : SW_MINIMIZE);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == 101 && !trayAvailable)
        {
            if (restartElevated(hwnd)) DestroyWindow(hwnd);
            return 0;
        }
        if (LOWORD(wParam) == 100) SendMessage(hwnd, WM_USER, 0, WM_RBUTTONDOWN);
        if (LOWORD(wParam) == 102)
        {
            saveSettings(hwnd);
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    case WM_USER:
        if (lParam == WM_LBUTTONDBLCLK)
        {
            ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
            return 0;
        }
        if (lParam != WM_RBUTTONDOWN) break;
        GetCursorPos(&pt);
        ::SetForegroundWindow(hwnd);
        xx = (int)TrackPopupMenu(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, NULL, hwnd, NULL);
        PostMessageW(hwnd, WM_NULL, 0, 0);
        if (xx == 0) break;
        if (xx >= IDR_LANG_BASE && xx < IDR_LANG_BASE + 3)
        {
            const int choice = xx - IDR_LANG_BASE;
            if (choice == savedSettings.language) break;
            savedSettings.language = choice;
            buildMenu(hwnd);
            applyWindowText(hwnd);
            refreshAudioStatus();
            saveSettings(hwnd);
            return 0;
        }
        {
            bool changed = false;
            if (xx >= IDR_PACK_BASE && xx < IDR_PACK_BASE + (int)availablePacks.size())
            {
                {
                    lock_guard<mutex> lock(packMutex);
                    activePack = availablePacks[xx - IDR_PACK_BASE];
                    ++packGeneration;
                }
                packActive = true;
                changed = true;
                packPlayer.prewarm();
            }
            if (xx == IDR_FAST)
            {
                flag_fast = !flag_fast.load();
                changed = true;
            }
            else if (xx == IDR_MOUSE)
            {
                flag_mouse = !flag_mouse.load();
                changed = true;
            }
            else if (xx >= IDR_VOLUME_BASE && xx < IDR_VOLUME_BASE + volumeOptions)
            {
                const int percent = (xx - IDR_VOLUME_BASE) * settings::volume_step;
                if (percent != savedSettings.volume)
                {
                    savedSettings.volume = percent;
                    packPlayer.setVolumePercent(percent);
                    changed = true;
                }
            }
            else if (xx == IDR_OPENDIR)
            {
                ShellExecuteW(NULL, L"open", runPathW().c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            else if (xx == IDR_QUIT)
            {
                DestroyWindow(hwnd);
                return 0;
            }
            if (changed)
            {
                buildMenu(hwnd);
                saveSettings(hwnd);
            }
        }
        break;
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
        audioStatusLabel = NULL;
        elevationButton = NULL;
        trayStatusLabel = NULL;
        quitButton = NULL;
        trayAvailable = false;
        if (hmenu)
        {
            DestroyMenu(hmenu);
            hmenu = NULL;
            packMenu = NULL;
            languageMenu = NULL;
        }
        nid.cbSize = sizeof(nid);
        nid.hWnd = hwnd;
        nid.uID = 0;
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
    return 0;
}
