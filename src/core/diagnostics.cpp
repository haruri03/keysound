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

#include "core/diagnostics.h"
#pragma comment(lib, "advapi32.lib")

DWORD integrityLevel() {
    HANDLE token = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return MAXDWORD;
    alignas(TOKEN_MANDATORY_LABEL) BYTE storage[512]{};
    DWORD size = 0, level = MAXDWORD;
    if (GetTokenInformation(token, TokenIntegrityLevel, storage, sizeof(storage), &size)) {
        PSID sid = reinterpret_cast<TOKEN_MANDATORY_LABEL*>(storage)->Label.Sid;
        if (IsValidSid(sid) && *GetSidSubAuthorityCount(sid))
            level = *GetSidSubAuthority(sid, *GetSidSubAuthorityCount(sid) - 1);
    }
    CloseHandle(token);
    return level;
}
const DWORD processIntegrity = integrityLevel();
const bool restrictedIntegrity = processIntegrity < SECURITY_MANDATORY_MEDIUM_RID;
void reportWin32(const wchar_t* operation, DWORD error) {
    wchar_t detail[512] = {};
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, 0, detail, 512, NULL);
    wchar_t line[768] = {};
    swprintf_s(line, L"keysound: %s: error=%lu %s\n", operation, error, detail);
    OutputDebugStringW(line);
    fwprintf(stderr, L"%s", line);
}
