#pragma once

#include "Common.h"
#include "KeyboardLayout.h"
#include "KeyMapping.h"

class ConfigStore {
public:
    explicit ConfigStore(const KeyboardLayout& layout);

    void Load(KeyMapping& mapping, bool& enabled, std::wstring& languageCode) const;
    void Save(const KeyMapping& mapping, bool enabled, const std::wstring& languageCode) const;

private:
    const KeyboardLayout& layout_;
};
