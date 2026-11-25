#include <windows.h>
#include <shlwapi.h>
#include <sddl.h>
#include <stdio.h>
#include <string>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "advapi32.lib")

static DWORD integrity(std::wstring& sidText)
{
    HANDLE token = nullptr;
    sidText.clear();
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return 0;
    DWORD size = 0;
    DWORD level = 0;
    GetTokenInformation(token, TokenIntegrityLevel, nullptr, 0, &size);
    if (size == 0) { CloseHandle(token); return 0; }
    BYTE* data = new BYTE[size];
    if (GetTokenInformation(token, TokenIntegrityLevel, data, size, &size))
    {
        PSID sid = reinterpret_cast<TOKEN_MANDATORY_LABEL*>(data)->Label.Sid;
        if (sid && IsValidSid(sid))
        {
            LPWSTR text = nullptr;
            if (ConvertSidToStringSidW(sid, &text))
            {
                sidText = text;
                LocalFree(text);
            }
            const DWORD count = *GetSidSubAuthorityCount(sid);
            level = *GetSidSubAuthority(sid, count - 1);
        }
    }
    delete[] data;
    CloseHandle(token);
    return level;
}

int main()
{
    wchar_t exe[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    PathRemoveFileSpecW(exe);
    std::wstring target(exe);
    target += L"/integrity-result.txt";
    std::wstring sidText;
    const DWORD level = integrity(sidText);
    FILE* file = nullptr;
    if (_wfopen_s(&file, target.c_str(), L"w") != 0 || !file) return 1;
    fprintf(file, "integrity=%lx sid=%ls\n", (unsigned long)level, sidText.c_str());
    fclose(file);
    return 0;
}
