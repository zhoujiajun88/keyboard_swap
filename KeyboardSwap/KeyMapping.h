#pragma once

#include "Common.h"

class KeyMapping {
public:
    const std::vector<Mapping>& Items() const { return mappings_; }
    std::vector<Mapping>& Items() { return mappings_; }

    const Mapping* Find(const std::wstring& source) const;
    Mapping* Find(const std::wstring& source);
    void Set(const std::wstring& source, const std::wstring& target);
    void Remove(const std::wstring& source);
    void Clear();

private:
    std::vector<Mapping> mappings_;
};
