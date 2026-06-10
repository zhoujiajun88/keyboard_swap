#pragma once

#include "Common.h"
#include "I18n.h"
#include "resource.h"

class TrayIcon {
public:
    TrayIcon(HWND hwnd, const I18n& i18n, bool& enabled);
    ~TrayIcon();

    void Add();
    void Remove();
    void ShowMenu();
    void ToggleAutorun();
    bool IsAutorunEnabled() const;

private:
    void SetAutorun(bool enable);

    HWND hwnd_ = nullptr;
    const I18n& i18n_;
    bool& enabled_;
    bool added_ = false;
};
