#pragma once

#include "Common.h"

class I18n {
public:
    I18n();

    bool IsChinese() const { return chinese_; }
    bool IsTraditionalChinese() const { return traditionalChinese_; }
    std::wstring LanguageCode() const { return languageCode_; }
    void SetLanguage(const std::wstring& languageCode);
    std::wstring T(const wchar_t* zh, const wchar_t* en) const;
    std::wstring ChineseText(const std::wstring& text) const;

private:
    bool chinese_ = false;
    bool traditionalChinese_ = false;
    std::wstring languageCode_;
};
