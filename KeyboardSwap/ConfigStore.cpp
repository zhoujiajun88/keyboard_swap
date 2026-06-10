#include "ConfigStore.h"

namespace {

std::wstring ReadJsonString(const std::wstring& text, const std::wstring& name) {
    size_t pos = text.find(L"\"" + name + L"\"");
    if (pos == std::wstring::npos) return L"";
    size_t colon = text.find(L':', pos);
    if (colon == std::wstring::npos) return L"";
    size_t q1 = text.find(L'"', colon + 1);
    size_t q2 = text.find(L'"', q1 + 1);
    if (q1 == std::wstring::npos || q2 == std::wstring::npos) return L"";
    return text.substr(q1 + 1, q2 - q1 - 1);
}

}

ConfigStore::ConfigStore(const KeyboardLayout& layout) : layout_(layout) {}

void ConfigStore::Load(KeyMapping& mapping, bool& enabled, std::wstring& languageCode) const {
    mapping.Clear();
    languageCode.clear();

    std::ifstream in(ConfigPath(), std::ios::binary);
    if (!in) return;

    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::wstring w = Utf8ToWide(text);
    enabled = w.find(L"\"enabled\": false") == std::wstring::npos;
    languageCode = ReadJsonString(w, L"language");

    size_t pos = 0;
    while ((pos = w.find(L"\"source\"", pos)) != std::wstring::npos) {
        size_t q1 = w.find(L'"', w.find(L':', pos) + 1);
        size_t q2 = w.find(L'"', q1 + 1);
        size_t target = w.find(L"\"target\"", q2);
        if (q1 == std::wstring::npos || q2 == std::wstring::npos || target == std::wstring::npos) break;
        size_t tq1 = w.find(L'"', w.find(L':', target) + 1);
        size_t tq2 = w.find(L'"', tq1 + 1);
        if (tq1 == std::wstring::npos || tq2 == std::wstring::npos) break;

        std::wstring source = w.substr(q1 + 1, q2 - q1 - 1);
        std::wstring destination = w.substr(tq1 + 1, tq2 - tq1 - 1);
        if (layout_.FindKey(source) && layout_.FindKey(destination)) mapping.Set(source, destination);
        pos = tq2 + 1;
    }
}

void ConfigStore::Save(const KeyMapping& mapping, bool enabled, const std::wstring& languageCode) const {
    std::ostringstream json;
    json << "{\n  \"version\": \"1.1\",\n  \"enabled\": " << (enabled ? "true" : "false") << ",\n";
    if (!languageCode.empty()) {
        json << "  \"language\": \"" << WideToUtf8(languageCode) << "\",\n";
    }
    json << "  \"mappings\": [\n";

    const auto& items = mapping.Items();
    for (size_t i = 0; i < items.size(); ++i) {
        json << "    { \"source\": \"" << WideToUtf8(items[i].source) << "\", \"target\": \"" << WideToUtf8(items[i].target) << "\" }";
        if (i + 1 < items.size()) json << ",";
        json << "\n";
    }
    json << "  ]\n}\n";

    std::wstring path = ConfigPath();
    std::wstring tmp = path + L".tmp";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        out << json.str();
    }
    MoveFileExW(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}
