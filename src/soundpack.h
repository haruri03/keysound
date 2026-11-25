#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iterator>
#include <cstdlib>
#include <cctype>
#include <cwctype>

namespace soundpack {
struct Json {
    enum Kind { Null, Bool, Number, String, Array, Object } kind = Null;
    std::string text;
    std::vector<Json> array;
    std::map<std::string, Json> object;
};
class Parser {
    const std::string &s; size_t p = 0; int depth = 0;
    void ws() { while (p < s.size() && (s[p]==' ' || s[p]=='\n' || s[p]=='\r' || s[p]=='\t')) ++p; }
    bool take(char c) { ws(); if (p<s.size() && s[p]==c) { ++p; return true; } return false; }
    static void utf8(std::string &out, unsigned u) {
        if (u < 0x80) out += char(u);
        else if (u < 0x800) { out += char(0xc0|(u>>6)); out += char(0x80|(u&63)); }
        else if (u < 0x10000) { out += char(0xe0|(u>>12)); out += char(0x80|((u>>6)&63)); out += char(0x80|(u&63)); }
        else { out += char(0xf0|(u>>18)); out += char(0x80|((u>>12)&63)); out += char(0x80|((u>>6)&63)); out += char(0x80|(u&63)); }
    }
    bool hex4(unsigned &n) { n=0; for (int i=0;i<4;i++) { if(p==s.size()) return false; char c=s[p++]; int v=(c>='0'&&c<='9')?c-'0':(c>='a'&&c<='f')?c-'a'+10:(c>='A'&&c<='F')?c-'A'+10:-1; if(v<0) return false; n=(n<<4)|v; } return true; }
    bool str(std::string &out) {
        if (!take('"')) return false;
        while(p<s.size()) { unsigned char c=s[p++]; if(c=='"') return true; if(c<32) return false;
            if(c!='\\') { out+=char(c); continue; } if(p==s.size()) return false;
            c=s[p++]; switch(c) {
            case '"': case '\\': case '/': out+=char(c); break;
            case 'b': out+='\b'; break; case 'f': out+='\f'; break; case 'n': out+='\n'; break; case 'r': out+='\r'; break; case 't': out+='\t'; break;
            case 'u': { unsigned u; if(!hex4(u)) return false; if(u>=0xd800&&u<=0xdbff) { if(p+2>s.size()||s[p]!='\\'||s[p+1]!='u') return false; p+=2; unsigned l; if(!hex4(l)||l<0xdc00||l>0xdfff) return false; u=0x10000+((u-0xd800)<<10)+(l-0xdc00); } else if(u>=0xdc00&&u<=0xdfff) return false; utf8(out,u); break; }
            default: return false; }
        } return false;
    }
    bool value(Json &v) {
        ws(); if(p==s.size() || depth>=64) return false; ++depth; bool ok=false; char c=s[p];
        if(c=='"') { v.kind=Json::String; ok=str(v.text); }
        else if(c=='{') { v.kind=Json::Object; ++p; if(take('}')) ok=true; else { do { std::string k; Json x; if(!str(k)||!take(':')||!value(x)) break; v.object[k]=x; if(take('}')) { ok=true; break; } } while(take(',')); } }
        else if(c=='[') { v.kind=Json::Array; ++p; if(take(']')) ok=true; else { do { Json x; if(!value(x)) break; v.array.push_back(x); if(take(']')) { ok=true; break; } } while(take(',')); } }
        else if(s.compare(p,4,"null")==0) {p+=4; v.kind=Json::Null; ok=true;}
        else if(s.compare(p,4,"true")==0) {p+=4; v.kind=Json::Bool; ok=true;}
        else if(s.compare(p,5,"false")==0) {p+=5; v.kind=Json::Bool; ok=true;}
        else { size_t start=p; if(s[p]=='-') ++p; if(p<s.size()&&s[p]=='0') ++p; else { size_t digits=p; while(p<s.size()&&s[p]>='0'&&s[p]<='9') ++p; if(p==digits) { --depth; return false; } } if(p<s.size()&&s[p]=='.') { ++p; size_t digits=p; while(p<s.size()&&s[p]>='0'&&s[p]<='9') ++p; if(p==digits) {--depth; return false;} } if(p<s.size()&&(s[p]=='e'||s[p]=='E')) { ++p; if(p<s.size()&&(s[p]=='+'||s[p]=='-')) ++p; size_t digits=p; while(p<s.size()&&s[p]>='0'&&s[p]<='9') ++p; if(p==digits) {--depth; return false;} } v.kind=Json::Number; v.text=s.substr(start,p-start); ok=true; }
        --depth; return ok;
    }
public:
    explicit Parser(const std::string &input):s(input) { if(s.size()>=3 && (unsigned char)s[0]==0xef && (unsigned char)s[1]==0xbb && (unsigned char)s[2]==0xbf) p=3; }
    bool parse(Json &v) { if(!value(v)) return false; ws(); return p==s.size(); }
};
inline bool wide(const std::string &s, std::wstring &out) {
    if(s.empty()) {out.clear(); return true;} if(s.find('\0')!=std::string::npos) return false;
    int n=MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), NULL, 0); if(!n) return false;
    out.resize(n); return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), &out[0], n)==n;
}
inline const Json *field(const Json &j,const char *key,Json::Kind kind) { auto i=j.object.find(key); return i!=j.object.end()&&i->second.kind==kind?&i->second:nullptr; }
inline bool filename(const std::wstring &s) { return !s.empty() && s!=L"." && s!=L".." && s.find_first_of(L"\\/:\"\r\n") == std::wstring::npos; }
enum class Mode { Assign, Repeat, Random, Mixed, Unknown };
inline std::wstring folded(const std::wstring &s) {
    std::wstring out; out.reserve(s.size());
    for (wchar_t c : s) out.push_back(static_cast<wchar_t>(std::towlower(static_cast<wint_t>(c))));
    return out;
}
inline Mode modeKind(const std::wstring &mode) {
    if (mode == L"\u6307\u5b9a") return Mode::Assign;
    if (mode == L"\u91cd\u590d") return Mode::Repeat;
    if (mode == L"\u968f\u673a") return Mode::Random;
    if (mode == L"混合") return Mode::Mixed;
    const std::wstring lower = folded(mode);
    if (lower == L"assign") return Mode::Assign;
    if (lower == L"repeat") return Mode::Repeat;
    if (lower == L"random") return Mode::Random;
    if (lower == L"mixed") return Mode::Mixed;
    return Mode::Unknown;
}
inline std::wstring resourcePath(const std::wstring &directory, const std::wstring &name) {
    std::wstring legacy = directory + L"\\sounds\\" + name;
    DWORD attributes = GetFileAttributesW(legacy.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY) ? legacy : directory + L"\\" + name;
}
struct Pack { std::wstring name, directory; std::map<std::wstring,std::wstring> assigned; std::wstring mode, repeat; std::vector<std::wstring> sounds; };
inline bool load(const std::wstring &directory, Pack &result) {
    std::wstring configPath = directory + L"\\index.json";
    std::ifstream in(configPath.c_str(), std::ios::binary); if(!in) return false;
    std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()); if(bytes.size()>4*1024*1024) return false;
    std::wstring valid; if(!wide(bytes,valid)) return false;
    Json j; if(!Parser(bytes).parse(j)||j.kind!=Json::Object) return false;
    const Json *name=field(j,"name",Json::String), *mode=field(j,"mode",Json::String); if(!name||!mode) return false;
    Pack p; p.directory=directory; if(!wide(name->text,p.name)||p.name.empty()||!wide(mode->text,p.mode)) return false;
    const Json *repeat=field(j,"repeat_sound",Json::String); if(repeat && (!wide(repeat->text,p.repeat)||(!p.repeat.empty()&&!filename(p.repeat)))) return false;
    const Json *a=field(j,"assigned_sounds",Json::Array); if(a) for(const Json &entry:a->array) { if(entry.kind!=Json::Object) return false; const Json *k=field(entry,"key",Json::String), *v=field(entry,"sound",Json::String); std::wstring wk,wv; if(!k||!v||!wide(k->text,wk)||!wide(v->text,wv)||wk.empty()||!filename(wv)) return false; p.assigned[wk]=resourcePath(directory,wv); }
    const Json *list=field(j,"soundsList",Json::Array); if(list) for(const Json &v:list->array) { std::wstring w; if(v.kind!=Json::String||!wide(v.text,w)||!filename(w)) return false; p.sounds.push_back(resourcePath(directory,w)); }
    result=p; return true;
}
}
