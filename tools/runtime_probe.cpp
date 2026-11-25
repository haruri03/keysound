#include <windows.h>
#include <stdio.h>
static void token(DWORD pid) {
 HANDLE p=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid),t=nullptr;
 if(!p || !OpenProcessToken(p,TOKEN_QUERY,&t)){printf("pid=%lu token error=%lu\n",pid,GetLastError());if(p)CloseHandle(p);return;}
 DWORD n=0,session=0; BYTE data[512]{}; TOKEN_ELEVATION el{}; DWORD ui=0;
 GetTokenInformation(t,TokenIntegrityLevel,data,sizeof(data),&n);
 auto sid=((TOKEN_MANDATORY_LABEL*)data)->Label.Sid;
 DWORD level=IsValidSid(sid)?*GetSidSubAuthority(sid,*GetSidSubAuthorityCount(sid)-1):0;
 GetTokenInformation(t,TokenElevation,&el,sizeof(el),&n);GetTokenInformation(t,TokenUIAccess,&ui,sizeof(ui),&n);
 ProcessIdToSessionId(pid,&session);
 printf("pid=%lu integrity=%lx elevated=%lu uiaccess=%lu session=%lu\n",pid,level,el.TokenIsElevated,ui,session);
 CloseHandle(t);CloseHandle(p);
}
int main(){token(GetCurrentProcessId());HWND shell=FindWindowW(L"Shell_TrayWnd",nullptr);DWORD pid=0;GetWindowThreadProcessId(shell,&pid);printf("shell=%p ",shell);token(pid);
 wchar_t name[256]{};DWORD n=0;GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,name,sizeof(name),&n);wprintf(L"desktop=%s\n",name);
 DWORD_PTR result=0;SetLastError(0);auto ok=SendMessageTimeoutW(shell,WM_NULL,0,0,SMTO_ABORTIFHUNG,1000,&result);printf("shell WM_NULL=%lld error=%lu\n",(long long)ok,GetLastError());
}
