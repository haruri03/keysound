#include <windows.h>

#include <cassert>
#include <cstdio>
#include <string>

#include "app/strings.h"
#include "settings.h"

int main()
{
    const bool chinese = i18n::systemUsesChinese();

    assert(i18n::resolve(i18n::FollowSystem) == (chinese ? i18n::Zh : i18n::En));
    assert(i18n::resolve(i18n::Chinese) == i18n::Zh);
    assert(i18n::resolve(i18n::English) == i18n::En);
    assert(i18n::resolve(-1) == (chinese ? i18n::Zh : i18n::En));
    assert(i18n::resolve(7) == (chinese ? i18n::Zh : i18n::En));

    assert(std::wstring(i18n::pick(i18n::Zh, L"\u4e2d\u6587", L"English")) == L"\u4e2d\u6587");
    assert(std::wstring(i18n::pick(i18n::En, L"\u4e2d\u6587", L"English")) == L"English");

    settings::Settings value;
    value.language = 2;
    const std::string text = settings::detail::serialize(value);
    assert(text.compare(0, 19, "keysound-settings:2") == 0);
    assert(text.find("\nlang=2\n") != std::string::npos);

    settings::Settings parsed;
    assert(settings::detail::parse(text, parsed));
    assert(parsed.language == 2);

    value.language = 0;
    const std::string follow = settings::detail::serialize(value);
    settings::Settings reread;
    assert(settings::detail::parse(follow, reread));
    assert(reread.language == 0);

    value.language = 3;
    assert(!settings::detail::valid(value));

    printf("i18n tests passed\n");
    return 0;
}
