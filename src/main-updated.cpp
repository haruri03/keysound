// keysound.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <io.h>  //读取文件
#include <direct.h>  //列举指定路径下所有的文件名
#include <string>
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <vector>
#include <comdef.h>  //主要是定义com组件。是开发环境中比较底层的东西。比如数据类型的转换函数，以及位数变化等。可以直接调用里边的函数。
#include <thread>   //线程
#include <time.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <algorithm>
#include "soundpack.h"
#include "settings.h"
#pragma comment(lib, "winmm.lib")   //winmm.lib,是Windows音频多媒体AP的静态库
using namespace std;
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // 设置入口地址
//屏蔽控制台应用程序的窗口



// 静态资源
#define IDI_ICON1 102


// 取随机数
int randEx(int a, int b)
{
	if (a == b)
		return a;
	LARGE_INTEGER seed;
	QueryPerformanceFrequency(&seed);
	QueryPerformanceCounter(&seed);
	srand(seed.QuadPart);
	return rand() % b + a;
}

atomic<int> flag_fast(0);//音效打断

// 播放音效函数
void playtheSound(string path)
{
	cout << path << endl;
	string lpath = "open " + path + " alias MySong";
	if (flag_fast) {
		mciSendString(TEXT("close all"), NULL, 0, NULL);//实现了打断
	}
	// 把lpath里的 \ 改成 /
	for (int i = 0; i < lpath.length(); i++)
	{
		if (lpath[i] == '\\')
			lpath[i] = '/';
	}
	cout << "--------------666---" << lpath;
	_bstr_t bstr(lpath.c_str());
	LPTSTR lr = (LPTSTR)bstr;
	/*
	char vo[20];
	char res[260];
	string path2 = "status " + path + " volume";
	_bstr_t bstr2(path2.c_str());
	LPTSTR lr2 = (LPTSTR)bstr2;
	
	char file1[100] = "setaudio 01.mp3 volume to "; // to后面一定要有空格，不然会报错
	mciSendString(lr2, (LPTSTR)res, 260, NULL);
	int volume = 0;
	volume = atoi(res); // 转化语句
	volume += 200;
	itoa(volume, vo, 10); //转化语句
	//string path3 = "setaudio MySong volume to " + vo;
	string path3 = "setaudio MySong volume to 10";
	_bstr_t bstr3(path3.c_str());
	LPTSTR lr3 = (LPTSTR)bstr3;
	mciSendString(lr3, NULL, 0, NULL); // 设定成功
	*/
	// (TEXT("open X:/C++/keysound/x64/Debug/sounds//2.wav alias mysong"), NULL, 0, NULL);
	mciSendString(lr, NULL, 0, NULL);
	mciSendString(TEXT("play MySong"), NULL, 0, NULL);// notify
	return;
}

//static LPTSTR lr;
// 播放音效函数
static int zhe = 0;
void playtheSound2(string path)//改成一个类，有成员  或者一个静态链表判断有没有添加过 这些考虑还是不对，可能要考虑不是用这个库？？？
{
	//Sleep(500);//半秒
	//mciSendString(TEXT("close MySong"), NULL, 0, NULL);
	cout << path << endl;
	string lpath = "open " + path + " alias MySong" + to_string(zhe);
	//string lpath = "open " + path;

	// 把lpath里的 \ 改成 /
	for (int i = 0; i < lpath.length(); i++)
	{
		if (lpath[i] == '\\')
			lpath[i] = '/';
	}
	cout << "--------------666---" << lpath << endl;
	_bstr_t bstr(lpath.c_str());
	LPTSTR lr = (LPTSTR)bstr;
	//lr = (LPTSTR)bstr;
	// (TEXT("open X:/C++/keysound/x64/Debug/sounds//2.wav alias mysong"), NULL, 0, NULL);
	mciSendString(lr, NULL, 0, NULL);
	//mciSendString(TEXT("play MySong"), NULL, 0, NULL);

	string path_zhe = "play MySong0";// +to_string(zhe);
	_bstr_t bstr_zhe(path_zhe.c_str());
	LPTSTR lr_zhe = (LPTSTR)bstr_zhe;
	mciSendString(lr_zhe, NULL, 0, NULL);

	zhe++;

	return;
}

// 取运行目录
string getRunPath()
{
	char buf[MAX_PATH];
	GetModuleFileNameA(NULL, buf, MAX_PATH);
	string::size_type pos = string(buf).find_last_of("\\/");
	string dir = string(buf).substr(0, pos);
	return dir;
}


// 初始化
void init_res(int a)
{
	string prefix = "./sounds/";
	if (_access(prefix.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(prefix.c_str());							// 则创建
	}
	string aa = "./sounds/random";
	if (_access(aa.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(aa.c_str());							// 则创建
	}
	string b = "./sounds/keyname";
	if (_access(b.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(b.c_str());							// 则创建
	}
	string c = "./sounds/keyname_default";
	if (_access(c.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(c.c_str());							// 则创建
	}
	string d = "./sounds/mouse";
	if (_access(d.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(d.c_str());							// 则创建
	}
	string e = "./sounds/piano";
	if (_access(e.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(e.c_str());							// 则创建
	}
	string f = "./sounds/yasuo";
	if (_access(f.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(f.c_str());							// 则创建
	}
	f = "./sounds/zhixia";
	if (_access(f.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(f.c_str());							// 则创建
	}
	f = "./sounds/majiang";
	if (_access(f.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(f.c_str());							// 则创建
	}
	f = "./sounds/fengmingqi";
	if (_access(f.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(f.c_str());							// 则创建
	}
	f = "./sounds/muyu";
	if (_access(f.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(f.c_str());							// 则创建
	}
	f = "./sounds/qifengle";
	if (_access(f.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(f.c_str());							// 则创建
	}
	f = "./sounds/cedar";
	if (_access(f.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(f.c_str());							// 则创建
	}
	f = "./sounds/cedar_default";
	if (_access(f.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(f.c_str());							// 则创建
	}
	f = "./sounds/yin";
	if (_access(f.c_str(), 0) == -1) { // 如果文件夹不存在
		_mkdir(f.c_str());							// 则创建
	}
	if (a == 1) {
		// 打开sounds文件夹
		// 用system打开同级目录下的sounds文件夹
		string dir = getRunPath();
		string cmd = "explorer " + dir + "\\sounds";
		system(cmd.c_str());
	}
	// 最后写入一些自带的基础音效
	// 以后用户可以自定义修改
}

// 文件夹遍历
void getFileNames(string path, vector<string>& files)
{
	//文件句柄
	//注意：我发现有些文章代码此处是long类型，实测运行中会报错访问异常
	intptr_t hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("/*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,递归查找
			//如果不是,把文件绝对路径存入vector中
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFileNames(p.assign(path).append("/").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("/").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

string keynameFun(DWORD code) {
	string key;
	switch (code)
	{
	case 0x30:
	case 0x60:
		cout << "检测到按键："
			<< "0" << endl;
		key = "0";
		break;
	case 0x31:
	case 0x61:
		cout << "检测到按键："
			<< "1" << endl;
		key = "1";
		break;
	case 0x32:
	case 0x62:
		cout << "检测到按键："
			<< "2" << endl;
		key = "2";
		break;
	case 0x33:
	case 0x63:
		cout << "检测到按键："
			<< "3" << endl;
		key = "3";
		break;
	case 0x34:
	case 0x64:
		cout << "检测到按键："
			<< "4" << endl;
		key = "4";
		break;
	case 0x35:
	case 0x65:
		cout << "检测到按键："
			<< "5" << endl;
		key = "5";
		break;
	case 0x36:
	case 0x66:
		cout << "检测到按键："
			<< "6" << endl;
		key = "6";
		break;
	case 0x37:
	case 0x67:
		cout << "检测到按键："
			<< "7" << endl;
		key = "7";
		break;
	case 0x38:
	case 0x68:
		cout << "检测到按键："
			<< "8" << endl;
		key = "8";
		break;
	case 0x39:
	case 0x69:
		cout << "检测到按键："
			<< "9" << endl;
		key = "9";
		break;
	case 0x41:
		cout << "检测到按键："
			<< "A" << endl;
		key = "A";
		break;
	case 0x42:
		cout << "检测到按键："
			<< "B" << endl;
		key = "B";
		break;
	case 0x43:
		cout << "检测到按键："
			<< "C" << endl;
		key = "C";
		break;
	case 0x44:
		cout << "检测到按键："
			<< "D" << endl;
		key = "D";
		break;
	case 0x45:
		cout << "检测到按键："
			<< "E" << endl;
		key = "E";
		break;
	case 0x46:
		cout << "检测到按键："
			<< "F" << endl;
		key = "F";
		break;
	case 0x47:
		cout << "检测到按键："
			<< "G" << endl;
		key = "G";
		break;
	case 0x48:
		cout << "检测到按键："
			<< "H" << endl;
		key = "H";
		break;
	case 0x49:
		cout << "检测到按键："
			<< "I" << endl;
		key = "I";
		break;
	case 0x4A:
		cout << "检测到按键："
			<< "J" << endl;
		key = "J";
		break;
	case 0x4B:
		cout << "检测到按键："
			<< "K" << endl;
		key = "K";
		break;
	case 0x4C:
		cout << "检测到按键："
			<< "L" << endl;
		key = "L";
		break;
	case 0x4D:
		cout << "检测到按键："
			<< "M" << endl;
		key = "M";
		break;
	case 0x4E:
		cout << "检测到按键："
			<< "N" << endl;
		key = "N";
		break;
	case 0x4F:
		cout << "检测到按键："
			<< "O" << endl;
		key = "O";
		break;
	case 0x50:
		cout << "检测到按键："
			<< "P" << endl;
		key = "P";
		break;
	case 0x51:
		cout << "检测到按键："
			<< "Q" << endl;
		key = "Q";
		break;
	case 0x52:
		cout << "检测到按键："
			<< "R" << endl;
		key = "R";
		break;
	case 0x53:
		cout << "检测到按键："
			<< "S" << endl;
		key = "S";
		break;
	case 0x54:
		cout << "检测到按键："
			<< "T" << endl;
		key = "T";
		break;
	case 0x55:
		cout << "检测到按键："
			<< "U" << endl;
		key = "U";
		break;
	case 0x56:
		cout << "检测到按键："
			<< "V" << endl;
		key = "V";
		break;
	case 0x57:
		cout << "检测到按键："
			<< "W" << endl;
		key = "W";
		break;
	case 0x58:
		cout << "检测到按键："
			<< "X" << endl;
		key = "X";
		break;
	case 0x59:
		cout << "检测到按键："
			<< "Y" << endl;
		key = "Y";
		break;
	case 0x5A:
		cout << "检测到按键："
			<< "Z" << endl;
		key = "Z";
		break;
	case 0x6A:
		cout << "检测到按键："
			<< "*" << endl;
		key = "*";
		break;
	case 0x6B:
		cout << "检测到按键："
			<< "+" << endl;
		key = "+";
		break;
	case 0x6D:
		cout << "检测到按键："
			<< "-" << endl;
		key = "-";
		break;
	case 0x6E:
		cout << "检测到按键："
			<< "." << endl;
		key = ".";
		break;
	case 0x6F:
		cout << "检测到按键："
			<< "/" << endl;
		key = "/";
		break;
	case 0x0D:
		cout << "检测到按键："
			<< "Enter" << endl;
		key = "Enter";
		break;
	case 0xA0:
	case 0xA1:
		cout << "检测到按键："
			<< "Shift" << endl;
		key = "Shift";
		break;
	case 0x08:
		cout << "检测到按键："
			<< "Backspace" << endl;
		key = "Backspace";
		break;
	case 0x20:
		cout << "检测到按键："
			<< "Space" << endl;
		key = "Space";
		break;
	case 162:
		cout << "检测到按键："
			<< "Ctrl" << endl;
		key = "Ctrl";
		break;
	default:
		cout << "其他按键：" << endl;
		key = "default";
		break;
	}

	return key;
}

HHOOK keyboardHook = 0; // 钩子句柄
static DWORD keyboardThreadId = 0;
static HANDLE keyboardReady = NULL;
static atomic<DWORD> keyboardHookError(0), mouseHookError(0);
static atomic<bool> hooksInstalled(false);
#pragma comment(lib, "advapi32.lib")
// Query our own token only; never change integrity, privilege or message filters.
static DWORD integrityLevel() {
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
static const DWORD processIntegrity = integrityLevel();
static const bool restrictedIntegrity = processIntegrity < SECURITY_MANDATORY_MEDIUM_RID;
// Startup diagnostics run outside hook callbacks (never log or do I/O in a hook).
static void reportWin32(const wchar_t* operation, DWORD error) {
    wchar_t detail[512] = {};
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, 0, detail, 512, NULL);
    wchar_t line[768] = {};
    swprintf_s(line, L"keysound: %s: error=%lu %s\n", operation, error, detail);
    OutputDebugStringW(line);
    fwprintf(stderr, L"%s", line);
}
// 按键变量
string key = "";
string prefix = "/sounds/keyname/";
// 模式flag 0是播放按钮名.wav 1是随机播放
atomic<int> flag(3);//指定
clock_t startTime, endTime, cha;
int index = 0;

// JSON packs are loaded on the tray thread and copied under a mutex for the hook.
static mutex packMutex;
static soundpack::Pack activePack;
static vector<soundpack::Pack> availablePacks;
static const int IDR_PACK_BASE = 1000;
static const int IDR_PACK_LIMIT = 30000;

static wstring runPathW() {
    vector<wchar_t> path(512);
    for (;;) {
        DWORD n = GetModuleFileNameW(NULL, path.data(), (DWORD)path.size());
        if (!n) return L"";
        if (n < path.size() - 1) { wstring s(path.data(), n); size_t pos = s.find_last_of(L"\\/"); return pos == wstring::npos ? L"" : s.substr(0, pos); }
        if (path.size() >= 32768) return L"";
        path.resize(path.size() * 2);
    }
}
static void discoverPacks(const wstring& base = runPathW()) {
    availablePacks.clear();
    wstring root = base + L"\\sounds";
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((root + L"\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || wcscmp(fd.cFileName,L".")==0 || wcscmp(fd.cFileName,L"..")==0) continue;
        soundpack::Pack p;
        if (soundpack::load(root + L"\\" + fd.cFileName, p) && availablePacks.size() < IDR_PACK_LIMIT-IDR_PACK_BASE) availablePacks.push_back(p);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}
static wstring packKey(const KBDLLHOOKSTRUCT &k) {
    DWORD v=k.vkCode;
    if (v >= 'A' && v <= 'Z') return wstring(1, (wchar_t)v);
    if (v >= '0' && v <= '9') return wstring(1, (wchar_t)v);
    if (v >= VK_NUMPAD0 && v <= VK_NUMPAD9) return wstring(1, (wchar_t)(L'0'+v-VK_NUMPAD0));
    if (v >= VK_F1 && v <= VK_F24) return L"F" + to_wstring(v-VK_F1+1);
    switch(v) {
    case VK_LSHIFT: return L"L Shift"; case VK_RSHIFT: return L"R Shift";
    case VK_SHIFT: return k.scanCode==0x36 ? L"R Shift" : L"L Shift";
    case VK_LCONTROL: return L"L Ctrl"; case VK_RCONTROL: return L"R Ctrl";
    case VK_CONTROL: return (k.flags & LLKHF_EXTENDED) ? L"R Ctrl" : L"L Ctrl";
    case VK_LMENU: return L"L Alt"; case VK_RMENU: return L"R Alt";
    case VK_MENU: return (k.flags & LLKHF_EXTENDED) ? L"R Alt" : L"L Alt";
    case VK_LWIN: case VK_RWIN: return L"Win";
    case VK_ESCAPE:return L"Esc"; case VK_SCROLL:return L"SrcLk"; case VK_PAUSE:return L"Pause";
    case VK_DELETE:return L"Del"; case VK_BACK:return L"Backspace"; case VK_INSERT:return L"Ins";
    case VK_HOME:return L"Home"; case VK_PRIOR:return L"PgUp"; case VK_NEXT:return L"PgDn"; case VK_END:return L"End";
    case VK_TAB:return L"Tab"; case VK_CAPITAL:return L"Caps"; case VK_SPACE:return L"Space"; case VK_RETURN:return L"Enter";
    case VK_LEFT:return L"\u2190"; case VK_UP:return L"\u2191"; case VK_DOWN:return L"\u2193"; case VK_RIGHT:return L"\u2192";
    case VK_OEM_3:return L"~"; case VK_OEM_MINUS:return L"-"; case VK_OEM_PLUS:return L"=";
    case VK_OEM_5:return L"\\"; case VK_OEM_6:return L"] }"; case VK_OEM_4:return L"[ {";
    case VK_OEM_7:return L"'"; case VK_OEM_1:return L"; :"; case VK_OEM_PERIOD:return L". >";
    case VK_OEM_COMMA:return L", <"; case VK_OEM_2:return L"/ ?";
    default:return L"";
    }
}
// The worker owns the entire MemoryAudio lifetime (including COM and destruction).
// Tests may define KEYSOUND_MEMORY_AUDIO_TYPE to an already-declared fake with
// the same API before including this file; no MF/XAudio2 header is then required.
#ifndef KEYSOUND_MEMORY_AUDIO_TYPE
#include "memory_audio.h"
#define KEYSOUND_MEMORY_AUDIO_TYPE keysound::MemoryAudio
#endif
static atomic<unsigned long> packGeneration(0);
class PackPlayer {
    using Audio = KEYSOUND_MEMORY_AUDIO_TYPE;
    struct Event {
        KBDLLHOOKSTRUCT key{};
        bool interrupt = false;
        unsigned long generation = 0;
        bool warm = false;
        ULONGLONG submitted = 0;
    };
    mutex queueMutex;
    condition_variable wake;
    deque<Event> queue;
    thread worker;
    atomic<bool> stopping{false};
    atomic<unsigned long> readyGeneration{0};

    bool cancelled(unsigned long generation) const {
        return stopping.load() || generation != packGeneration.load() || flag.load() != 13;
    }
    void observe(Audio& audio) {
        cachedBytes.store(audio.cached_bytes());
        cachedCount.store(audio.cached_count());
        voiceCount.store(audio.voice_count());
    }
    // MemoryAudio::reset destroys every source voice BEFORE releasing PCM.
    // This must also run for a warm request that switches back to a built-in mode.
    HRESULT reset(Audio& audio) {
        HRESULT hr = audio.reset();
        observe(audio);
        return hr;
    }
    void finishWarm(unsigned long generation, HRESULT hr, bool loaded) {
        lock_guard<mutex> lock(queueMutex);
        // Never let completion of an old decode overwrite a newer warm request.
        if (stopping.load() || generation != packGeneration.load()) return;
        lastError.store(hr);
        if (FAILED(hr)) errorCount.fetch_add(1);
        readyGeneration.store(generation);
        readyState.store(loaded && flag.load() == 13);
        loading.store(false);
    }
    void warm(Audio& audio, bool& initialized, soundpack::Pack& snapshot, unsigned long generation) {
        HRESULT hr = reset(audio);
        snapshot = soundpack::Pack{};
        if (FAILED(hr)) { finishWarm(generation, hr, false); return; }
        if (cancelled(generation)) { finishWarm(generation, S_OK, false); return; }
        {
            lock_guard<mutex> lock(packMutex);
            if (!cancelled(generation)) snapshot = activePack;
        }
        if (cancelled(generation)) { finishWarm(generation, S_OK, false); return; }
        vector<wstring> paths;
        if (snapshot.mode == L"\u6307\u5b9a") {
            for (const auto& item : snapshot.assigned) paths.push_back(item.second);
        } else if (snapshot.mode == L"\u91cd\u590d" && !snapshot.repeat.empty()) {
            paths.push_back(snapshot.directory + L"\\sounds\\" + snapshot.repeat);
        } else if (snapshot.mode == L"\u968f\u673a") paths = snapshot.sounds;
        if (paths.empty()) { finishWarm(generation, HRESULT_FROM_WIN32(ERROR_NOT_FOUND), false); return; }
        if (!initialized) {
            hr = audio.initialize();
            if (FAILED(hr)) { finishWarm(generation, hr, false); return; }
            initialized = true;
        }
        // No keyboard event can interrupt warm-up or become a deferred playback.
        hr = audio.preload(paths, [this, generation] { return cancelled(generation); });
        observe(audio);
        if (FAILED(hr) || cancelled(generation)) {
            // preload may retain partial success: package publication is all-or-nothing.
            HRESULT resetHr = reset(audio);
            snapshot = soundpack::Pack{};
            if (FAILED(resetHr)) hr = resetHr;
            finishWarm(generation, hr, false);
            return;
        }
        finishWarm(generation, S_OK, true);
    }
    void run() {
        Audio audio; // Construct AND destroy on this worker, never on the tray/hook.
        bool initialized = false;
        soundpack::Pack snapshot;
        for (;;) {
            Event event;
            {
                unique_lock<mutex> lock(queueMutex);
                wake.wait(lock, [this] { return stopping.load() || !queue.empty(); });
                if (stopping.load()) break;
                event = queue.front(); queue.pop_front();
                queuedCount.store(queue.size());
            }
            try {
                if (event.warm) { warm(audio, initialized, snapshot, event.generation); continue; }
                if (cancelled(event.generation) || !readyState.load() ||
                    event.generation != readyGeneration.load() ||
                    GetTickCount64() - event.submitted > maxEventAgeMs) {
                    droppedCount.fetch_add(1); continue;
                }
                wstring path;
                if (snapshot.mode == L"\u6307\u5b9a") {
                    auto it = snapshot.assigned.find(packKey(event.key));
                    if (it != snapshot.assigned.end()) path = it->second;
                } else if (snapshot.mode == L"\u91cd\u590d" && !snapshot.repeat.empty()) {
                    path = snapshot.directory + L"\\sounds\\" + snapshot.repeat;
                } else if (snapshot.mode == L"\u968f\u673a" && !snapshot.sounds.empty()) {
                    path = snapshot.sounds[(size_t)rand() % snapshot.sounds.size()];
                }
                if (path.empty()) continue;
                // Serialize the final check/submission with prewarm publication:
                // once prewarm returns, no old event can start a new voice.
                // submit still uses try_lock and drops rather than waiting here.
                lock_guard<mutex> lock(queueMutex);
                if (cancelled(event.generation) || !readyState.load() ||
                    GetTickCount64() - event.submitted > maxEventAgeMs) {
                    droppedCount.fetch_add(1); continue;
                }
                // Cached-only API: NEVER fall back to MCI/disk playback on error.
                HRESULT hr = audio.play(path, event.interrupt ? Audio::PlayMode::interrupt : Audio::PlayMode::overlap);
                observe(audio);
                if (SUCCEEDED(hr)) playCount.fetch_add(1);
                else {
                    errorCount.fetch_add(1);
                    if (!cancelled(event.generation)) lastError.store(hr);
                }
            } catch (const std::bad_alloc&) {
                reset(audio);
                finishWarm(event.generation, E_OUTOFMEMORY, false);
            } catch (...) {
                reset(audio);
                finishWarm(event.generation, E_FAIL, false);
            }
        }
        HRESULT hr = audio.shutdown();
        cachedBytes.store(0); cachedCount.store(0); voiceCount.store(0);
        if (FAILED(hr)) { lastError.store(hr); errorCount.fetch_add(1); }
        loading.store(false); readyState.store(false);
    }
public:
    // Independent atomic UI observations, not a transactionally consistent snapshot.
    // playCount counts successful submissions to XAudio2, not audible completions.
    static constexpr size_t maxQueue = 64;
    static constexpr ULONGLONG maxEventAgeMs = 100;
    atomic<size_t> cachedBytes{0}, cachedCount{0}, voiceCount{0}, queuedCount{0};
    atomic<bool> loading{false}, readyState{false};
    atomic<HRESULT> lastError{S_OK};
    atomic<unsigned long long> playCount{0}, errorCount{0}, droppedCount{0}, enqueueCount{0};

    void start() {
        lock_guard<mutex> lock(queueMutex);
        if (!stopping.load() && !worker.joinable()) worker = thread(&PackPlayer::run, this);
    }
    void submit(const KBDLLHOOKSTRUCT& key, bool interrupt) {
        // Hook path: no waiting, initialization, decoder, file access or new thread.
        if (!readyState.load() || loading.load() || stopping.load() || flag.load() != 13) {
            droppedCount.fetch_add(1); return;
        }
        unique_lock<mutex> lock(queueMutex, try_to_lock);
        const unsigned long generation = packGeneration.load();
        if (!lock || stopping.load() || !readyState.load() || loading.load() ||
            generation != readyGeneration.load() || queue.size() >= maxQueue) {
            droppedCount.fetch_add(1); return;
        }
        Event event{};
        event.key = key; event.interrupt = interrupt; event.generation = generation;
        event.submitted = GetTickCount64();
        try { queue.push_back(event); }
        catch (...) { droppedCount.fetch_add(1); return; }
        queuedCount.store(queue.size()); enqueueCount.fetch_add(1);
        lock.unlock();
        wake.notify_one();
    }
    void prewarm() {
        {
            lock_guard<mutex> lock(queueMutex);
            if (stopping.load()) return;
            // Also bump here: repeated prewarm calls cancel even without a caller bump.
            Event event{}; event.generation = ++packGeneration; event.warm = true;
            readyState.store(false); loading.store(true); lastError.store(S_OK);
            for (const auto& pending : queue) if (!pending.warm) droppedCount.fetch_add(1);
            queue.clear(); queuedCount.store(0);
            try { queue.push_back(event); }
            catch (...) {
                // A warm command still needs to reach the worker to release old PCM.
                stopping.store(true); loading.store(false);
                lastError.store(E_OUTOFMEMORY); errorCount.fetch_add(1);
            }
        }
        wake.notify_one();
    }
    void shutdown() {
        stopping.store(true); // Decoder cancellation must not wait for queueMutex/join.
        {
            lock_guard<mutex> lock(queueMutex);
            readyState.store(false);
            for (const auto& pending : queue) if (!pending.warm) droppedCount.fetch_add(1);
            queue.clear(); queuedCount.store(0);
        }
        wake.notify_one();
        if (worker.joinable()) worker.join();
        loading.store(false);
    }
    ~PackPlayer() { shutdown(); }
};
static PackPlayer packPlayer;

// Legacy processing runs only on the bounded worker, with a copied hook event.
static LRESULT processLegacyKeyboard(int nCode, WPARAM wParam, LPARAM lParam, int mode, bool interrupt)
{
	const int flag = mode;
	const bool flag_fast = interrupt;
	KBDLLHOOKSTRUCT* ks = (KBDLLHOOKSTRUCT*)lParam;
	/*
	typedef struct tagKBDLLHOOKSTRUCT {
		DWORD     vkCode;		// 按键代号
		DWORD     scanCode;		// 硬件扫描代号，同 vkCode 也可以作为按键的代号。sad
		DWORD     flags;		// 事件类型，一般按键按下为 0 抬起为 128。
		DWORD     time;			// 消息时间戳
		ULONG_PTR dwExtraInfo;	// 消息附加信息，一般为 0。
	}KBDLLHOOKSTRUCT,*LPKBDLLHOOKSTRUCT,*PKBDLLHOOKSTRUCT;
	*/
	// if (ks->flags == 128 || ks->flags == 129)
	cout << "flags: " << ks->flags << endl;

	//if (ks->flags == 0)
	if (ks->flags == 128 || ks->flags == 129)
	{
		endTime = clock();
		cha = endTime - startTime;
		startTime = clock();
		cout << (long)cha << endl;
		if (flag == 2)//键名模式
		{
			cout << "按键--9999--：" << ks->vkCode << endl;
			// 监控键盘
			switch (ks->vkCode)
			{
			case 0x30:
			case 0x60:
				cout << "检测到按键："
					<< "0" << endl;
				key = "0";
				break;
			case 0x31:
			case 0x61:
				cout << "检测到按键："
					<< "1" << endl;
				key = "1";
				break;
			case 0x32:
			case 0x62:
				cout << "检测到按键："
					<< "2" << endl;
				key = "2";
				break;
			case 0x33:
			case 0x63:
				cout << "检测到按键："
					<< "3" << endl;
				key = "3";
				break;
			case 0x34:
			case 0x64:
				cout << "检测到按键："
					<< "4" << endl;
				key = "4";
				break;
			case 0x35:
			case 0x65:
				cout << "检测到按键："
					<< "5" << endl;
				key = "5";
				break;
			case 0x36:
			case 0x66:
				cout << "检测到按键："
					<< "6" << endl;
				key = "6";
				break;
			case 0x37:
			case 0x67:
				cout << "检测到按键："
					<< "7" << endl;
				key = "7";
				break;
			case 0x38:
			case 0x68:
				cout << "检测到按键："
					<< "8" << endl;
				key = "8";
				break;
			case 0x39:
			case 0x69:
				cout << "检测到按键："
					<< "9" << endl;
				key = "9";
				break;
			case 0x41:
				cout << "检测到按键："
					<< "A" << endl;
				key = "A";
				break;
			case 0x42:
				cout << "检测到按键："
					<< "B" << endl;
				key = "B";
				break;
			case 0x43:
				cout << "检测到按键："
					<< "C" << endl;
				key = "C";
				break;
			case 0x44:
				cout << "检测到按键："
					<< "D" << endl;
				key = "D";
				break;
			case 0x45:
				cout << "检测到按键："
					<< "E" << endl;
				key = "E";
				break;
			case 0x46:
				cout << "检测到按键："
					<< "F" << endl;
				key = "F";
				break;
			case 0x47:
				cout << "检测到按键："
					<< "G" << endl;
				key = "G";
				break;
			case 0x48:
				cout << "检测到按键："
					<< "H" << endl;
				key = "H";
				break;
			case 0x49:
				cout << "检测到按键："
					<< "I" << endl;
				key = "I";
				break;
			case 0x4A:
				cout << "检测到按键："
					<< "J" << endl;
				key = "J";
				break;
			case 0x4B:
				cout << "检测到按键："
					<< "K" << endl;
				key = "K";
				break;
			case 0x4C:
				cout << "检测到按键："
					<< "L" << endl;
				key = "L";
				break;
			case 0x4D:
				cout << "检测到按键："
					<< "M" << endl;
				key = "M";
				break;
			case 0x4E:
				cout << "检测到按键："
					<< "N" << endl;
				key = "N";
				break;
			case 0x4F:
				cout << "检测到按键："
					<< "O" << endl;
				key = "O";
				break;
			case 0x50:
				cout << "检测到按键："
					<< "P" << endl;
				key = "P";
				break;
			case 0x51:
				cout << "检测到按键："
					<< "Q" << endl;
				key = "Q";
				break;
			case 0x52:
				cout << "检测到按键："
					<< "R" << endl;
				key = "R";
				break;
			case 0x53:
				cout << "检测到按键："
					<< "S" << endl;
				key = "S";
				break;
			case 0x54:
				cout << "检测到按键："
					<< "T" << endl;
				key = "T";
				break;
			case 0x55:
				cout << "检测到按键："
					<< "U" << endl;
				key = "U";
				break;
			case 0x56:
				cout << "检测到按键："
					<< "V" << endl;
				key = "V";
				break;
			case 0x57:
				cout << "检测到按键："
					<< "W" << endl;
				key = "W";
				break;
			case 0x58:
				cout << "检测到按键："
					<< "X" << endl;
				key = "X";
				break;
			case 0x59:
				cout << "检测到按键："
					<< "Y" << endl;
				key = "Y";
				break;
			case 0x5A:
				cout << "检测到按键："
					<< "Z" << endl;
				key = "Z";
				break;
			case 0x6A:
				cout << "检测到按键："
					<< "*" << endl;
				key = "*";
				break;
			case 0x6B:
				cout << "检测到按键："
					<< "+" << endl;
				key = "+";
				break;
			case 0x6D:
				cout << "检测到按键："
					<< "-" << endl;
				key = "-";
				break;
			case 0x6E:
				cout << "检测到按键："
					<< "." << endl;
				key = ".";
				break;
			case 0x6F:
				cout << "检测到按键："
					<< "/" << endl;
				key = "/";
				break;
			case 0x0D:
				cout << "检测到按键："
					<< "Enter" << endl;
				key = "Enter";
				break;
			case 0xA0:
			case 0xA1:
				cout << "检测到按键："
					<< "Shift" << endl;
				key = "Shift";
				break;
			case 0x08:
				cout << "检测到按键："
					<< "Backspace" << endl;
				key = "Backspace";
				break;
			case 0x20:
				cout << "检测到按键："
					<< "Space" << endl;
				key = "Space";
				break;
			case 162:
				cout << "检测到按键："
					<< "Ctrl" << endl;
				key = "Ctrl";
				break;
			default:
				cout << "其他按键：" << endl;
				key = "default";
				break;
			}
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path_keyname("./sounds/keyname");
			getFileNames(path_keyname, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}

			boolean flag_keyname = false;
			for (vector<string>::iterator ite = fileNames.begin(); ite != fileNames.end(); ite++)
			{
				cout << *ite << " ";

				cout << (*ite).substr(17, (*ite).size() - 21) << endl;
				if ((*ite).substr(17, (*ite).size() - 21) == key) {

					flag_keyname = true;
					break;
				}
			}
			
			if (flag_keyname) {
				string path = prefix + key + ".wav";
				// 播放key.wav音效
				// cout << "播放音效" << path;
				// 取运行目录
				string dir = getRunPath();
				// 拼接音效文件路径
				path = dir + path;
				cout << "播放音效" << path + "\n";
				// 播放音效
				//playtheSound(path);
				// 线程分离
				//
				//playtheSound(path);
				if (flag_fast) {
					playtheSound(path);
				}
				else {
					playtheSound(path);
					
				}
			}
			else{
				vector<string> fileNames_2;
				string path_keyname("./sounds/keyname_default");
				getFileNames(path_keyname, fileNames_2);
				// 如果fileNames_2为空，直接返回
				if (fileNames_2.empty())
				{
					cout << "随机模式default文件为空" << endl;
					return 0;
				}
				int index = randEx(0, fileNames_2.size() - 1);
				// 取运行目录
				string dir = getRunPath();
				cout << dir << endl;
				string path_ = dir + fileNames_2[index].substr(1);
				
				if (flag_fast) {
					playtheSound(path_);
				}
				else {
					// 创建线程
					// 线程分离
					playtheSound(path_);
					
				}
			}
		}
		else if (flag == 1)//随机模式
		{
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path("./sounds/random");
			getFileNames(path, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}
			// 随机抽取容器中的一个文件名
			int index = randEx(0, fileNames.size() - 1);
			/*
			if (cha < 1000) {
				index++;
				if (index == fileNames.size()) {//旋律连续啊
					index = 0;
				}
			}
			else {
				index = 8 * randEx(0, fileNames.size() / 8 - 1);
				cout << "随机：" << index << endl;
				cout << fileNames[index] << endl;//./sounds/random/301.wav
			}
			*/
			// 取运行目录
			string dir = getRunPath();
			cout << dir << endl;
			//string path_ = dir + fileNames[index].substr(1, dir.size() - 1);
			//string path_ = dir + fileNames[index].substr(1, fileNames[index].size() - 1);
			string path_ = dir + fileNames[index].substr(1);
			// 创建线程
			//playtheSound(path_);
			// 线程分离
			//
			if (flag_fast) {
				playtheSound(path_);
			}
			else {
				playtheSound(path_);
				
			}
		}
		else if (flag == 3)//piano
		{
			cout << "按键--9999--：" << ks->vkCode << endl;
			// 监控键盘
			switch (ks->vkCode)
			{
			case 0x30:
			case 0x60:
				cout << "检测到按键："
					<< "0" << endl;
				key = "0";
				break;
			case 0x31:
			case 0x61:
				cout << "检测到按键："
					<< "1" << endl;
				key = "1";
				break;
			case 0x32:
			case 0x62:
				cout << "检测到按键："
					<< "2" << endl;
				key = "2";
				break;
			case 0x33:
			case 0x63:
				cout << "检测到按键："
					<< "3" << endl;
				key = "3";
				break;
			case 0x34:
			case 0x64:
				cout << "检测到按键："
					<< "4" << endl;
				key = "4";
				break;
			case 0x35:
			case 0x65:
				cout << "检测到按键："
					<< "5" << endl;
				key = "5";
				break;
			case 0x36:
			case 0x66:
				cout << "检测到按键："
					<< "6" << endl;
				key = "6";
				break;
			case 0x37:
			case 0x67:
				cout << "检测到按键："
					<< "7" << endl;
				key = "7";
				break;
			case 0x38:
			case 0x68:
				cout << "检测到按键："
					<< "8" << endl;
				key = "8";
				break;
			case 0x39:
			case 0x69:
				cout << "检测到按键："
					<< "9" << endl;
				key = "9";
				break;
			case 0x41:
				cout << "检测到按键："
					<< "A" << endl;
				key = "A";
				break;
			case 0x42:
				cout << "检测到按键："
					<< "B" << endl;
				key = "B";
				break;
			case 0x43:
				cout << "检测到按键："
					<< "C" << endl;
				key = "C";
				break;
			case 0x44:
				cout << "检测到按键："
					<< "D" << endl;
				key = "D";
				break;
			case 0x45:
				cout << "检测到按键："
					<< "E" << endl;
				key = "E";
				break;
			case 0x46:
				cout << "检测到按键："
					<< "F" << endl;
				key = "F";
				break;
			case 0x47:
				cout << "检测到按键："
					<< "G" << endl;
				key = "G";
				break;
			case 0x48:
				cout << "检测到按键："
					<< "H" << endl;
				key = "H";
				break;
			case 0x49:
				cout << "检测到按键："
					<< "I" << endl;
				key = "I";
				break;
			case 0x4A:
				cout << "检测到按键："
					<< "J" << endl;
				key = "J";
				break;
			case 0x4B:
				cout << "检测到按键："
					<< "K" << endl;
				key = "K";
				break;
			case 0x4C:
				cout << "检测到按键："
					<< "L" << endl;
				key = "L";
				break;
			case 0x4D:
				cout << "检测到按键："
					<< "M" << endl;
				key = "M";
				break;
			case 0x4E:
				cout << "检测到按键："
					<< "N" << endl;
				key = "N";
				break;
			case 0x4F:
				cout << "检测到按键："
					<< "O" << endl;
				key = "O";
				break;
			case 0x50:
				cout << "检测到按键："
					<< "P" << endl;
				key = "P";
				break;
			case 0x51:
				cout << "检测到按键："
					<< "Q" << endl;
				key = "Q";
				break;
			case 0x52:
				cout << "检测到按键："
					<< "R" << endl;
				key = "R";
				break;
			case 0x53:
				cout << "检测到按键："
					<< "S" << endl;
				key = "S";
				break;
			case 0x54:
				cout << "检测到按键："
					<< "T" << endl;
				key = "T";
				break;
			case 0x55:
				cout << "检测到按键："
					<< "U" << endl;
				key = "U";
				break;
			case 0x56:
				cout << "检测到按键："
					<< "V" << endl;
				key = "V";
				break;
			case 0x57:
				cout << "检测到按键："
					<< "W" << endl;
				key = "W";
				break;
			case 0x58:
				cout << "检测到按键："
					<< "X" << endl;
				key = "X";
				break;
			case 0x59:
				cout << "检测到按键："
					<< "Y" << endl;
				key = "Y";
				break;
			case 0x5A:
				cout << "检测到按键："
					<< "Z" << endl;
				key = "Z";
				break;
			case 0x6A:
				cout << "检测到按键："
					<< "*" << endl;
				key = "*";
				break;
			case 0x6B:
				cout << "检测到按键："
					<< "+" << endl;
				key = "+";
				break;
			case 0x6D:
				cout << "检测到按键："
					<< "-" << endl;
				key = "-";
				break;
			case 0x6E:
				cout << "检测到按键："
					<< "." << endl;
				key = ".";
				break;
			case 0x6F:
				cout << "检测到按键："
					<< "/" << endl;
				key = "/";
				break;
			case 0x0D:
				cout << "检测到按键："
					<< "Enter" << endl;
				key = "Enter";
				break;
			case 0xA0:
			case 0xA1:
				cout << "检测到按键："
					<< "Shift" << endl;
				key = "Shift";
				break;
			case 0x08:
				cout << "检测到按键："
					<< "Backspace" << endl;
				key = "Backspace";
				break;
			case 0x20:
				cout << "检测到按键："
					<< "Space" << endl;
				key = "Space";
				break;
			case 162:
				cout << "检测到按键："
					<< "Ctrl" << endl;
				key = "Ctrl";
				break;
			default:
				cout << "其他按键：" << endl;
				key = "default";
				break;
			}
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path_keyname("./sounds/piano");
			getFileNames(path_keyname, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}

			boolean flag_keyname = false;
			for (vector<string>::iterator ite = fileNames.begin(); ite != fileNames.end(); ite++)
			{
				cout << *ite << " ";

				cout << (*ite).substr(15, (*ite).size() - 19) << endl;
				if ((*ite).substr(15, (*ite).size() - 19) == key) {

					flag_keyname = true;
					break;
				}
			}
			if (!flag_keyname) {
				flag_keyname = true;
				key = "default";
			}
			if (flag_keyname) {
				string path = "/sounds/piano/" + key + ".wav";
				// 播放key.wav音效
				// cout << "播放音效" << path;
				// 取运行目录
				string dir = getRunPath();
				// 拼接音效文件路径
				path = dir + path;
				cout << "播放音效" << path + "\n";
				// 播放音效
				//playtheSound(path);
				// 线程分离
				//
				//playtheSound(path);
				if (flag_fast) {
					playtheSound(path);
				}
				else {
					playtheSound(path);
					
				}
			}
		}
		else if (flag == 4)//起风了
		{
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path("./sounds/qifengle");
			getFileNames(path, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}
			// 随机抽取容器中的一个文件名
			//int index = randEx(0, fileNames.size() - 1);
			if (cha < 1000) {
				index++;
				if (index == fileNames.size()) {//旋律连续啊
					index = 0;
				}
			}
			else {
				index = 8 * randEx(0, fileNames.size() / 8 - 1);
				cout << "随机：" << index << endl;
				cout << fileNames[index] << endl;//./sounds/random/301.wav
			}
			// 取运行目录
			string dir = getRunPath();
			cout << dir << endl;
			//string path_ = dir + fileNames[index].substr(1, dir.size() - 1);
			//string path_ = dir + fileNames[index].substr(1, fileNames[index].size() - 1);
			string path_ = dir + fileNames[index].substr(1);
			// 创建线程
			//playtheSound(path_);
			// 线程分离
			//
			if (flag_fast) {
				playtheSound(path_);
			}
			else {
				playtheSound(path_);
				
			}
		}
		else if (flag == 5)//疾风剑豪
		{
			string key = keynameFun(ks->vkCode);
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path("./sounds/yasuo");
			getFileNames(path, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}
			boolean flag_keyname = false;
			for (vector<string>::iterator ite = fileNames.begin(); ite != fileNames.end(); ite++)
			{
				cout << *ite << " ";

				cout << (*ite).substr(15, (*ite).size() - 19) << endl;
				if ((*ite).substr(15, (*ite).size() - 19) == key) {

					flag_keyname = true;
					break;
				}
			}
			if (!flag_keyname) {
				flag_keyname = true;
				key = "default";
			}
			if (flag_keyname) {
				string path = "/sounds/yasuo/" + key + ".wav";
				// 取运行目录
				string dir = getRunPath();
				// 拼接音效文件路径
				path = dir + path;
				cout << "播放音效" << path + "\n";
				if (flag_fast) {
					playtheSound(path);
				}
				else {
					playtheSound(path);
					
				}
			}

		}
		else if (flag == 6)//知夏轴
		{
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path("./sounds/zhixia");
			getFileNames(path, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}
			// 随机抽取容器中的一个文件名
			int index = randEx(0, fileNames.size() - 1);
			// 取运行目录
			string dir = getRunPath();
			cout << dir << endl;
			//string path_ = dir + fileNames[index].substr(1, dir.size() - 1);
			//string path_ = dir + fileNames[index].substr(1, fileNames[index].size() - 1);
			string path_ = dir + fileNames[index].substr(1);
			if (flag_fast) {
				playtheSound(path_);
			}
			else {
				// 创建线程
				// 线程分离
				playtheSound(path_);
				
			}
		}
		else if (flag == 7)//麻将音
		{
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path("./sounds/majiang");
			getFileNames(path, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}
			// 随机抽取容器中的一个文件名
			int index = randEx(0, fileNames.size() - 1);
			// 取运行目录
			string dir = getRunPath();
			cout << dir << endl;
			//string path_ = dir + fileNames[index].substr(1, dir.size() - 1);
			//string path_ = dir + fileNames[index].substr(1, fileNames[index].size() - 1);
			string path_ = dir + fileNames[index].substr(1);
			if (flag_fast) {
				playtheSound(path_);
			}
			else {
				// 创建线程
				// 线程分离
				playtheSound(path_);
				
			}
		}
		else if (flag == 8)//蜂鸣器
		{
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path("./sounds/fengmingqi");
			getFileNames(path, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}
			// 随机抽取容器中的一个文件名
			int index = randEx(0, fileNames.size() - 1);
			// 取运行目录
			string dir = getRunPath();
			cout << dir << endl;
			//string path_ = dir + fileNames[index].substr(1, dir.size() - 1);
			//string path_ = dir + fileNames[index].substr(1, fileNames[index].size() - 1);
			string path_ = dir + fileNames[index].substr(1);
			if (flag_fast) {
				playtheSound(path_);
			}
			else {
				// 创建线程
				// 线程分离
				playtheSound(path_);
				
			}
		}
		else if (flag == 9)//木鱼音
		{
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path("./sounds/muyu");
			getFileNames(path, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}
			// 随机抽取容器中的一个文件名
			int index = randEx(0, fileNames.size() - 1);
			// 取运行目录
			string dir = getRunPath();
			cout << dir << endl;
			//string path_ = dir + fileNames[index].substr(1, dir.size() - 1);
			//string path_ = dir + fileNames[index].substr(1, fileNames[index].size() - 1);
			string path_ = dir + fileNames[index].substr(1);
			if (flag_fast) {
				playtheSound(path_);
			}
			else {
				// 创建线程
				// 线程分离
				playtheSound(path_);
				
			}
		}
		else if (flag == 10)//雨滴音
		{
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path("./sounds/yudi");
			getFileNames(path, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}
			// 随机抽取容器中的一个文件名
			int index = randEx(0, fileNames.size() - 1);
			// 取运行目录
			string dir = getRunPath();
			cout << dir << endl;
			//string path_ = dir + fileNames[index].substr(1, dir.size() - 1);
			//string path_ = dir + fileNames[index].substr(1, fileNames[index].size() - 1);
			string path_ = dir + fileNames[index].substr(1);
			if (flag_fast) {
				playtheSound(path_);
			}
			else {
				// 创建线程
				// 线程分离
				playtheSound(path_);
				
			}
		}
		else if (flag == 11)//Cader
		{
			string key = keynameFun(ks->vkCode);
			// 拿到sounds文件夹下的所有文件
			vector<string> fileNames;
			string path("./sounds/cedar");
			getFileNames(path, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}
			boolean flag_keyname = false;
			for (vector<string>::iterator ite = fileNames.begin(); ite != fileNames.end(); ite++)
			{
				cout << *ite << " ";

				cout << (*ite).substr(15, (*ite).size() - 19) << endl;
				if ((*ite).substr(15, (*ite).size() - 19) == key) {

					flag_keyname = true;
					break;
				}
			}
			if (flag_keyname) {
				string path = "/sounds/cedar/" + key + ".wav";
				// 取运行目录
				string dir = getRunPath();
				// 拼接音效文件路径
				path = dir + path;
				cout << "播放音效" << path + "\n";
				if (flag_fast) {
					playtheSound(path);
				}
				else {
					playtheSound(path);
					
				}
			}
			else {
				vector<string> fileNames_2;
				string path_keyname("./sounds/cedar_default");
				getFileNames(path_keyname, fileNames_2);
				// 如果fileNames_2为空，直接返回
				if (fileNames_2.empty())
				{
					cout << "随机模式default文件为空" << endl;
					return 0;
				}
				int index = randEx(0, fileNames_2.size() - 1);
				// 取运行目录
				string dir = getRunPath();
				cout << dir << endl;
				string path_ = dir + fileNames_2[index].substr(1);

				if (flag_fast) {
					playtheSound(path_);
				}
				else {
					// 创建线程
					// 线程分离
					playtheSound(path_);
					
				}
			}
		}
		else if (flag == 12)//音游
		{
		// 拿到sounds文件夹下的所有文件
		vector<string> fileNames;
		string path("./sounds/yin");
		getFileNames(path, fileNames);
		// 如果fileNames为空，直接返回
		if (fileNames.empty())
		{
			cout << "随机模式文件为空" << endl;
			return 0;
		}
		// 随机抽取容器中的一个文件名
		int index = randEx(0, fileNames.size() - 1);
		// 取运行目录
		string dir = getRunPath();
		cout << dir << endl;
		string path_ = dir + fileNames[index].substr(1);
		if (flag_fast) {
			playtheSound(path_);
		}
		else {
			// 创建线程
			// 线程分离
			playtheSound(path_);
			
		}
		}
	}

	// 将消息传递给钩子链中的下一个钩子
	return 0;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

//开启监听鼠标
HHOOK mouse_Hook;

LRESULT CALLBACK LowLevelMouseProc(INT nCode,
	WPARAM wParam,
	LPARAM lParam
);
BOOL UninstallHook();  //卸载  
BOOL InstallKbHook();     //安装


BOOL InstallKbHook()
{

	if (mouse_Hook)   UninstallHook();
	mouse_Hook = SetWindowsHookEx(
		WH_MOUSE_LL,   // 钩子类型，WH_MOUSE_LL 为鼠标钩子
		(HOOKPROC)(LowLevelMouseProc),
		//AfxGetApp()->m_hInstance, 
		//GetModuleHandleA(NULL),
		GetModuleHandle(NULL),
		NULL);
	const DWORD error = mouse_Hook ? ERROR_SUCCESS : GetLastError();
	mouseHookError.store(error);
	if (!mouse_Hook) reportWin32(L"SetWindowsHookExW(WH_MOUSE_LL)", error);
	return(mouse_Hook != NULL);
}
 
BOOL UninstallHook()
{

	BOOL jud = FALSE;
	if (mouse_Hook) {
		jud = UnhookWindowsHookEx(mouse_Hook);
		mouse_Hook = NULL;  //set NULL  
	}

	return jud;
}

atomic<int> flag_mouse(1);
 
static LRESULT processLegacyMouse(INT nCode, WPARAM wParam, LPARAM lParam, bool interrupt)
{
	const bool flag_fast = interrupt;
	MSLLHOOKSTRUCT* pkbhs = (MSLLHOOKSTRUCT*)lParam;
	switch (nCode)
	{

	case HC_ACTION:
	{
		//鼠标左击  
		//if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN || wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP) {
		if (flag_mouse && (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP)) {
			cout << "鼠标被点击了" << endl;

			
			vector<string> fileNames;
			string path("./sounds/mouse");
			getFileNames(path, fileNames);
			// 如果fileNames为空，直接返回
			if (fileNames.empty())
			{
				cout << "随机模式文件为空" << endl;
				return 0;
			}
			// 随机抽取容器中的一个文件名
			int index = randEx(0, fileNames.size() - 1);

			// 取运行目录
			string dir = getRunPath();
			cout << dir << endl;
			string path_mouse = dir + fileNames[index].substr(1);

			if (flag_mouse) {
				if (flag_fast) {
					playtheSound(path_mouse);
				}
				else {
					// 创建线程
					playtheSound(path_mouse);
					// 线程分离
					
				}
			}
		}
	}
		default:break;
	}

	return 0;
}

// Copy hook-owned structures before returning; never wait for the worker on a hook thread.
class LegacyInputWorker {
    struct Event {
        bool mouse;
        WPARAM message;
        KBDLLHOOKSTRUCT keyboard;
        MSLLHOOKSTRUCT mouseData;
        int mode;
        bool interrupt;
    };
    mutex guard;
    condition_variable ready;
    deque<Event> pending;
    thread worker;
    bool stopping = false;
    void run() {
        for (;;) {
            Event event{};
            {
                unique_lock<mutex> lock(guard);
                ready.wait(lock, [this] { return stopping || !pending.empty(); });
                if (stopping) break;
                event = pending.front();
                pending.pop_front();
            }
            if (event.mouse)
                processLegacyMouse(HC_ACTION, event.message, reinterpret_cast<LPARAM>(&event.mouseData), event.interrupt);
            else
                processLegacyKeyboard(HC_ACTION, event.message, reinterpret_cast<LPARAM>(&event.keyboard), event.mode, event.interrupt);
        }
    }
    void submit(const Event& event) {
        unique_lock<mutex> lock(guard, try_to_lock);
        if (!lock || stopping || pending.size() >= 64) return;
        pending.push_back(event);
        lock.unlock();
        ready.notify_one();
    }
public:
    void start() { worker = thread(&LegacyInputWorker::run, this); }
    void keyboard(WPARAM message, const KBDLLHOOKSTRUCT& data, int mode, bool interrupt) {
        Event event{}; event.message = message; event.keyboard = data; event.mode = mode; event.interrupt = interrupt;
        submit(event);
    }
    void mouse(WPARAM message, const MSLLHOOKSTRUCT& data, bool interrupt) {
        Event event{}; event.mouse = true; event.message = message; event.mouseData = data; event.interrupt = interrupt;
        submit(event);
    }
    void shutdown() {
        { lock_guard<mutex> lock(guard); stopping = true; pending.clear(); }
        ready.notify_one();
        if (worker.joinable()) worker.join();
    }
    ~LegacyInputWorker() { shutdown(); }
};
static LegacyInputWorker legacyInputWorker;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && lParam) {
        const int mode = flag.load();
        const KBDLLHOOKSTRUCT& data = *reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        // This state is owned only by the hook thread. Track physical scan codes,
        // including the extended bit, so mode switches do not re-trigger held keys.
        static bool down[512]{};
        const unsigned slot = (data.scanCode & 255u) | ((data.flags & LLKHF_EXTENDED) ? 256u : 0u);
        const bool pressed = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
        const bool released = wParam == WM_KEYUP || wParam == WM_SYSKEYUP;
        if (released) down[slot] = false;
        if (pressed && !down[slot]) {
            down[slot] = true;
            if (mode == 13) packPlayer.submit(data, flag_fast.load() != 0);
            else {
                // Legacy worker expects release flags; enqueue once on physical down.
                KBDLLHOOKSTRUCT legacy = data;
                legacy.flags = LLKHF_UP | (data.flags & LLKHF_EXTENDED);
                legacyInputWorker.keyboard(WM_KEYUP, legacy, mode, flag_fast.load() != 0);
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(INT nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && lParam && flag_mouse.load() &&
        (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP))
        legacyInputWorker.mouse(wParam, *reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam), flag_fast.load() != 0);
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// 鼠标hook
int key_hook2()
{
	// 安装钩子
	//InstallKbHook();
	mouse_Hook = SetWindowsHookEx(
		WH_MOUSE_LL,   // 钩子类型，WH_MOUSE_LL 为鼠标钩子
		(HOOKPROC)(LowLevelMouseProc),
		//AfxGetApp()->m_hInstance, 
		//GetModuleHandleA(NULL),
		GetModuleHandle(NULL),
		NULL);

	// 不可漏掉消息处理，不然程序会卡死
	
	MSG msg;
	while (1)
	{
		// 如果消息队列中有消息
		if (PeekMessageA(
			&msg,			// MSG 接收这个消息
			NULL,			// 检测消息的窗口句柄，NULL：检索当前线程所有窗口消息
			NULL,			// 检查消息范围中第一个消息的值，NULL：检查所有消息（必须和下面的同时为NULL）
			NULL,			// 检查消息范围中最后一个消息的值，NULL：检查所有消息（必须和上面的同时为NULL）
			PM_REMOVE // 处理消息的方式，PM_REMOVE：处理后将消息从队列中删除
		))
		{
			// 把按键消息传递给字符消息
			TranslateMessage(&msg);

			// 将消息分派给窗口程序
			DispatchMessageW(&msg);
		}
		else
			Sleep(100); // 避免CPU全负载运行
	}
	
	// 删除钩子
	//UninstallHook();
	if (mouse_Hook) {
		UnhookWindowsHookEx(mouse_Hook);
		mouse_Hook = NULL;  //set NULL  
	}
	return 0;
}

// 键盘hook
int key_hook()
{
	keyboardThreadId = GetCurrentThreadId();
	MSG initialMessage;
	PeekMessage(&initialMessage, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	// Signal readiness only after both global hooks have a known outcome.
	SetLastError(ERROR_SUCCESS);
	keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandleW(NULL), 0);
	const DWORD error = keyboardHook ? ERROR_SUCCESS : GetLastError();
	keyboardHookError.store(error);
	if (!keyboardHook) reportWin32(L"SetWindowsHookExW(WH_KEYBOARD_LL)", error);
	const BOOL mouseInstalled = InstallKbHook();
	hooksInstalled.store(keyboardHook != NULL && mouseInstalled != FALSE);
	SetEvent(keyboardReady);
	if (!keyboardHook) {
		UninstallHook();
		return -1;
	}

	// 不可漏掉消息处理，不然程序会卡死
	MSG msg{};
	BOOL result;
	while ((result = GetMessageW(&msg, NULL, 0, 0)) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	if (result == -1) reportWin32(L"GetMessageW(hooks)", GetLastError());
	// Delete hooks on their owning message-pump thread.
	UnhookWindowsHookEx(keyboardHook);
	keyboardHook = NULL;
	UninstallHook();
	hooksInstalled.store(false);
	return 0;
}



#define IDR_PAUSE 12
#define IDR_START 13
#define IDR_TEST 14
#define IDR_OPENDIR 18
// 菜单常量
#define IDR_QUIT 15
#define IDR_RANDOM 16
#define IDR_KETNAME 17

#define IDR_FAST 25
#define IDR_MOUSE 26
#define IDR_KEYNAME_DEFAULT 27
#define IDR_PIANO 28
#define IDR_YASUO 29
#define IDR_ZHIXIA 30
#define IDR_MAJIANG 31
#define IDR_FENGMINGQI 32
#define IDR_MUYU 33
#define IDR_YUDI 34
#define IDR_QIFENGLE 35
#define IDR_CEDAR 36
#define IDR_YIN 37

LPCTSTR szAppClassName = TEXT("keysound");
LPCTSTR szAppWindowName = TEXT("keysound");
HMENU hmenu;//菜单句柄
HMENU packMenu;
static const int modeMenuIds[] = {0, IDR_RANDOM, IDR_KETNAME, IDR_PIANO, IDR_QIFENGLE, IDR_YASUO, IDR_ZHIXIA, IDR_MAJIANG, IDR_FENGMINGQI, IDR_MUYU, IDR_YUDI, IDR_CEDAR, IDR_YIN};
static settings::Settings savedSettings;
static wstring settingsPath() { return runPathW() + L"\\keysound-settings.dat"; }
static void restoreSettings() {
    settings::load(settingsPath(), savedSettings);
    flag_fast = savedSettings.interrupt;
    flag_mouse = savedSettings.mouse;
    flag = savedSettings.builtin_mode;
    if (savedSettings.use_pack) {
        for (size_t i = 0; i < availablePacks.size(); ++i) {
            if (_wcsicmp(availablePacks[i].directory.c_str(), savedSettings.pack_path.c_str()) == 0) {
                { lock_guard<mutex> lock(packMutex); activePack = availablePacks[i]; ++packGeneration; }
                flag = 13;
                CheckMenuItem(packMenu, IDR_PACK_BASE + (UINT)i, MF_BYCOMMAND | MF_CHECKED);
                packPlayer.prewarm();
                break;
            }
        }
    }
    for (int i = 1; i <= 12; ++i) CheckMenuItem(hmenu, modeMenuIds[i], MF_BYCOMMAND | (flag == i ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hmenu, IDR_FAST, MF_BYCOMMAND | (flag_fast ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hmenu, IDR_MOUSE, MF_BYCOMMAND | (flag_mouse ? MF_CHECKED : MF_UNCHECKED));
}
static void saveSettings(HWND hwnd) {
    int current = flag.load();
    savedSettings.interrupt = flag_fast.load() != 0;
    savedSettings.mouse = flag_mouse.load() != 0;
    savedSettings.use_pack = current == 13;
    if (savedSettings.use_pack) { lock_guard<mutex> lock(packMutex); savedSettings.pack_path = activePack.directory; }
    else if (current >= 1 && current <= 12) savedSettings.builtin_mode = current;
    if (settings::save(settingsPath(), savedSettings) != settings::SaveResult::Saved)
        MessageBoxW(hwnd, L"无法保存设置。请检查程序目录的写入权限及设置文件。", L"keysound", MB_OK | MB_ICONWARNING);
}
static bool trayAvailable = false;
static DWORD trayLastError = ERROR_SUCCESS;
static HWND trayStatusLabel = NULL;
static HWND audioStatusLabel = NULL;
static HWND elevationButton = NULL;
// Called only by an explicit button click. Never elevate automatically.
static bool restartElevated(HWND owner) {
    vector<wchar_t> executable(32768);
    DWORD length = GetModuleFileNameW(NULL, executable.data(), (DWORD)executable.size());
    if (!length || length >= executable.size()) return false;
    const wstring directory = runPathW();
    SHELLEXECUTEINFOW request{};
    request.cbSize = sizeof(request); request.fMask = SEE_MASK_NOCLOSEPROCESS;
    request.hwnd = owner; request.lpVerb = L"runas";
    request.lpFile = executable.data(); request.lpDirectory = directory.c_str();
    request.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&request)) {
        DWORD error = GetLastError();
        if (error != ERROR_CANCELLED) {
            reportWin32(L"ShellExecuteExW(runas)", error);
            MessageBoxW(owner, L"无法重新启动，当前程序会继续运行。", L"keysound", MB_OK | MB_ICONWARNING);
        }
        return false;
    }
    if (request.hProcess) CloseHandle(request.hProcess);
    return true;
}
static const UINT taskbarCreatedMessage = RegisterWindowMessageW(L"TaskbarCreated");
static bool addTrayIcon(HWND hwnd) {
    NOTIFYICONDATAW data{};
    data.cbSize = sizeof(data);
    data.hWnd = hwnd;
    data.uID = 0;
    data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    data.uCallbackMessage = WM_USER;
    data.hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_ICON1));
    if (!data.hIcon) data.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    lstrcpyW(data.szTip, L"keysound");
    SetLastError(ERROR_SUCCESS);
    trayAvailable = Shell_NotifyIconW(NIM_ADD, &data) != FALSE;
    // Shell_NotifyIcon does not promise extended error information; zero on
    // failure means no detail was supplied, not a successful registration.
    trayLastError = trayAvailable ? ERROR_SUCCESS : GetLastError();
    if (!trayAvailable) reportWin32(L"Shell_NotifyIconW(NIM_ADD) returned FALSE (error may be unavailable)", trayLastError);
    else OutputDebugStringW(L"keysound: Shell_NotifyIconW(NIM_ADD) returned TRUE\n");
    if (trayStatusLabel) SetWindowTextW(trayStatusLabel, restrictedIntegrity ?
        L"当前进程为低完整性：托盘/后台输入受限。\n请退出后从资源管理器正常启动（无需管理员）。" :
        trayAvailable ? L"托盘注册已接受；请在系统托盘检查图标。" : L"托盘注册失败；自动重试中，可使用下方菜单。");
    if (elevationButton) ShowWindow(elevationButton, trayAvailable ? SW_HIDE : SW_SHOW);
    if (trayAvailable) KillTimer(hwnd, 1); else SetTimer(hwnd, 1, 5000, NULL);
    return trayAvailable;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	NOTIFYICONDATA nid{};
	POINT pt;//用于接收鼠标坐标
	int xx;//用于接收菜单选项返回值

	// A top-level window (not HWND_MESSAGE) receives Explorer's restart broadcast.
	if (taskbarCreatedMessage && message == taskbarCreatedMessage) {
		trayAvailable = false;
		addTrayIcon(hwnd);
		return 0;
	}
	switch (message)
	{
	case WM_CREATE://窗口创建时候的消息.
		addTrayIcon(hwnd);
		hmenu = CreatePopupMenu();//生成菜单
		AppendMenu(hmenu, MF_UNCHECKED, IDR_RANDOM, L"随机模式");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_KETNAME, L"键名模式");
		AppendMenu(hmenu, MF_CHECKED, IDR_PIANO, L"钢琴模式");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_CEDAR, L"Cedar");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_YIN, L"音游模式");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_QIFENGLE, L"起风了");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_YASUO, L"疾风剑豪");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_ZHIXIA, L"知夏轴");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_MAJIANG, L"麻将音");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_FENGMINGQI, L"蜂鸣器");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_MUYU, L"木鱼音");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_YUDI, L"雨滴");

		discoverPacks();
		packMenu = CreatePopupMenu();
		for (size_t i=0; i<availablePacks.size(); ++i)
			AppendMenuW(packMenu, MF_STRING, IDR_PACK_BASE+(UINT)i, availablePacks[i].name.c_str());
		if (availablePacks.empty()) AppendMenuW(packMenu, MF_STRING | MF_GRAYED, 0, L"(empty)");
		AppendMenuW(hmenu, MF_POPUP, (UINT_PTR)packMenu, L"Sound packs");
		AppendMenu(hmenu, MF_UNCHECKED, IDR_FAST, L"音效打断");
		AppendMenu(hmenu, MF_CHECKED, IDR_MOUSE, L"鼠标音效");
		AppendMenu(hmenu, MF_STRING, IDR_OPENDIR, L"打开目录");
		//AppendMenu(hmenu, MF_STRING, IDR_AUTORUN, L"开机自启");
		AppendMenu(hmenu, MF_STRING, IDR_QUIT, L"退出");
		restoreSettings();
		{
			HWND label = trayStatusLabel = CreateWindowW(L"STATIC", restrictedIntegrity ?
                L"当前进程为低完整性：托盘/后台输入受限。\n请退出后从资源管理器正常启动（无需管理员）。" :
                trayAvailable ? L"托盘注册已接受；请在系统托盘检查图标。" : L"托盘注册失败；自动重试中，可使用下方菜单。",
				WS_CHILD | WS_VISIBLE, 16, 16, 400, 44, hwnd, NULL, GetModuleHandle(NULL), NULL);
			HWND button = CreateWindowW(L"BUTTON", L"打开音效菜单", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				16, 66, 150, 32, hwnd, (HMENU)100, GetModuleHandle(NULL), NULL);
			HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessageW(label, WM_SETFONT, (WPARAM)font, TRUE);
			SendMessageW(button, WM_SETFONT, (WPARAM)font, TRUE);
            elevationButton = CreateWindowW(L"BUTTON", L"以管理员身份重新启动",
                WS_CHILD | BS_PUSHBUTTON | (trayAvailable ? 0 : WS_VISIBLE),
                178, 66, 230, 32, hwnd, (HMENU)101, GetModuleHandleW(NULL), NULL);
            SendMessageW(elevationButton, WM_SETFONT, (WPARAM)font, TRUE);
            audioStatusLabel = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE,
                16, 106, 400, 44, hwnd, NULL, GetModuleHandleW(NULL), NULL);
            SendMessageW(audioStatusLabel, WM_SETFONT, (WPARAM)font, TRUE);
            SetTimer(hwnd, 2, 500, NULL);
		}
		break;
	case WM_TIMER:
        if (wParam == 2 && audioStatusLabel) {
            wchar_t status[256]{};
            swprintf_s(status, L"%s  PCM: %.2f MiB / %zu 文件\n提交: %llu  队列: %zu  HRESULT: 0x%08lX",
                flag.load() != 13 ? L"内建模式（旧播放器）" : packPlayer.loading.load() ? L"正在预解码" : packPlayer.readyState.load() ? L"内存音效就绪" : L"内存音效未就绪",
                packPlayer.cachedBytes.load() / 1048576.0, packPlayer.cachedCount.load(),
                packPlayer.playCount.load(), packPlayer.queuedCount.load(), (unsigned long)packPlayer.lastError.load());
            SetWindowTextW(audioStatusLabel, status);
        }
		if (wParam == 1 && !trayAvailable) addTrayIcon(hwnd);
		break;
    case WM_CLOSE:
        // The close button is not the application's Exit command.
        ShowWindow(hwnd, trayAvailable ? SW_HIDE : SW_MINIMIZE);
        return 0;
	case WM_COMMAND:
        if (LOWORD(wParam) == 101 && !trayAvailable) {
            if (restartElevated(hwnd)) DestroyWindow(hwnd);
            return 0;
        }
		if (LOWORD(wParam) == 100) SendMessage(hwnd, WM_USER, 0, WM_RBUTTONDOWN);
		break;
	case WM_USER://连续使用该程序时候的消息.
        if (lParam == WM_LBUTTONDBLCLK) {
            ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
            return 0;
        }
		if (lParam == WM_RBUTTONDOWN)
		{
			GetCursorPos(&pt);//取鼠标坐标
			::SetForegroundWindow(hwnd);//解决在菜单外单击左键菜单不消失的问题
			// EnableMenuItem(hmenu, IDR_PAUSE, MF_GRAYED);//让菜单中的某一项变灰
			xx = TrackPopupMenu(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, NULL, hwnd, NULL);//显示菜单并获取选项ID
			PostMessageW(hwnd, WM_NULL, 0, 0); // Complete notification-area menu dismissal.
			if (xx >= IDR_PACK_BASE && xx < IDR_PACK_BASE + (int)availablePacks.size()) {
				{ lock_guard<mutex> lock(packMutex); activePack = availablePacks[xx-IDR_PACK_BASE]; ++packGeneration; }
				flag = 13;
				packPlayer.prewarm();
				const int modes[] = {IDR_RANDOM, IDR_KETNAME, IDR_PIANO, IDR_CEDAR, IDR_YIN, IDR_QIFENGLE, IDR_YASUO, IDR_ZHIXIA, IDR_MAJIANG, IDR_FENGMINGQI, IDR_MUYU, IDR_YUDI};
				for (int id : modes) CheckMenuItem(hmenu, id, MF_BYCOMMAND | MF_UNCHECKED);
				for (size_t i=0; i<availablePacks.size(); ++i) CheckMenuItem(packMenu, IDR_PACK_BASE+(UINT)i, MF_BYCOMMAND | (IDR_PACK_BASE+(int)i==xx ? MF_CHECKED : MF_UNCHECKED));
			} else if (xx == IDR_RANDOM || xx == IDR_KETNAME || xx == IDR_PIANO || xx == IDR_CEDAR || xx == IDR_YIN || xx == IDR_QIFENGLE || xx == IDR_YASUO || xx == IDR_ZHIXIA || xx == IDR_MAJIANG || xx == IDR_FENGMINGQI || xx == IDR_MUYU || xx == IDR_YUDI) {
				for (size_t i=0; i<availablePacks.size(); ++i) CheckMenuItem(packMenu, IDR_PACK_BASE+(UINT)i, MF_BYCOMMAND | MF_UNCHECKED);
			}
			if (xx == IDR_QUIT) {
				DestroyWindow(hwnd); // WM_DESTROY owns tray/menu cleanup.
				return 0;
			}
			if (xx == IDR_RANDOM) {
				// 选中随机模式 取消键名模式
				flag = 1;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);

			}
			if (xx == IDR_KETNAME) {
				// 选中键名模式 取消随机模式
				flag = 2;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);
			}
			if (xx == IDR_PIANO) {
				flag = 3;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);
			}
			if (xx == IDR_QIFENGLE) {
				flag = 4;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);
			}
			if (xx == IDR_YASUO) {
				flag = 5;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);
			}
			if (xx == IDR_ZHIXIA) {
				flag = 6;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);
			}
			if (xx == IDR_MAJIANG) {
				flag = 7;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);
			}
			if (xx == IDR_FENGMINGQI) {
				flag = 8;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);
			}
			if (xx == IDR_MUYU) {
				flag = 9;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);
			}
			if (xx == IDR_YUDI) {
				flag = 10;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_CHECKED);
			}
			if (xx == IDR_CEDAR) {
				flag = 11;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);
			}
			if (xx == IDR_YIN) {
				flag = 12;
				CheckMenuItem(hmenu, IDR_RANDOM, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_KETNAME, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_PIANO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_CEDAR, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YIN, MF_CHECKED);
				CheckMenuItem(hmenu, IDR_QIFENGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YASUO, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_ZHIXIA, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MAJIANG, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_FENGMINGQI, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_MUYU, MF_UNCHECKED);
				CheckMenuItem(hmenu, IDR_YUDI, MF_UNCHECKED);
			}
			if (xx == IDR_FAST) {
				// 音效打断
				flag_fast = !flag_fast;
				if (flag_fast) {
					CheckMenuItem(hmenu, IDR_FAST, MF_CHECKED);
				}
				else {
					CheckMenuItem(hmenu, IDR_FAST, MF_UNCHECKED);
				}

			}
			if (xx == IDR_MOUSE) {
				// 音效打断
				flag_mouse = !flag_mouse;
				if (flag_mouse) {
					CheckMenuItem(hmenu, IDR_MOUSE, MF_CHECKED);
				}
				else {
					CheckMenuItem(hmenu, IDR_MOUSE, MF_UNCHECKED);
				}

			}
			if (xx == IDR_OPENDIR) {
				// 打开目录
				init_res(1);
			}
			bool modeChanged = xx >= IDR_PACK_BASE && xx < IDR_PACK_BASE + (int)availablePacks.size();
			for (int i = 1; i <= 12; ++i) if (xx == modeMenuIds[i]) modeChanged = true;
			if (modeChanged && flag.load() != 13) packPlayer.prewarm();
			if (modeChanged || xx == IDR_FAST || xx == IDR_MOUSE) saveSettings(hwnd);
		}
		break;
	case WM_DESTROY://窗口销毁时候的消息.
		KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
        audioStatusLabel = NULL;
        elevationButton = NULL;
		trayStatusLabel = NULL;
		trayAvailable = false;
		if (hmenu) { DestroyMenu(hmenu); hmenu = NULL; packMenu = NULL; }
		nid.cbSize = sizeof(nid);
		nid.hWnd = hwnd;
		nid.uID = 0;
		Shell_NotifyIcon(NIM_DELETE, &nid);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hwnd, message, wParam, lParam);
	}
	// Handled messages (especially WM_CREATE) must report successful handling.
	return 0;
}



// 设置右下角图标
int SetIco()
{
	cout << "SetIco start" << endl;
	HWND hwnd;
	MSG msg{};
	WNDCLASSW wndclass{};

	HWND handle = FindWindow(NULL, szAppWindowName);
	// if (handle != NULL)
	// {
	//     MessageBox(NULL, TEXT("Application is already running"), szAppClassName, MB_ICONERROR);
	//     return 0;
	// }

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	cout << "WndProc start" << endl;
	wndclass.lpfnWndProc = WndProc;
	cout << "WndProc end" << endl;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandleW(NULL);
	wndclass.hIcon = LoadIcon(NULL, (LPCTSTR)(32512));//IDI_APPLICATION
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppClassName;

	if (!RegisterClassW(&wndclass))
	{
		DWORD error = GetLastError();
		reportWin32(L"RegisterClassW(tray window)", error);
		MessageBoxW(NULL, L"无法注册托盘窗口类，请检查诊断错误。", szAppClassName, MB_ICONERROR);
		return 1;
	}

	// 此处使用WS_EX_TOOLWINDOW 属性来隐藏显示在任务栏上的窗口程序按钮
	//hwnd = CreateWindowEx(WS_EX_TOOLWINDOW,
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppClassName, szAppWindowName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		440,
		210,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	if (!hwnd) {
		reportWin32(L"CreateWindowExW(tray window)", GetLastError());
		UnregisterClassW(szAppClassName, wndclass.hInstance);
		return 1;
	}
	if (!hooksInstalled.load()) {
		wchar_t text[256] = {};
		swprintf_s(text, L"全局输入钩子安装失败：键盘错误=%lu，鼠标错误=%lu。后台输入不可保证；请检查权限或安全软件。",
			keyboardHookError.load(), mouseHookError.load());
		MessageBoxW(hwnd, text, L"keysound 输入诊断", MB_OK | MB_ICONWARNING);
	}
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	cout << "消息 start" << endl;

	//消息循环
	BOOL result;
	while ((result = GetMessageW(&msg, NULL, 0, 0)) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	if (result == -1) reportWin32(L"GetMessageW(tray)", GetLastError());
	if (IsWindow(hwnd)) DestroyWindow(hwnd);
	UnregisterClassW(szAppClassName, wndclass.hInstance);
	return result == -1 ? 1 : 0;
}



int main()
{
	// 初始化函数 创建sounds文件夹 并写入基础音效 后续用户可以自定义修改
	cout << '1' << endl;
	init_res(0);
	cout << '2' << endl;
	// Start the playback worker before installing the hook; join both on exit.
	packPlayer.start();
	legacyInputWorker.start();
	keyboardReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!keyboardReady) { legacyInputWorker.shutdown(); packPlayer.shutdown(); return 1; }
	thread t2(key_hook);
	WaitForSingleObject(keyboardReady, INFINITE);
	CloseHandle(keyboardReady);
	keyboardReady = NULL;
	cout << 3 << endl;

	//thread t3(key_hook2);
	cout << 4 << endl;
	// 线程分离
	//t3.detach();
	cout << 5 << endl;
	SetIco();
	PostThreadMessage(keyboardThreadId, WM_QUIT, 0, 0);
	t2.join();
	legacyInputWorker.shutdown();
	packPlayer.shutdown();

	cout << 6 << endl;
	return 0;
}
