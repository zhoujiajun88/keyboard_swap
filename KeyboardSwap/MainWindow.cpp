#include "MainWindow.h"

#include <windowsx.h>

namespace {

constexpr wchar_t kClassName[] = L"KeyboardSwapWindow";
constexpr float kHintX = 1424.0f;
constexpr float kHintY = 106.0f;
constexpr float kHintW = 278.0f;
constexpr float kHintH = 80.0f;

bool Intersects(const RECT& a, const RECT& b) {
    RECT out{};
    return IntersectRect(&out, &a, &b) != FALSE;
}

void Inflate(RECT& rc, int amount) {
    InflateRect(&rc, amount, amount);
}

}

MainWindow::MainWindow(HINSTANCE instance, I18n& i18n, KeyboardLayout& layout, KeyMapping& mapping, ConfigStore& config, bool& enabled, std::wstring& languageCode)
    : instance_(instance), i18n_(i18n), layout_(layout), mapping_(mapping), config_(config), enabled_(enabled), languageCode_(languageCode) {}

MainWindow::~MainWindow() = default;

bool MainWindow::Create(bool startInTray) {
    keyboardImage_ = LoadPngResource(instance_, IDR_KEYBOARD_PNG);
    badgeImage_ = LoadPngResource(instance_, IDR_BADGE_PNG);

    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance_;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconW(instance_, MAKEINTRESOURCEW(IDI_APP_ICON));
    wc.hIconSm = static_cast<HICON>(LoadImageW(instance_, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, Dpi(16), Dpi(16), LR_DEFAULTCOLOR));
    wc.hbrBackground = nullptr;
    RegisterClassExW(&wc);

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    dpi_ = GetDpiForSystem();
    hwnd_ = CreateWindowExW(0, kClassName, L"Keyboard Swap", style,
        CW_USEDEFAULT, CW_USEDEFAULT, Dpi(kWindowWidth), Dpi(kWindowHeight),
        nullptr, nullptr, instance_, this);
    if (!hwnd_) return false;

    dpi_ = GetDpiForWindow(hwnd_);
    RecomputeLayout();
    tray_ = std::make_unique<TrayIcon>(hwnd_, i18n_, enabled_);
    hook_ = std::make_unique<KeyboardHook>(instance_, hwnd_, layout_, mapping_, enabled_);
    hook_->Install();
    tray_->Add();

    if (startInTray) {
        ShowWindow(hwnd_, SW_HIDE);
    } else {
        ShowWindow(hwnd_, SW_SHOW);
        UpdateWindow(hwnd_);
    }
    return true;
}

Gdiplus::RectF MainWindow::KeyRect(const KeyDef& key) const {
    return Gdiplus::RectF(
        static_cast<float>(keyboardBox_.left) + key.rect.X * keyboardScale_,
        static_cast<float>(keyboardBox_.top) + key.rect.Y * keyboardScale_,
        key.rect.Width * keyboardScale_,
        key.rect.Height * keyboardScale_);
}

RECT MainWindow::KeyRectPixels(const KeyDef& key) const {
    Gdiplus::RectF r = KeyRect(key);
    RECT rc{
        static_cast<LONG>(r.X) - 2,
        static_cast<LONG>(r.Y) - 2,
        static_cast<LONG>(r.X + r.Width) + 3,
        static_cast<LONG>(r.Y + r.Height) + 3
    };
    return rc;
}

const KeyDef* MainWindow::HitKey(float x, float y) const {
    for (const auto& key : layout_.Keys()) {
        Gdiplus::RectF r = KeyRect(key);
        if (x >= r.X && x <= r.X + r.Width && y >= r.Y && y <= r.Y + r.Height) return &key;
    }
    return nullptr;
}

void MainWindow::RecomputeLayout() {
    RECT client{};
    GetClientRect(hwnd_, &client);
    int margin = Dpi(10);
    int keyboardW = std::min(static_cast<int>(client.right - client.left) - margin * 2, Dpi(740));
    int keyboardH = MulDiv(keyboardW, kOriginalKeyboardHeight, kOriginalKeyboardWidth);
    keyboardBox_ = {margin, margin, margin + keyboardW, margin + keyboardH};
    keyboardScale_ = static_cast<float>(keyboardW) / static_cast<float>(kOriginalKeyboardWidth);
    tableBox_ = {margin, keyboardBox_.bottom + Dpi(12), client.right - margin, client.bottom - margin};
}

void MainWindow::ResizeForDpi(UINT dpi, const RECT* suggested) {
    dpi_ = dpi;
    DWORD style = static_cast<DWORD>(GetWindowLongPtrW(hwnd_, GWL_STYLE));
    if (suggested) {
        SetWindowPos(hwnd_, nullptr, suggested->left, suggested->top, suggested->right - suggested->left, suggested->bottom - suggested->top, SWP_NOZORDER | SWP_NOACTIVATE);
    } else {
        SetWindowPos(hwnd_, nullptr, 0, 0, Dpi(kWindowWidth), Dpi(kWindowHeight), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
    RecomputeLayout();
    InvalidateAll();
}

void MainWindow::InvalidateAll() {
    InvalidateRect(hwnd_, nullptr, FALSE);
}

void MainWindow::InvalidateBlinkArea() {
    RECT dirty{};
    bool hasDirty = false;
    auto add = [&](const std::wstring& id) {
        const KeyDef* key = layout_.FindKey(id);
        if (!key) return;
        RECT keyRc = KeyRectPixels(*key);
        Inflate(keyRc, Dpi(2));
        if (!hasDirty) {
            dirty = keyRc;
            hasDirty = true;
        } else {
            UnionRect(&dirty, &dirty, &keyRc);
        }
    };

    add(pendingSource_);
    add(feedbackTarget_);
    if (const Mapping* existing = mapping_.Find(pendingSource_)) add(existing->target);

    RECT hint{
        keyboardBox_.left + static_cast<LONG>(kHintX * keyboardScale_),
        keyboardBox_.top + static_cast<LONG>(kHintY * keyboardScale_),
        keyboardBox_.left + static_cast<LONG>((kHintX + kHintW) * keyboardScale_),
        keyboardBox_.top + static_cast<LONG>((kHintY + kHintH) * keyboardScale_)
    };
    Inflate(hint, Dpi(2));
    if (!hasDirty) dirty = hint;
    else UnionRect(&dirty, &dirty, &hint);

    InvalidateRect(hwnd_, &dirty, FALSE);
}

void MainWindow::DrawText(Gdiplus::Graphics& g, const std::wstring& text, const Gdiplus::Font& font, const Gdiplus::RectF& rc, Gdiplus::Color color, Gdiplus::StringAlignment align) {
    Gdiplus::SolidBrush brush(color);
    Gdiplus::StringFormat fmt;
    fmt.SetAlignment(align);
    fmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    fmt.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    g.DrawString(text.c_str(), -1, &font, rc, &fmt, &brush);
}

void MainWindow::Paint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd_, &ps);
    RECT client{};
    GetClientRect(hwnd_, &client);
    int width = client.right - client.left;
    int height = client.bottom - client.top;
    HDC memDc = CreateCompatibleDC(hdc);
    HBITMAP bitmap = CreateCompatibleBitmap(hdc, width, height);
    HGDIOBJ old = SelectObject(memDc, bitmap);
    Gdiplus::Graphics g(memDc);
    DrawContent(g, ps.rcPaint);
    BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top, memDc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
    SelectObject(memDc, old);
    DeleteObject(bitmap);
    DeleteDC(memDc);
    EndPaint(hwnd_, &ps);
}

void MainWindow::DrawContent(Gdiplus::Graphics& g, const RECT& paintRc) {
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    g.Clear(Gdiplus::Color(255, 246, 247, 249));
    if (Intersects(paintRc, keyboardBox_)) DrawKeyboard(g);
    if (Intersects(paintRc, tableBox_)) DrawTable(g);
}

void MainWindow::DrawKeyboard(Gdiplus::Graphics& g) {
    if (keyboardImage_) {
        g.DrawImage(keyboardImage_.get(), keyboardBox_.left, keyboardBox_.top,
            keyboardBox_.right - keyboardBox_.left, keyboardBox_.bottom - keyboardBox_.top);
    }

    DWORD now = GetTickCount();
    for (const auto& flash : testKeyFlashes_) {
        const KeyDef* key = layout_.FindKey(flash.keyId);
        if (!key) continue;
        DWORD age = now - flash.startedAt;
        if (age >= 1000) continue;
        int alpha = static_cast<int>(125.0f * (1.0f - static_cast<float>(age) / 1000.0f));
        Gdiplus::RectF r = KeyRect(*key);
        Gdiplus::SolidBrush brush(Gdiplus::Color(alpha, 86, 182, 255));
        g.FillRectangle(&brush, r);
    }

    for (const auto& key : layout_.Keys()) {
        bool red = state_ == EditState::SelectingTarget && key.id == pendingSource_;
        if (state_ == EditState::EditingExisting && key.id == pendingSource_) red = true;
        bool green = key.id == feedbackTarget_;
        const Mapping* existing = mapping_.Find(pendingSource_);
        if (state_ == EditState::EditingExisting && existing && key.id == existing->target) green = true;
        if (!blinkOn_) { red = false; green = false; }
        if (red || green) {
            Gdiplus::RectF r = KeyRect(key);
            Gdiplus::SolidBrush brush(red ? Gdiplus::Color(150, 220, 49, 49) : Gdiplus::Color(150, 28, 166, 94));
            g.FillRectangle(&brush, r);
        }
    }

    for (const auto& item : mapping_.Items()) {
        const KeyDef* key = layout_.FindKey(item.source);
        if (!key || !badgeImage_) continue;
        Gdiplus::RectF r = KeyRect(*key);
        float size = std::max(DpiF(10), 36.0f * keyboardScale_);
        g.DrawImage(badgeImage_.get(), r.X + r.Width - size, r.Y, size, size);
    }

    if (state_ != EditState::Idle) {
        Gdiplus::RectF hint(
            static_cast<float>(keyboardBox_.left) + kHintX * keyboardScale_,
            static_cast<float>(keyboardBox_.top) + kHintY * keyboardScale_,
            kHintW * keyboardScale_,
            kHintH * keyboardScale_);
        Gdiplus::SolidBrush bg(Gdiplus::Color(236, 255, 255, 255));
        Gdiplus::Pen pen(Gdiplus::Color(255, 70, 86, 105), DpiF(1.2f));
        g.FillRectangle(&bg, hint);
        g.DrawRectangle(&pen, hint);
        Gdiplus::FontFamily family(L"Segoe UI");
        Gdiplus::Font font(&family, std::max(DpiF(7), 18.0f * keyboardScale_), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        DrawText(g,
            state_ == EditState::EditingExisting ? S(L"再次单击编辑要映射的按键", L"Click another key to edit mapping") : S(L"再次单击选择要映射的按键", L"Click another key to map"),
            font, hint, Gdiplus::Color(255, 30, 41, 59));
    }
}

void MainWindow::DrawTable(Gdiplus::Graphics& g) {
    Gdiplus::FontFamily family(L"Segoe UI");
    Gdiplus::Font title(&family, DpiF(16), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::Font header(&family, DpiF(14), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::Font cell(&family, DpiF(14), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush white(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::SolidBrush line(Gdiplus::Color(255, 220, 226, 235));
    Gdiplus::Pen border(Gdiplus::Color(255, 203, 213, 225), DpiF(1));
    Gdiplus::Pen rowPen(Gdiplus::Color(255, 226, 232, 240), DpiF(1));

    Gdiplus::RectF box(static_cast<float>(tableBox_.left), static_cast<float>(tableBox_.top), static_cast<float>(tableBox_.right - tableBox_.left), static_cast<float>(tableBox_.bottom - tableBox_.top));
    g.FillRectangle(&white, box);
    g.DrawRectangle(&border, box);
    DrawText(g, S(L"键位映射", L"Key Mappings"), title, Gdiplus::RectF(box.X + DpiF(12), box.Y + DpiF(10), box.Width - DpiF(24), DpiF(24)), Gdiplus::Color(255, 15, 23, 42), Gdiplus::StringAlignmentNear);

    float y = box.Y + DpiF(48);
    float rowH = DpiF(34);
    const auto& items = mapping_.Items();
    float maxY = box.Y + box.Height - DpiF(4);
    int visibleRows = std::max(0, static_cast<int>((maxY - (y + rowH)) / rowH));
    bool needsScrollbar = static_cast<int>(items.size()) > visibleRows;
    float scrollbarGutter = needsScrollbar ? DpiF(18) : 0.0f;
    float contentRight = box.X + box.Width - scrollbarGutter;
    float contentWidth = contentRight - box.X;
    float c1 = box.X + DpiF(16), c2 = box.X + contentWidth * 0.38f, c3 = box.X + contentWidth * 0.66f;
    g.FillRectangle(&line, Gdiplus::RectF(box.X, y, contentWidth, rowH));
    DrawText(g, S(L"原按键", L"Source Key"), header, Gdiplus::RectF(c1, y, c2 - c1, rowH), Gdiplus::Color(255, 51, 65, 85), Gdiplus::StringAlignmentNear);
    DrawText(g, S(L"映射按键", L"Mapped Key"), header, Gdiplus::RectF(c2, y, c3 - c2, rowH), Gdiplus::Color(255, 51, 65, 85), Gdiplus::StringAlignmentNear);
    DrawText(g, S(L"操作", L"Actions"), header, Gdiplus::RectF(c3, y, contentRight - c3, rowH), Gdiplus::Color(255, 51, 65, 85), Gdiplus::StringAlignmentNear);
    y += rowH;

    int startRow = std::min(tableScrollRows_, static_cast<int>(items.size()));
    for (size_t i = static_cast<size_t>(startRow); i < items.size(); ++i) {
        const auto& item = items[i];
        if (y + rowH > maxY) break;
        const KeyDef* source = layout_.FindKey(item.source);
        const KeyDef* target = layout_.FindKey(item.target);
        g.DrawLine(&rowPen, box.X, y, contentRight, y);
        DrawText(g, source ? (i18n_.IsChinese() ? i18n_.ChineseText(source->zhName) : source->enName) : item.source, cell, Gdiplus::RectF(c1, y, c2 - c1, rowH), Gdiplus::Color(255, 15, 23, 42), Gdiplus::StringAlignmentNear);
        DrawText(g, target ? (i18n_.IsChinese() ? i18n_.ChineseText(target->zhName) : target->enName) : item.target, cell, Gdiplus::RectF(c2, y, c3 - c2, rowH), Gdiplus::Color(255, 15, 23, 42), Gdiplus::StringAlignmentNear);

        Gdiplus::RectF editRect(c3, y + DpiF(5), DpiF(68), DpiF(24));
        Gdiplus::RectF deleteRect(c3 + DpiF(78), y + DpiF(5), DpiF(76), DpiF(24));
        Gdiplus::SolidBrush editBg(Gdiplus::Color(255, 232, 242, 255));
        Gdiplus::SolidBrush delBg(Gdiplus::Color(255, 255, 237, 237));
        Gdiplus::Pen editPen(Gdiplus::Color(255, 96, 165, 250), DpiF(1));
        Gdiplus::Pen delPen(Gdiplus::Color(255, 248, 113, 113), DpiF(1));
        g.FillRectangle(&editBg, editRect); g.DrawRectangle(&editPen, editRect);
        g.FillRectangle(&delBg, deleteRect); g.DrawRectangle(&delPen, deleteRect);
        DrawText(g, S(L"编辑", L"Edit"), cell, editRect, Gdiplus::Color(255, 29, 78, 216));
        DrawText(g, S(L"删除", L"Delete"), cell, deleteRect, Gdiplus::Color(255, 185, 28, 28));
        y += rowH;
    }

    if (mapping_.Items().empty()) {
        DrawText(g, S(L"点击上方键盘创建映射", L"Click a key above to create a mapping"), cell,
            Gdiplus::RectF(box.X, y + DpiF(24), box.Width, DpiF(40)), Gdiplus::Color(255, 100, 116, 139));
    }

    if (needsScrollbar) {
        float trackW = DpiF(8);
        float trackX = box.X + box.Width - DpiF(13);
        float trackY = box.Y + DpiF(48) + rowH + DpiF(4);
        float trackH = box.Y + box.Height - DpiF(10) - trackY;
        int maxScroll = std::max(1, static_cast<int>(items.size()) - visibleRows);
        float thumbH = std::max(DpiF(24), trackH * static_cast<float>(visibleRows) / static_cast<float>(items.size()));
        float thumbY = trackY + (trackH - thumbH) * static_cast<float>(tableScrollRows_) / static_cast<float>(maxScroll);
        Gdiplus::SolidBrush track(Gdiplus::Color(255, 232, 238, 246));
        Gdiplus::SolidBrush thumb(Gdiplus::Color(255, 148, 163, 184));
        g.FillRectangle(&track, Gdiplus::RectF(trackX, trackY, trackW, trackH));
        g.FillRectangle(&thumb, Gdiplus::RectF(trackX, thumbY, trackW, thumbH));
    }
}

void MainWindow::OnClick(int x, int y) {
    if (x >= tableBox_.left && x <= tableBox_.right && y >= tableBox_.top && y <= tableBox_.bottom) {
        float boxX = static_cast<float>(tableBox_.left);
        float boxW = static_cast<float>(tableBox_.right - tableBox_.left);
        float rowY = static_cast<float>(tableBox_.top) + DpiF(48) + DpiF(34);
        float rowH = DpiF(34);
        const auto& items = mapping_.Items();
        int visibleRows = std::max(0, static_cast<int>((static_cast<float>(tableBox_.bottom - tableBox_.top) - DpiF(48) - DpiF(34) - DpiF(4)) / rowH));
        bool needsScrollbar = static_cast<int>(items.size()) > visibleRows;
        float contentWidth = boxW - (needsScrollbar ? DpiF(18) : 0.0f);
        float c3 = boxX + contentWidth * 0.66f;

        if (needsScrollbar && x >= tableBox_.right - Dpi(18)) {
            float trackY = static_cast<float>(tableBox_.top) + DpiF(48) + rowH + DpiF(4);
            float trackH = static_cast<float>(tableBox_.bottom) - DpiF(10) - trackY;
            float ratio = trackH > 0 ? (static_cast<float>(y) - trackY) / trackH : 0.0f;
            int maxScroll = std::max(0, static_cast<int>(items.size()) - visibleRows);
            tableScrollRows_ = std::clamp(static_cast<int>(ratio * maxScroll + 0.5f), 0, maxScroll);
            InvalidateRect(hwnd_, &tableBox_, FALSE);
            return;
        }

        int startRow = std::min(tableScrollRows_, static_cast<int>(items.size()));
        for (size_t i = static_cast<size_t>(startRow); i < items.size(); ++i) {
            const auto& item = items[i];
            RECT edit{static_cast<LONG>(c3), static_cast<LONG>(rowY + DpiF(5)), static_cast<LONG>(c3 + DpiF(68)), static_cast<LONG>(rowY + DpiF(29))};
            RECT del{static_cast<LONG>(c3 + DpiF(78)), static_cast<LONG>(rowY + DpiF(5)), static_cast<LONG>(c3 + DpiF(154)), static_cast<LONG>(rowY + DpiF(29))};
            POINT pt{x, y};
            if (PtInRect(&edit, pt)) { BeginEdit(item.source); return; }
            if (PtInRect(&del, pt)) { RemoveMapping(item.source); return; }
            rowY += rowH;
        }
        if (state_ != EditState::Idle) CancelEdit();
        return;
    }

    const KeyDef* key = HitKey(static_cast<float>(x), static_cast<float>(y));
    if (key) {
        OnKeyClick(*key);
    } else if (state_ != EditState::Idle) {
        CancelEdit();
    }
}

void MainWindow::OnKeyClick(const KeyDef& key) {
    if (state_ == EditState::Idle) {
        if (mapping_.Find(key.id)) BeginEdit(key.id);
        else BeginCreate(key.id);
        return;
    }
    if (key.id == pendingSource_) {
        CancelEdit();
        return;
    }
    CompleteMapping(key.id);
}

void MainWindow::BeginCreate(const std::wstring& source) {
    state_ = EditState::SelectingTarget;
    pendingSource_ = source;
    feedbackTarget_.clear();
    blinkOn_ = true;
    SetTimer(hwnd_, TIMER_BLINK, 400, nullptr);
    InvalidateBlinkArea();
}

void MainWindow::BeginEdit(const std::wstring& source) {
    state_ = EditState::EditingExisting;
    pendingSource_ = source;
    feedbackTarget_.clear();
    blinkOn_ = true;
    SetTimer(hwnd_, TIMER_BLINK, 400, nullptr);
    InvalidateBlinkArea();
}

void MainWindow::CompleteMapping(const std::wstring& target) {
    mapping_.Set(pendingSource_, target);
    feedbackTarget_ = target;
    ClampTableScroll();
    config_.Save(mapping_, enabled_, languageCode_);
    KillTimer(hwnd_, TIMER_BLINK);
    blinkOn_ = true;
    SetTimer(hwnd_, TIMER_FINISH, 1500, nullptr);
    InvalidateAll();
}

void MainWindow::FinishFeedback() {
    KillTimer(hwnd_, TIMER_FINISH);
    state_ = EditState::Idle;
    pendingSource_.clear();
    feedbackTarget_.clear();
    blinkOn_ = true;
    InvalidateAll();
}

void MainWindow::CancelEdit() {
    KillTimer(hwnd_, TIMER_BLINK);
    KillTimer(hwnd_, TIMER_FINISH);
    state_ = EditState::Idle;
    pendingSource_.clear();
    feedbackTarget_.clear();
    blinkOn_ = true;
    InvalidateAll();
}

void MainWindow::RemoveMapping(const std::wstring& source) {
    mapping_.Remove(source);
    ClampTableScroll();
    if (pendingSource_ == source) FinishFeedback();
    config_.Save(mapping_, enabled_, languageCode_);
    InvalidateAll();
}

void MainWindow::ClampTableScroll() {
    float rowH = DpiF(34);
    int visibleRows = std::max(0, static_cast<int>((tableBox_.bottom - tableBox_.top - DpiF(48) - DpiF(34) - DpiF(4)) / rowH));
    int maxScroll = std::max(0, static_cast<int>(mapping_.Items().size()) - visibleRows);
    tableScrollRows_ = std::clamp(tableScrollRows_, 0, maxScroll);
}

void MainWindow::ScrollTable(int wheelDelta) {
    if (mapping_.Items().empty()) return;
    int rows = wheelDelta / WHEEL_DELTA;
    if (rows == 0) rows = wheelDelta > 0 ? 1 : -1;
    tableScrollRows_ -= rows;
    ClampTableScroll();
    RECT dirty = tableBox_;
    InvalidateRect(hwnd_, &dirty, FALSE);
}

void MainWindow::ChangeLanguageAndRestart(const std::wstring& languageCode) {
    languageCode_ = languageCode;
    config_.Save(mapping_, enabled_, languageCode_);
    ShellExecuteW(nullptr, L"open", ExePath().c_str(), nullptr, ExeDir().c_str(), SW_SHOWNORMAL);
    DestroyWindow(hwnd_);
}

void MainWindow::FlashTestKey(const KeyDef& key) {
    DWORD now = GetTickCount();
    auto it = std::find_if(testKeyFlashes_.begin(), testKeyFlashes_.end(), [&](const TestKeyFlash& flash) {
        return flash.keyId == key.id;
    });
    if (it == testKeyFlashes_.end()) {
        testKeyFlashes_.push_back({key.id, now});
    } else {
        it->startedAt = now;
    }
    RECT dirty = KeyRectPixels(key);
    Inflate(dirty, Dpi(2));
    InvalidateRect(hwnd_, &dirty, FALSE);
    SetTimer(hwnd_, TIMER_KEY_FADE, 30, nullptr);
}

void MainWindow::UpdateTestKeyFade() {
    DWORD now = GetTickCount();
    RECT dirty{};
    bool hasDirty = false;
    for (const auto& flash : testKeyFlashes_) {
        const KeyDef* key = layout_.FindKey(flash.keyId);
        if (!key) continue;
        RECT keyRc = KeyRectPixels(*key);
        Inflate(keyRc, Dpi(2));
        if (!hasDirty) {
            dirty = keyRc;
            hasDirty = true;
        } else {
            UnionRect(&dirty, &dirty, &keyRc);
        }
    }

    testKeyFlashes_.erase(std::remove_if(testKeyFlashes_.begin(), testKeyFlashes_.end(), [&](const TestKeyFlash& flash) {
        return now - flash.startedAt >= 1000;
    }), testKeyFlashes_.end());

    if (hasDirty) InvalidateRect(hwnd_, &dirty, FALSE);
    if (testKeyFlashes_.empty()) KillTimer(hwnd_, TIMER_KEY_FADE);
}

LRESULT MainWindow::HandleMessage(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_SIZE:
        if (wp != SIZE_MINIMIZED) {
            RecomputeLayout();
            ClampTableScroll();
            InvalidateAll();
        }
        return 0;
    case WM_DPICHANGED:
        ResizeForDpi(HIWORD(wp), reinterpret_cast<RECT*>(lp));
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
        Paint();
        return 0;
    case WM_LBUTTONDOWN:
        OnClick(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
        return 0;
    case WM_MOUSEWHEEL: {
        POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        ScreenToClient(hwnd_, &pt);
        if (PtInRect(&tableBox_, pt)) {
            ScrollTable(GET_WHEEL_DELTA_WPARAM(wp));
            return 0;
        }
        break;
    }
    case WM_TIMER:
        if (wp == TIMER_BLINK) {
            blinkOn_ = !blinkOn_;
            InvalidateBlinkArea();
        } else if (wp == TIMER_FINISH) {
            FinishFeedback();
        } else if (wp == TIMER_KEY_FADE) {
            UpdateTestKeyFade();
        }
        return 0;
    case WM_KEY_TEST_FLASH: {
        const KeyDef* key = layout_.FindByHookEvent(static_cast<DWORD>(wp), LOWORD(lp), HIWORD(lp) != 0);
        if (key) FlashTestKey(*key);
        return 0;
    }
    case WM_CLOSE:
        ShowWindow(hwnd_, SW_HIDE);
        return 0;
    case WM_TRAYICON:
        if (LOWORD(lp) == WM_LBUTTONDBLCLK) {
            ShowWindow(hwnd_, SW_SHOW);
            SetForegroundWindow(hwnd_);
        } else if (LOWORD(lp) == WM_RBUTTONUP || LOWORD(lp) == WM_CONTEXTMENU) {
            tray_->ShowMenu();
        }
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDM_TRAY_SHOW:
            ShowWindow(hwnd_, SW_SHOW);
            SetForegroundWindow(hwnd_);
            break;
        case IDM_TRAY_ENABLE:
            enabled_ = !enabled_;
            config_.Save(mapping_, enabled_, languageCode_);
            break;
        case IDM_TRAY_AUTORUN:
            tray_->ToggleAutorun();
            break;
        case IDM_TRAY_LANG_ZH:
            ChangeLanguageAndRestart(L"zh-CN");
            break;
        case IDM_TRAY_LANG_ZHT:
            ChangeLanguageAndRestart(L"zh-TW");
            break;
        case IDM_TRAY_LANG_EN:
            ChangeLanguageAndRestart(L"en");
            break;
        case IDM_TRAY_EXIT:
            DestroyWindow(hwnd_);
            break;
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd_, msg, wp, lp);
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    MainWindow* window = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        window = reinterpret_cast<MainWindow*>(cs->lpCreateParams);
        window->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    return window ? window->HandleMessage(msg, wp, lp) : DefWindowProcW(hwnd, msg, wp, lp);
}
