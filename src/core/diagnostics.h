#pragma once
#include <windows.h>
DWORD integrityLevel();
extern const DWORD processIntegrity;
extern const bool restrictedIntegrity;
void reportWin32(const wchar_t* operation, DWORD error);
