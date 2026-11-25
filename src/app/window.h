#pragma once
#include <windows.h>
extern bool trayAvailable;
extern LPCTSTR szAppClassName;
extern LPCTSTR szAppWindowName;
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
