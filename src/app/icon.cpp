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

#include "app/icon.h"
#include "app/resources.h"
#include "app/window.h"
#include "core/diagnostics.h"
#include "input/hooks.h"

int SetIco()
{
	cout << "SetIco start" << endl;
	HWND hwnd;
	MSG msg{};
	WNDCLASSW wndclass{};

	HWND handle = FindWindow(NULL, szAppWindowName);

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	cout << "WndProc start" << endl;
	wndclass.lpfnWndProc = WndProc;
	cout << "WndProc end" << endl;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandleW(NULL);
	wndclass.hIcon = LoadIcon(NULL, (LPCTSTR)(32512));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppClassName;

	if (!RegisterClassW(&wndclass))
	{
		DWORD error = GetLastError();
		reportWin32(L"RegisterClassW(tray window)", error);
		MessageBoxW(NULL, L"无法注册托盘窗口类，请检查诊断错误。", szAppClassName, MB_ICONERROR);
		return 1;
	}

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppClassName, szAppWindowName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		440,
		340,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	if (!hwnd) {
		reportWin32(L"CreateWindowExW(tray window)", GetLastError());
		UnregisterClassW(szAppClassName, wndclass.hInstance);
		return 1;
	}
	if (!hooksInstalled.load()) {
		wchar_t text[256] = {};
		swprintf_s(text, L"全局输入钩子安装失败：键盘错误=%lu，鼠标错误=%lu。后台输入不可保证；请检查权限或安全软件。",
			keyboardHookError.load(), mouseHookError.load());
		MessageBoxW(hwnd, text, L"keysound 输入诊断", MB_OK | MB_ICONWARNING);
	}
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	cout << "消息 start" << endl;

	BOOL result;
	while ((result = GetMessageW(&msg, NULL, 0, 0)) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	if (result == -1) reportWin32(L"GetMessageW(tray)", GetLastError());
	if (IsWindow(hwnd)) DestroyWindow(hwnd);
	UnregisterClassW(szAppClassName, wndclass.hInstance);
	return result == -1 ? 1 : 0;
}



