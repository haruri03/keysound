#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
int main() {
 HWND w=CreateWindowExW(0,L"STATIC",L"Keysound tray test",0,0,0,0,0,0,0,GetModuleHandleW(0),0);
 NOTIFYICONDATAW n{}; n.cbSize=sizeof(n); n.hWnd=w; n.uID=1; n.uFlags=NIF_ICON|NIF_TIP; n.hIcon=LoadIconW(NULL,IDI_APPLICATION); lstrcpyW(n.szTip,L"Keysound tray test");
 BOOL ok=Shell_NotifyIconW(NIM_ADD,&n); printf("window=%p icon=%p add=%d error=%lu\n", w, n.hIcon,ok,GetLastError());
 NOTIFYICONIDENTIFIER id{}; id.cbSize=sizeof(id); id.hWnd=w; id.uID=1; RECT r{}; printf("rect=%08lx\n",Shell_NotifyIconGetRect(&id,&r));
 Shell_NotifyIconW(NIM_DELETE,&n); DestroyWindow(w); return ok?0:1;
}
