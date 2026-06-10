#include "I18n.h"

I18n::I18n() {
    SetLanguage(L"");
}

void I18n::SetLanguage(const std::wstring& languageCode) {
    languageCode_ = languageCode;
    traditionalChinese_ = false;
    if (languageCode == L"zh-CN") {
        chinese_ = true;
        return;
    }
    if (languageCode == L"zh-TW") {
        chinese_ = true;
        traditionalChinese_ = true;
        return;
    }
    if (languageCode == L"en") {
        chinese_ = false;
        return;
    }
    chinese_ = PRIMARYLANGID(LANGIDFROMLCID(GetUserDefaultUILanguage())) == LANG_CHINESE;
}

std::wstring I18n::T(const wchar_t* zh, const wchar_t* en) const {
    return chinese_ ? ChineseText(zh) : en;
}

std::wstring I18n::ChineseText(const std::wstring& text) const {
    if (!traditionalChinese_ || text.empty()) return text;

    int size = LCMapStringEx(LOCALE_NAME_SYSTEM_DEFAULT, LCMAP_TRADITIONAL_CHINESE, text.c_str(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr, 0);
    if (size <= 0) return text;
    std::wstring converted(size, L'\0');
    int written = LCMapStringEx(LOCALE_NAME_SYSTEM_DEFAULT, LCMAP_TRADITIONAL_CHINESE, text.c_str(), static_cast<int>(text.size()), converted.data(), size, nullptr, nullptr, 0);
    if (written <= 0) return text;
    converted.resize(written);
    return converted;
}
