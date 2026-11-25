#include <windows.h>
#include <atomic>
#include <string>
#include "audio/pack_player.h"
#include "audio/trace.h"
#include "audio/toggles.h"
#include "core/diagnostics.h"
#include "input/hooks.h"
#include "packs/registry.h"

HHOOK keyboardHook = 0;
HHOOK mouse_Hook = 0;
DWORD keyboardThreadId = 0;
HANDLE keyboardReady = NULL;
std::atomic<DWORD> keyboardHookError(0);
std::atomic<DWORD> mouseHookError(0);
std::atomic<bool> hooksInstalled(false);

BOOL InstallKbHook()
{
    if (mouse_Hook) UninstallHook();
    mouse_Hook = SetWindowsHookEx(
        WH_MOUSE_LL,
        (HOOKPROC)(LowLevelMouseProc),
        GetModuleHandle(NULL),
        NULL);
    const DWORD error = mouse_Hook ? ERROR_SUCCESS : GetLastError();
    mouseHookError.store(error);
    if (!mouse_Hook) reportWin32(L"SetWindowsHookExW(WH_MOUSE_LL)", error);
    return mouse_Hook != NULL;
}

BOOL UninstallHook()
{
    BOOL ok = FALSE;
    if (mouse_Hook)
    {
        ok = UnhookWindowsHookEx(mouse_Hook);
        mouse_Hook = NULL;
    }
    return ok;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && lParam)
    {
        const KBDLLHOOKSTRUCT& data = *reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        static bool down[512]{};
        const unsigned slot = (data.scanCode & 255u) | ((data.flags & LLKHF_EXTENDED) ? 256u : 0u);
        const bool pressed = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
        const bool released = wParam == WM_KEYUP || wParam == WM_SYSKEYUP;
        if (released) down[slot] = false;
        if (pressed && !down[slot])
        {
            down[slot] = true;
            packPlayer.submit(data, flag_fast.load());
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && lParam && flag_mouse.load() &&
        (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP))
    {
        KBDLLHOOKSTRUCT button{};
        button.vkCode = wParam == WM_LBUTTONUP ? VK_LBUTTON : wParam == WM_RBUTTONUP ? VK_RBUTTON : VK_MBUTTON;
        packPlayer.submit(button, flag_fast.load(), true);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int key_hook2()
{
    mouse_Hook = SetWindowsHookEx(
        WH_MOUSE_LL,
        (HOOKPROC)(LowLevelMouseProc),
        GetModuleHandle(NULL),
        NULL);

    MSG msg;
    while (1)
    {
        if (PeekMessageA(&msg, NULL, NULL, NULL, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else
            Sleep(100);
    }

    if (mouse_Hook)
    {
        UnhookWindowsHookEx(mouse_Hook);
        mouse_Hook = NULL;
    }
    return 0;
}

int key_hook()
{
    keyboardThreadId = GetCurrentThreadId();
    MSG initialMessage;
    PeekMessage(&initialMessage, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    SetLastError(ERROR_SUCCESS);
    keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandleW(NULL), 0);
    const DWORD error = keyboardHook ? ERROR_SUCCESS : GetLastError();
    keyboardHookError.store(error);
    if (!keyboardHook) reportWin32(L"SetWindowsHookExW(WH_KEYBOARD_LL)", error);
    const BOOL mouseInstalled = InstallKbHook();
    hooksInstalled.store(keyboardHook != NULL && mouseInstalled != FALSE);
    SetEvent(keyboardReady);
    if (!keyboardHook)
    {
        UninstallHook();
        return -1;
    }

    SetTimer(NULL, 1, 100, NULL);
    MSG msg{};
    BOOL result;
    while ((result = GetMessageW(&msg, NULL, 0, 0)) > 0)
    {
        if (msg.message == WM_TIMER && msg.hwnd == NULL)
        {
            if (keysound::trace::enabled())
                keysound::trace::record(keysound::trace::Kind::Heartbeat, 0, 0, 0, keysound::trace::now(), 0);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    KillTimer(NULL, 1);
    if (result == -1) reportWin32(L"GetMessageW(hooks)", GetLastError());
    UnhookWindowsHookEx(keyboardHook);
    keyboardHook = NULL;
    UninstallHook();
    hooksInstalled.store(false);
    return 0;
}
