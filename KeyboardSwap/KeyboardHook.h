#pragma once

#include "Common.h"
#include "KeyboardLayout.h"
#include "KeyMapping.h"
#include "resource.h"

class KeyboardHook {
public:
    KeyboardHook(HINSTANCE instance, HWND notifyHwnd, const KeyboardLayout& layout, const KeyMapping& mapping, bool& enabled);
    ~KeyboardHook();

    void Install();
    void Uninstall();

private:
    static LRESULT CALLBACK Proc(int code, WPARAM wp, LPARAM lp);

    LRESULT Handle(WPARAM wp, KBDLLHOOKSTRUCT* data);
    void SendKey(const KeyDef& key, bool up);

    HINSTANCE instance_ = nullptr;
    HWND notifyHwnd_ = nullptr;
    HHOOK hook_ = nullptr;
    const KeyboardLayout& layout_;
    const KeyMapping& mapping_;
    bool& enabled_;
    static KeyboardHook* self_;
};
