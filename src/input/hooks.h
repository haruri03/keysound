#pragma once
#include <atomic>
#include <windows.h>

extern HHOOK keyboardHook;
extern HHOOK mouse_Hook;
extern DWORD keyboardThreadId;
extern HANDLE keyboardReady;
extern std::atomic<DWORD> keyboardHookError;
extern std::atomic<DWORD> mouseHookError;
extern std::atomic<bool> hooksInstalled;

BOOL InstallKbHook();
BOOL UninstallHook();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(INT nCode, WPARAM wParam, LPARAM lParam);
int key_hook();
int key_hook2();
