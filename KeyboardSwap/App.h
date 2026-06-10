#pragma once

#include "Common.h"
#include "ConfigStore.h"
#include "I18n.h"
#include "KeyboardLayout.h"
#include "KeyMapping.h"
#include "MainWindow.h"

class App {
public:
    App(HINSTANCE instance, std::wstring commandLine);
    int Run();

private:
    HINSTANCE instance_ = nullptr;
    std::wstring commandLine_;
    bool startInTray_ = false;
    ULONG_PTR gdiplusToken_ = 0;
    I18n i18n_;
    KeyboardLayout layout_;
    KeyMapping mapping_;
    ConfigStore config_;
    bool enabled_ = true;
    std::wstring languageCode_;
    std::unique_ptr<MainWindow> window_;
};
