#include "KeyboardHook.h"

KeyboardHook* KeyboardHook::self_ = nullptr;

KeyboardHook::KeyboardHook(HINSTANCE instance, HWND notifyHwnd, const KeyboardLayout& layout, const KeyMapping& mapping, bool& enabled)
    : instance_(instance), notifyHwnd_(notifyHwnd), layout_(layout), mapping_(mapping), enabled_(enabled) {
    self_ = this;
}

KeyboardHook::~KeyboardHook() {
    Uninstall();
    if (self_ == this) self_ = nullptr;
}

void KeyboardHook::Install() {
    if (!hook_) hook_ = SetWindowsHookExW(WH_KEYBOARD_LL, Proc, instance_, 0);
}

void KeyboardHook::Uninstall() {
    if (hook_) {
        UnhookWindowsHookEx(hook_);
        hook_ = nullptr;
    }
}

void KeyboardHook::SendKey(const KeyDef& key, bool up) {
    INPUT input{};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = key.scan;
    input.ki.dwFlags = KEYEVENTF_SCANCODE | (key.extended ? KEYEVENTF_EXTENDEDKEY : 0) | (up ? KEYEVENTF_KEYUP : 0);
    input.ki.dwExtraInfo = kInjectedExtraInfo;
    SendInput(1, &input, sizeof(input));
}

LRESULT KeyboardHook::Handle(WPARAM wp, KBDLLHOOKSTRUCT* data) {
    if (!data) return CallNextHookEx(hook_, 0, wp, reinterpret_cast<LPARAM>(data));
    if ((data->flags & LLKHF_INJECTED) || data->dwExtraInfo == kInjectedExtraInfo) {
        return CallNextHookEx(hook_, 0, wp, reinterpret_cast<LPARAM>(data));
    }

    bool up = (wp == WM_KEYUP || wp == WM_SYSKEYUP);
    bool down = (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN);
    if (!up && !down) return CallNextHookEx(hook_, 0, wp, reinterpret_cast<LPARAM>(data));

    const KeyDef* source = layout_.FindByHookEvent(data->vkCode, data->scanCode, (data->flags & LLKHF_EXTENDED) != 0);
    if (!source) return CallNextHookEx(hook_, 0, wp, reinterpret_cast<LPARAM>(data));

    const Mapping* item = mapping_.Find(source->id);
    const KeyDef* displayKey = source;
    if (enabled_ && item) {
        if (const KeyDef* mapped = layout_.FindKey(item->target)) displayKey = mapped;
    }

    if (down && notifyHwnd_ && GetForegroundWindow() == notifyHwnd_) {
        LPARAM packed = MAKELPARAM(displayKey->scan, displayKey->extended ? 1 : 0);
        PostMessageW(notifyHwnd_, WM_KEY_TEST_FLASH, static_cast<WPARAM>(displayKey->vk), packed);
    }

    if (!enabled_ || !item) return CallNextHookEx(hook_, 0, wp, reinterpret_cast<LPARAM>(data));

    const KeyDef* target = layout_.FindKey(item->target);
    if (!target) return CallNextHookEx(hook_, 0, wp, reinterpret_cast<LPARAM>(data));

    SendKey(*target, up);
    return 1;
}

LRESULT CALLBACK KeyboardHook::Proc(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION && self_) return self_->Handle(wp, reinterpret_cast<KBDLLHOOKSTRUCT*>(lp));
    return CallNextHookEx(self_ ? self_->hook_ : nullptr, code, wp, lp);
}
