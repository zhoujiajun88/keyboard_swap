#pragma once

#include "Common.h"
#include "ConfigStore.h"
#include "I18n.h"
#include "KeyboardHook.h"
#include "KeyboardLayout.h"
#include "KeyMapping.h"
#include "TrayIcon.h"
#include "resource.h"

class MainWindow {
public:
    MainWindow(HINSTANCE instance, I18n& i18n, KeyboardLayout& layout, KeyMapping& mapping, ConfigStore& config, bool& enabled, std::wstring& languageCode);
    ~MainWindow();

    bool Create(bool startInTray);
    HWND Hwnd() const { return hwnd_; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp);
    void Paint();
    void DrawContent(Gdiplus::Graphics& g, const RECT& paintRc);
    void DrawKeyboard(Gdiplus::Graphics& g);
    void DrawTable(Gdiplus::Graphics& g);
    void DrawText(Gdiplus::Graphics& g, const std::wstring& text, const Gdiplus::Font& font, const Gdiplus::RectF& rc, Gdiplus::Color color, Gdiplus::StringAlignment align = Gdiplus::StringAlignmentCenter);
    void OnClick(int x, int y);
    void OnKeyClick(const KeyDef& key);
    void BeginCreate(const std::wstring& source);
    void BeginEdit(const std::wstring& source);
    void CompleteMapping(const std::wstring& target);
    void RemoveMapping(const std::wstring& source);
    void CancelEdit();
    void FinishFeedback();
    void ScrollTable(int wheelDelta);
    void ClampTableScroll();
    void ChangeLanguageAndRestart(const std::wstring& languageCode);
    void FlashTestKey(const KeyDef& key);
    void UpdateTestKeyFade();
    void RecomputeLayout();
    void ResizeForDpi(UINT dpi, const RECT* suggested = nullptr);
    void InvalidateAll();
    void InvalidateBlinkArea();
    Gdiplus::RectF KeyRect(const KeyDef& key) const;
    RECT KeyRectPixels(const KeyDef& key) const;
    const KeyDef* HitKey(float x, float y) const;

    int Dpi(int value) const { return ScaleForDpi(value, dpi_); }
    float DpiF(float value) const { return value * static_cast<float>(dpi_) / 96.0f; }
    std::wstring S(const wchar_t* zh, const wchar_t* en) const { return i18n_.T(zh, en); }

    HINSTANCE instance_ = nullptr;
    HWND hwnd_ = nullptr;
    I18n& i18n_;
    KeyboardLayout& layout_;
    KeyMapping& mapping_;
    ConfigStore& config_;
    bool& enabled_;
    std::wstring& languageCode_;
    std::unique_ptr<TrayIcon> tray_;
    std::unique_ptr<KeyboardHook> hook_;
    std::unique_ptr<Gdiplus::Bitmap> keyboardImage_;
    std::unique_ptr<Gdiplus::Bitmap> badgeImage_;
    UINT dpi_ = 96;
    RECT keyboardBox_{};
    RECT tableBox_{};
    float keyboardScale_ = 1.0f;
    bool blinkOn_ = true;
    int tableScrollRows_ = 0;
    struct TestKeyFlash {
        std::wstring keyId;
        DWORD startedAt = 0;
    };
    std::vector<TestKeyFlash> testKeyFlashes_;
    EditState state_ = EditState::Idle;
    std::wstring pendingSource_;
    std::wstring feedbackTarget_;
};
