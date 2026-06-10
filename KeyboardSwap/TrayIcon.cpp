#include "TrayIcon.h"

TrayIcon::TrayIcon(HWND hwnd, const I18n& i18n, bool& enabled)
    : hwnd_(hwnd), i18n_(i18n), enabled_(enabled) {}

TrayIcon::~TrayIcon() {
    Remove();
}

void TrayIcon::Add() {
    if (added_) return;
    NOTIFYICONDATAW nid{sizeof(nid)};
    nid.hWnd = hwnd_;
    nid.uID = ID_TRAY;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = static_cast<HICON>(LoadImageW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
    wcscpy_s(nid.szTip, L"Keyboard Swap");
    if (Shell_NotifyIconW(NIM_ADD, &nid)) added_ = true;
    if (IsAutorunEnabled()) SetAutorun(true);
}

void TrayIcon::Remove() {
    if (!added_) return;
    NOTIFYICONDATAW nid{sizeof(nid)};
    nid.hWnd = hwnd_;
    nid.uID = ID_TRAY;
    Shell_NotifyIconW(NIM_DELETE, &nid);
    added_ = false;
}

bool TrayIcon::IsAutorunEnabled() const {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &key) != ERROR_SUCCESS) return false;
    wchar_t value[MAX_PATH]{};
    DWORD size = sizeof(value);
    LONG ok = RegGetValueW(key, nullptr, kRunValueName, RRF_RT_REG_SZ, nullptr, value, &size);
    RegCloseKey(key);
    return ok == ERROR_SUCCESS && std::wstring(value).find(ExePath()) != std::wstring::npos;
}

void TrayIcon::SetAutorun(bool enable) {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS) return;
    if (enable) {
        std::wstring value = L"\"" + ExePath() + L"\" --tray";
        RegSetValueExW(key, kRunValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()), static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValueW(key, kRunValueName);
    }
    RegCloseKey(key);
}

void TrayIcon::ToggleAutorun() {
    SetAutorun(!IsAutorunEnabled());
}

void TrayIcon::ShowMenu() {
    HMENU menu = CreatePopupMenu();
    HMENU languageMenu = CreatePopupMenu();
    AppendMenuW(languageMenu, MF_STRING | (i18n_.IsChinese() && !i18n_.IsTraditionalChinese() ? MF_CHECKED : 0), IDM_TRAY_LANG_ZH, L"简体中文");
    AppendMenuW(languageMenu, MF_STRING | (i18n_.IsTraditionalChinese() ? MF_CHECKED : 0), IDM_TRAY_LANG_ZHT, L"繁體中文");
    AppendMenuW(languageMenu, MF_STRING | (!i18n_.IsChinese() ? MF_CHECKED : 0), IDM_TRAY_LANG_EN, L"English");

    AppendMenuW(menu, MF_STRING, IDM_TRAY_SHOW, i18n_.T(L"显示主窗口", L"Show Window").c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING | (enabled_ ? MF_CHECKED : 0), IDM_TRAY_ENABLE, i18n_.T(L"生效按键映射", L"Enable Key Mapping").c_str());
    AppendMenuW(menu, MF_STRING | (IsAutorunEnabled() ? MF_CHECKED : 0), IDM_TRAY_AUTORUN, i18n_.T(L"开机自动启动", L"Start With Windows").c_str());
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(languageMenu), i18n_.T(L"语言", L"Language").c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, IDM_TRAY_EXIT, i18n_.T(L"退出", L"Exit").c_str());

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd_);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd_, nullptr);
    DestroyMenu(menu);
}
