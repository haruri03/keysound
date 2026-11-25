#include <windows.h>
#include <iostream>
#include <string>
#include <cassert>
static std::wstring status(HWND app) {
    wchar_t text[512]{};
    for (HWND child = GetWindow(app,GW_CHILD); child; child=GetWindow(child,GW_HWNDNEXT)) {
        GetWindowTextW(child,text,512);
        if (wcsstr(text,L"HRESULT:")) return text;
    }
    return L"";
}
int main() {
    HWND app=FindWindowW(L"keysound",L"keysound"); assert(app);
    HWND scratch=CreateWindowExW(0,L"STATIC",L"KeySound isolated input test",WS_OVERLAPPEDWINDOW|WS_VISIBLE,100,100,400,200,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    assert(scratch); SetForegroundWindow(scratch); Sleep(200);
    if(GetForegroundWindow()!=scratch) { DestroyWindow(scratch); std::cerr<<"Focus not obtained; input not sent\n"; return 2; }
    const auto before=status(app);
    INPUT inputs[21]{};
    for(int i=0;i<21;++i) { inputs[i].type=INPUT_KEYBOARD; inputs[i].ki.wVk='A'; inputs[i].ki.wScan=0x1e; }
    inputs[20].ki.dwFlags=KEYEVENTF_KEYUP;
    assert(SendInput(21,inputs,sizeof(INPUT))==21);
    const DWORD end=GetTickCount()+1000;
    while(GetTickCount()<end) { MSG msg; while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);} Sleep(10); }
    const auto after=status(app);
    const bool background=GetForegroundWindow()!=app;
    DestroyWindow(scratch);
    std::wcout<<L"Before: "<<before<<L"\nAfter: "<<after<<L"\nBackground="<<background<<L"\n";
    assert(background && before!=after);
}
