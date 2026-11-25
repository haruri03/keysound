#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
int main() {
 HWND window = FindWindowW(L"keysound", L"keysound");
 if (!window) { puts("window not found"); return 1; }
 NOTIFYICONIDENTIFIER id{}; id.cbSize=sizeof(id); id.hWnd=window; id.uID=0;
 RECT rc{}; HRESULT result = Shell_NotifyIconGetRect(&id,&rc);
 printf("window=%p, notification HRESULT=0x%08lx, rect=%ld,%ld,%ld,%ld\n",window,result,rc.left,rc.top,rc.right,rc.bottom);
 return FAILED(result) ? 2 : 0;
}
