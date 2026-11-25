#include <windows.h>
#include <cassert>
#include "app/window.h"

static LRESULT CALLBACK fixture(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_CLOSE) return WndProc(h,m,w,l);
    return DefWindowProcW(h,m,w,l);
}
int main() {
    WNDCLASSW c{}; c.lpfnWndProc=fixture; c.hInstance=GetModuleHandleW(nullptr); c.lpszClassName=L"KeySoundLifecycleTest";
    assert(RegisterClassW(&c));
    HWND h=CreateWindowW(c.lpszClassName,L"isolated",WS_OVERLAPPEDWINDOW,0,0,200,100,nullptr,nullptr,c.hInstance,nullptr);
    assert(h); ShowWindow(h,SW_SHOWNOACTIVATE);
    trayAvailable=true;
    SendMessageW(h,WM_CLOSE,0,0);
    assert(IsWindow(h) && !IsWindowVisible(h));
    ShowWindow(h,SW_SHOWNOACTIVATE);
    trayAvailable=false;
    SendMessageW(h,WM_CLOSE,0,0);
    assert(IsWindow(h) && IsIconic(h));
    DestroyWindow(h); UnregisterClassW(c.lpszClassName,c.hInstance);
}
