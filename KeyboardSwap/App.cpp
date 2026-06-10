#include "App.h"

#include <utility>

App::App(HINSTANCE instance, std::wstring commandLine)
    : instance_(instance), commandLine_(std::move(commandLine)), config_(layout_) {
    startInTray_ = commandLine_.find(L"--tray") != std::wstring::npos || commandLine_.find(L"/tray") != std::wstring::npos;
}

int App::Run() {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    if (Gdiplus::GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput, nullptr) != Gdiplus::Ok) return 1;

    config_.Load(mapping_, enabled_, languageCode_);
    i18n_.SetLanguage(languageCode_);
    config_.Save(mapping_, enabled_, languageCode_);
    window_ = std::make_unique<MainWindow>(instance_, i18n_, layout_, mapping_, config_, enabled_, languageCode_);
    if (!window_->Create(startInTray_)) {
        if (gdiplusToken_) Gdiplus::GdiplusShutdown(gdiplusToken_);
        return 1;
    }

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    config_.Save(mapping_, enabled_, languageCode_);
    window_.reset();
    if (gdiplusToken_) Gdiplus::GdiplusShutdown(gdiplusToken_);
    return static_cast<int>(msg.wParam);
}
