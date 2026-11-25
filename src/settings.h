#pragma once
#include <windows.h>
#include <charconv>
#include <string>
#include <system_error>
#include <vector>

namespace settings {
inline constexpr int volume_step = 10;
inline constexpr int max_volume = 100;

struct Settings {
    bool use_pack = false;
    std::wstring pack_path;
    bool interrupt = false;
    bool mouse = true;
    int language = 0;
    int volume = max_volume;
};
enum class LoadResult { Loaded, Missing, Invalid, IoError };
enum class SaveResult { Saved, Invalid, Protected, IoError };

namespace detail {
inline bool valid(const Settings& s) {
    return s.language >= 0 && s.language <= 2 &&
        s.volume >= 0 && s.volume <= max_volume && s.volume % volume_step == 0 &&
        (!s.use_pack || !s.pack_path.empty()) &&
        s.pack_path.size() <= 16384 && s.pack_path.find(L'\0') == std::wstring::npos;
}
inline std::string encode(const std::wstring& s) {
    const char* hex = "0123456789ABCDEF"; std::string out;
    for (wchar_t c : s) { unsigned v = static_cast<unsigned short>(c); for (int n=12; n>=0; n-=4) out += hex[(v >> n) & 15]; }
    return out;
}
inline bool decode(const std::string& s, std::wstring& out) {
    if (s.size()%4 || s.size()>65536) return false;
    for (size_t i=0;i<s.size();i+=4) {
        unsigned v=0;
        for (size_t j=0;j<4;++j) { char c=s[i+j]; int d=c>='0'&&c<='9'?c-'0':c>='A'&&c<='F'?c-'A'+10:-1; if(d<0)return false; v=(v<<4)|d; }
        if (!v) return false;
        if (v>=0xd800 && v<=0xdbff) {
            if (i+8>s.size()) return false;
            unsigned lo=0; for(size_t j=4;j<8;++j) {char c=s[i+j]; int d=c>='0'&&c<='9'?c-'0':c>='A'&&c<='F'?c-'A'+10:-1; if(d<0)return false; lo=(lo<<4)|d;}
            if(lo<0xdc00 || lo>0xdfff) return false;
            out += static_cast<wchar_t>(v); out += static_cast<wchar_t>(lo); i+=4;
        } else { if(v>=0xdc00 && v<=0xdfff) return false; out+=static_cast<wchar_t>(v); }
    }
    return true;
}
inline std::string serialize(const Settings& s) {
    return std::string("keysound-settings:3\nmode=1\npack=") + (s.use_pack ? "1" : "0") +
        "\npath=" + encode(s.pack_path) + "\ninterrupt=" + (s.interrupt ? "1" : "0") + "\nmouse=" + (s.mouse ? "1" : "0") +
        "\nlang=" + std::to_string(s.language) + "\nvolume=" + std::to_string(s.volume) + "\n";
}
inline bool parse(const std::string& text, Settings& s) {
    std::vector<std::string> lines; size_t p=0;
    while(p<text.size()) {size_t e=text.find('\n',p); if(e==std::string::npos)return false; lines.push_back(text.substr(p,e-p)); p=e+1;}
    const bool v1 = lines.size()==6 && lines[0]=="keysound-settings:1";
    const bool v2 = lines.size()==7 && lines[0]=="keysound-settings:2";
    const bool v3 = lines.size()==8 && lines[0]=="keysound-settings:3";
    if(!v1 && !v2 && !v3) return false;
    if(lines[1].compare(0,5,"mode=") || lines[2].compare(0,5,"pack=") || lines[3].compare(0,5,"path=") ||
       lines[4].compare(0,10,"interrupt=") || lines[5].compare(0,6,"mouse=")) return false;
    std::string mode=lines[1].substr(5), pack=lines[2].substr(5), fast=lines[4].substr(10), mouse=lines[5].substr(6);
    if(mode.empty() || mode.size()>2 || mode[0]<'1' || mode[0]>'9' || (mode.size()==2 && (mode[1]<'0'||mode[1]>'9')))return false;
    if((pack!="0"&&pack!="1")||(fast!="0"&&fast!="1")||(mouse!="0"&&mouse!="1"))return false;
    s.use_pack=pack=="1"; s.interrupt=fast=="1"; s.mouse=mouse=="1"; s.language=0;
    if(v2 || v3) {
        if(lines[6].compare(0,5,"lang=")) return false;
        const std::string lang=lines[6].substr(5);
        if(lang.size()!=1 || lang[0]<'0' || lang[0]>'2') return false;
        s.language=lang[0]-'0';
    }
    s.volume = max_volume;
    if(v3) {
        const std::string& field = lines[7];
        if(field.compare(0, 7, "volume=")) return false;
        const char* end = field.data() + field.size();
        const auto result = std::from_chars(field.data() + 7, end, s.volume);
        if(result.ec != std::errc{} || result.ptr != end) return false;
    }
    return decode(lines[3].substr(5),s.pack_path) && valid(s);
}
inline LoadResult read(const std::wstring& path, Settings& out) {
    HANDLE h=CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(h==INVALID_HANDLE_VALUE) { DWORD e=GetLastError(); return e==ERROR_FILE_NOT_FOUND||e==ERROR_PATH_NOT_FOUND?LoadResult::Missing:LoadResult::IoError; }
    LARGE_INTEGER size; if(!GetFileSizeEx(h,&size)) {CloseHandle(h); return LoadResult::IoError;}
    if(size.QuadPart<0 || size.QuadPart>70000) {CloseHandle(h); return LoadResult::Invalid;}
    std::string text(static_cast<size_t>(size.QuadPart),'\0'); DWORD got=0;
    bool ok=text.empty() || (ReadFile(h,&text[0],static_cast<DWORD>(text.size()),&got,NULL) && got==text.size());
    CloseHandle(h); if(!ok)return LoadResult::IoError;
    Settings parsed; if(!parse(text,parsed))return LoadResult::Invalid; out=parsed; return LoadResult::Loaded;
}
}
inline LoadResult load(const std::wstring& path, Settings& out) {
    out=Settings(); if(path.empty() || path.find(L'\0')!=std::wstring::npos)return LoadResult::IoError;
    return detail::read(path,out);
}
inline SaveResult save(const std::wstring& path, const Settings& value) {
    if(path.empty() || path.find(L'\0')!=std::wstring::npos || !detail::valid(value))return SaveResult::Invalid;
    std::wstring decoded; if(!detail::decode(detail::encode(value.pack_path), decoded))return SaveResult::Invalid;
    Settings previous; LoadResult old=detail::read(path,previous);
    if(old==LoadResult::Invalid)return SaveResult::Protected;
    if(old==LoadResult::IoError)return SaveResult::IoError;
    std::wstring tmp; HANDLE h=INVALID_HANDLE_VALUE;
    for(unsigned i=0;i<100;++i) {
        tmp=path+L".tmp."+std::to_wstring(GetCurrentProcessId())+L"."+std::to_wstring(i);
        h=CreateFileW(tmp.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if(h!=INVALID_HANDLE_VALUE)break;
        if(GetLastError()!=ERROR_FILE_EXISTS && GetLastError()!=ERROR_ALREADY_EXISTS)return SaveResult::IoError;
    }
    if(h==INVALID_HANDLE_VALUE)return SaveResult::IoError;
    std::string data=detail::serialize(value); DWORD written=0;
    bool ok=WriteFile(h,data.data(),static_cast<DWORD>(data.size()),&written,NULL) && written==data.size() && FlushFileBuffers(h);
    if(!CloseHandle(h))ok=false;
    if(ok) {
        ok=old==LoadResult::Missing ? MoveFileExW(tmp.c_str(),path.c_str(),MOVEFILE_WRITE_THROUGH)!=0 :
            ReplaceFileW(path.c_str(),tmp.c_str(),NULL,0,NULL,NULL)!=0;
    }
    if(!ok) {DeleteFileW(tmp.c_str()); return SaveResult::IoError;}
    return SaveResult::Saved;
}
}
