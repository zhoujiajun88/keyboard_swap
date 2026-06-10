#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>

#include <algorithm>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

constexpr int kWindowWidth = 760;
constexpr int kWindowHeight = 594;
constexpr int kOriginalKeyboardWidth = 1792;
constexpr int kOriginalKeyboardHeight = 680;
constexpr ULONG_PTR kInjectedExtraInfo = 0x4b53574150494e46ULL;
constexpr wchar_t kRunValueName[] = L"KeyboardSwap";

enum class EditState {
    Idle,
    SelectingTarget,
    EditingExisting
};

struct KeyDef {
    std::wstring id;
    std::wstring zhName;
    std::wstring enName;
    WORD vk;
    WORD scan;
    bool extended;
    Gdiplus::RectF rect;
};

struct Mapping {
    std::wstring source;
    std::wstring target;
};

inline int ScaleForDpi(int value, UINT dpi) {
    return MulDiv(value, static_cast<int>(dpi), 96);
}

inline std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring out(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), n);
    return out;
}

inline std::string WideToUtf8(const std::wstring& s) {
    if (s.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), n, nullptr, nullptr);
    return out;
}

inline std::wstring ExePath() {
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return path;
}

inline std::wstring ExeDir() {
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    PathRemoveFileSpecW(path);
    return path;
}

inline std::wstring ConfigPath() {
    return ExeDir() + L"\\keymap.json";
}

inline WORD Scan(WORD vk, bool extended = false) {
    return static_cast<WORD>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC_EX) & 0xff);
}

inline std::unique_ptr<Gdiplus::Bitmap> LoadPngResource(HINSTANCE instance, int id) {
    HRSRC resource = FindResourceW(instance, MAKEINTRESOURCEW(id), RT_RCDATA);
    if (!resource) return {};
    DWORD size = SizeofResource(instance, resource);
    HGLOBAL loaded = LoadResource(instance, resource);
    void* bytes = LockResource(loaded);
    HGLOBAL copy = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!copy) return {};
    void* dest = GlobalLock(copy);
    CopyMemory(dest, bytes, size);
    GlobalUnlock(copy);

    IStream* stream = nullptr;
    if (CreateStreamOnHGlobal(copy, TRUE, &stream) != S_OK) {
        GlobalFree(copy);
        return {};
    }
    auto image = std::make_unique<Gdiplus::Bitmap>(stream);
    stream->Release();
    if (image->GetLastStatus() != Gdiplus::Ok) return {};
    return image;
}
