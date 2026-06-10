#pragma once

#include "Common.h"

class KeyboardLayout {
public:
    KeyboardLayout();

    const std::vector<KeyDef>& Keys() const { return keys_; }
    const KeyDef* FindKey(const std::wstring& id) const;
    const KeyDef* FindByHookEvent(DWORD vkCode, DWORD scanCode, bool extended) const;

private:
    std::vector<KeyDef> keys_;
};
