#pragma once
#include <windows.h>

namespace i18n {

enum Choice { FollowSystem = 0, Chinese = 1, English = 2 };

enum Lang { Zh, En };

inline bool systemUsesChinese()
{
    return PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_CHINESE;
}

inline Lang resolve(int choice)
{
    if (choice == Chinese)
        return Zh;
    if (choice == English)
        return En;
    return systemUsesChinese() ? Zh : En;
}

inline const wchar_t* pick(Lang lang, const wchar_t* chinese, const wchar_t* english)
{
    return lang == Zh ? chinese : english;
}

}
