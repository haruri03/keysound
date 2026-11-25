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

#include "core/paths.h"

string getRunPath()
{
	char buf[MAX_PATH];
	GetModuleFileNameA(NULL, buf, MAX_PATH);
	string::size_type pos = string(buf).find_last_of("\\/");
	string dir = string(buf).substr(0, pos);
	return dir;
}

wstring runPathW() {
    vector<wchar_t> path(512);
    for (;;) {
        DWORD n = GetModuleFileNameW(NULL, path.data(), (DWORD)path.size());
        if (!n) return L"";
        if (n < path.size() - 1) { wstring s(path.data(), n); size_t pos = s.find_last_of(L"\\/"); return pos == wstring::npos ? L"" : s.substr(0, pos); }
        if (path.size() >= 32768) return L"";
        path.resize(path.size() * 2);
    }
}
