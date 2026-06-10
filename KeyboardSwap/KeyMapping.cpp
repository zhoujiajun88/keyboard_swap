#include "KeyMapping.h"

const Mapping* KeyMapping::Find(const std::wstring& source) const {
    auto it = std::find_if(mappings_.begin(), mappings_.end(), [&](const Mapping& item) { return item.source == source; });
    return it == mappings_.end() ? nullptr : &*it;
}

Mapping* KeyMapping::Find(const std::wstring& source) {
    auto it = std::find_if(mappings_.begin(), mappings_.end(), [&](const Mapping& item) { return item.source == source; });
    return it == mappings_.end() ? nullptr : &*it;
}

void KeyMapping::Set(const std::wstring& source, const std::wstring& target) {
    if (Mapping* mapping = Find(source)) {
        mapping->target = target;
    } else {
        mappings_.push_back({source, target});
    }
}

void KeyMapping::Remove(const std::wstring& source) {
    mappings_.erase(std::remove_if(mappings_.begin(), mappings_.end(), [&](const Mapping& item) { return item.source == source; }), mappings_.end());
}

void KeyMapping::Clear() {
    mappings_.clear();
}
