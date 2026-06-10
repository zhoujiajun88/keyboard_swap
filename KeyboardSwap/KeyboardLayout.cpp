#include "KeyboardLayout.h"

namespace {

KeyDef MakeKey(const wchar_t* id, const wchar_t* zh, const wchar_t* en, WORD vk, bool ext, float x, float y, float w, float h) {
    return {id, zh, en, vk, Scan(vk, ext), ext, Gdiplus::RectF(x, y, w, h)};
}

}

KeyboardLayout::KeyboardLayout() {
    auto add = [&](const wchar_t* id, const wchar_t* zh, const wchar_t* en, WORD vk, bool ext, float x, float y, float w, float h) {
        keys_.push_back(MakeKey(id, zh, en, vk, ext, x, y, w, h));
    };

    add(L"ESC", L"Esc", L"Esc", VK_ESCAPE, false, 104, 98, 67, 66);
    add(L"F1", L"F1", L"F1", VK_F1, false, 248, 98, 67, 66);
    add(L"F2", L"F2", L"F2", VK_F2, false, 321, 98, 68, 66);
    add(L"F3", L"F3", L"F3", VK_F3, false, 395, 98, 68, 66);
    add(L"F4", L"F4", L"F4", VK_F4, false, 469, 98, 68, 66);
    add(L"F5", L"F5", L"F5", VK_F5, false, 575, 98, 68, 66);
    add(L"F6", L"F6", L"F6", VK_F6, false, 648, 98, 67, 66);
    add(L"F7", L"F7", L"F7", VK_F7, false, 721, 98, 67, 66);
    add(L"F8", L"F8", L"F8", VK_F8, false, 793, 98, 66, 66);
    add(L"F9", L"F9", L"F9", VK_F9, false, 896, 98, 68, 66);
    add(L"F10", L"F10", L"F10", VK_F10, false, 969, 98, 68, 66);
    add(L"F11", L"F11", L"F11", VK_F11, false, 1042, 98, 67, 66);
    add(L"F12", L"F12", L"F12", VK_F12, false, 1114, 98, 67, 66);
    add(L"PRTSC", L"Print", L"Print", VK_SNAPSHOT, true, 1201, 98, 68, 66);
    add(L"SCROLL", L"Scroll", L"Scroll", VK_SCROLL, false, 1275, 98, 68, 66);
    add(L"PAUSE", L"Pause", L"Pause", VK_PAUSE, false, 1349, 98, 68, 66);

    add(L"GRAVE", L"`", L"`", VK_OEM_3, false, 104, 197, 66, 67);
    add(L"1", L"1", L"1", '1', false, 176, 197, 66, 67);
    add(L"2", L"2", L"2", '2', false, 248, 197, 66, 67);
    add(L"3", L"3", L"3", '3', false, 320, 197, 66, 67);
    add(L"4", L"4", L"4", '4', false, 392, 197, 66, 67);
    add(L"5", L"5", L"5", '5', false, 463, 197, 66, 67);
    add(L"6", L"6", L"6", '6', false, 536, 197, 66, 67);
    add(L"7", L"7", L"7", '7', false, 608, 197, 66, 67);
    add(L"8", L"8", L"8", '8', false, 680, 197, 66, 67);
    add(L"9", L"9", L"9", '9', false, 752, 197, 66, 67);
    add(L"0", L"0", L"0", '0', false, 824, 197, 65, 67);
    add(L"MINUS", L"-", L"-", VK_OEM_MINUS, false, 895, 197, 66, 67);
    add(L"EQUAL", L"=", L"=", VK_OEM_PLUS, false, 967, 197, 66, 67);
    add(L"BACKSPACE", L"退格", L"Backspace", VK_BACK, false, 1039, 197, 142, 67);
    add(L"INSERT", L"Insert", L"Insert", VK_INSERT, true, 1201, 197, 67, 67);
    add(L"HOME", L"Home", L"Home", VK_HOME, true, 1274, 197, 68, 67);
    add(L"PGUP", L"PgUp", L"PgUp", VK_PRIOR, true, 1348, 197, 68, 67);
    add(L"NUMLOCK", L"Num", L"Num", VK_NUMLOCK, true, 1437, 199, 62, 65);
    add(L"NUMDIVIDE", L"/", L"Numpad /", VK_DIVIDE, true, 1505, 199, 58, 65);
    add(L"NUMMULTIPLY", L"*", L"Numpad *", VK_MULTIPLY, false, 1568, 199, 58, 65);
    add(L"NUMSUBTRACT", L"-", L"Numpad -", VK_SUBTRACT, false, 1632, 199, 57, 65);

    add(L"TAB", L"Tab", L"Tab", VK_TAB, false, 104, 270, 102, 68);
    add(L"Q", L"Q", L"Q", 'Q', false, 212, 270, 66, 68);
    add(L"W", L"W", L"W", 'W', false, 284, 270, 66, 68);
    add(L"E", L"E", L"E", 'E', false, 356, 270, 66, 68);
    add(L"R", L"R", L"R", 'R', false, 428, 270, 66, 68);
    add(L"T", L"T", L"T", 'T', false, 500, 270, 66, 68);
    add(L"Y", L"Y", L"Y", 'Y', false, 572, 270, 66, 68);
    add(L"U", L"U", L"U", 'U', false, 644, 270, 66, 68);
    add(L"I", L"I", L"I", 'I', false, 716, 270, 66, 68);
    add(L"O", L"O", L"O", 'O', false, 788, 270, 65, 68);
    add(L"P", L"P", L"P", 'P', false, 859, 270, 66, 68);
    add(L"LBRACKET", L"[", L"[", VK_OEM_4, false, 931, 270, 66, 68);
    add(L"RBRACKET", L"]", L"]", VK_OEM_6, false, 1003, 270, 66, 68);
    add(L"BACKSLASH", L"\\", L"\\", VK_OEM_5, false, 1075, 270, 106, 68);
    add(L"DELETE", L"Delete", L"Delete", VK_DELETE, true, 1201, 270, 67, 68);
    add(L"END", L"End", L"End", VK_END, true, 1274, 270, 68, 68);
    add(L"PGDN", L"PgDn", L"PgDn", VK_NEXT, true, 1348, 270, 68, 68);
    add(L"NUM7", L"7", L"Numpad 7", VK_NUMPAD7, false, 1437, 270, 62, 68);
    add(L"NUM8", L"8", L"Numpad 8", VK_NUMPAD8, false, 1505, 270, 58, 68);
    add(L"NUM9", L"9", L"Numpad 9", VK_NUMPAD9, false, 1568, 270, 58, 68);
    add(L"NUMADD", L"+", L"Numpad +", VK_ADD, false, 1632, 270, 57, 144);

    add(L"CAPSLOCK", L"Caps", L"Caps", VK_CAPITAL, false, 104, 346, 122, 68);
    add(L"A", L"A", L"A", 'A', false, 232, 346, 66, 68);
    add(L"S", L"S", L"S", 'S', false, 304, 346, 66, 68);
    add(L"D", L"D", L"D", 'D', false, 376, 346, 66, 68);
    add(L"F", L"F", L"F", 'F', false, 448, 346, 66, 68);
    add(L"G", L"G", L"G", 'G', false, 520, 346, 66, 68);
    add(L"H", L"H", L"H", 'H', false, 592, 346, 65, 68);
    add(L"J", L"J", L"J", 'J', false, 663, 346, 66, 68);
    add(L"K", L"K", L"K", 'K', false, 735, 346, 66, 68);
    add(L"L", L"L", L"L", 'L', false, 807, 346, 65, 68);
    add(L"SEMICOLON", L";", L";", VK_OEM_1, false, 879, 346, 65, 68);
    add(L"APOSTROPHE", L"'", L"'", VK_OEM_7, false, 950, 346, 66, 68);
    add(L"ENTER", L"回车", L"Enter", VK_RETURN, false, 1023, 346, 158, 68);
    add(L"NUM4", L"4", L"Numpad 4", VK_NUMPAD4, false, 1437, 346, 62, 68);
    add(L"NUM5", L"5", L"Numpad 5", VK_NUMPAD5, false, 1505, 346, 58, 68);
    add(L"NUM6", L"6", L"Numpad 6", VK_NUMPAD6, false, 1568, 346, 58, 68);

    add(L"LSHIFT", L"左Shift", L"Left Shift", VK_LSHIFT, false, 104, 420, 160, 68);
    add(L"Z", L"Z", L"Z", 'Z', false, 271, 420, 65, 68);
    add(L"X", L"X", L"X", 'X', false, 342, 420, 66, 68);
    add(L"C", L"C", L"C", 'C', false, 414, 420, 66, 68);
    add(L"V", L"V", L"V", 'V', false, 486, 420, 66, 68);
    add(L"B", L"B", L"B", 'B', false, 558, 420, 65, 68);
    add(L"N", L"N", L"N", 'N', false, 629, 420, 66, 68);
    add(L"M", L"M", L"M", 'M', false, 701, 420, 66, 68);
    add(L"COMMA", L",", L",", VK_OEM_COMMA, false, 773, 420, 66, 68);
    add(L"PERIOD", L".", L".", VK_OEM_PERIOD, false, 845, 420, 66, 68);
    add(L"SLASH", L"/", L"/", VK_OEM_2, false, 916, 420, 66, 68);
    add(L"RSHIFT", L"右Shift", L"Right Shift", VK_RSHIFT, false, 988, 420, 193, 68);
    add(L"UP", L"上", L"Up", VK_UP, true, 1274, 423, 68, 69);
    add(L"NUM1", L"1", L"Numpad 1", VK_NUMPAD1, false, 1437, 420, 62, 68);
    add(L"NUM2", L"2", L"Numpad 2", VK_NUMPAD2, false, 1505, 420, 58, 68);
    add(L"NUM3", L"3", L"Numpad 3", VK_NUMPAD3, false, 1568, 420, 58, 68);
    add(L"NUMENTER", L"回车", L"Numpad Enter", VK_RETURN, true, 1632, 420, 57, 145);

    add(L"LCTRL", L"左Ctrl", L"Left Ctrl", VK_LCONTROL, false, 104, 497, 88, 69);
    add(L"LWIN", L"左Win", L"Left Win", VK_LWIN, true, 199, 497, 86, 69);
    add(L"LALT", L"左Alt", L"Left Alt", VK_LMENU, false, 291, 497, 88, 69);
    add(L"SPACE", L"空格", L"Space", VK_SPACE, false, 385, 497, 434, 69);
    add(L"RALT", L"右Alt", L"Right Alt", VK_RMENU, true, 825, 497, 87, 69);
    add(L"RWIN", L"右Win", L"Right Win", VK_RWIN, true, 919, 497, 85, 69);
    add(L"APP", L"菜单", L"Menu", VK_APPS, true, 1009, 497, 76, 69);
    add(L"RCTRL", L"右Ctrl", L"Right Ctrl", VK_RCONTROL, true, 1092, 497, 89, 69);
    add(L"LEFT", L"左", L"Left", VK_LEFT, true, 1201, 499, 67, 68);
    add(L"DOWN", L"下", L"Down", VK_DOWN, true, 1274, 499, 68, 68);
    add(L"RIGHT", L"右", L"Right", VK_RIGHT, true, 1348, 499, 68, 68);
    add(L"NUM0", L"0", L"Numpad 0", VK_NUMPAD0, false, 1437, 496, 126, 69);
    add(L"NUMDECIMAL", L".", L"Numpad .", VK_DECIMAL, false, 1568, 497, 58, 68);
}

const KeyDef* KeyboardLayout::FindKey(const std::wstring& id) const {
    auto it = std::find_if(keys_.begin(), keys_.end(), [&](const KeyDef& key) { return key.id == id; });
    return it == keys_.end() ? nullptr : &*it;
}

const KeyDef* KeyboardLayout::FindByHookEvent(DWORD vkCode, DWORD scanCode, bool extended) const {
    for (const auto& key : keys_) {
        if (key.vk == vkCode) {
            if (key.vk == VK_SHIFT || key.vk == VK_CONTROL || key.vk == VK_MENU) continue;
            if ((key.vk == VK_RETURN || key.vk == VK_DIVIDE || key.vk == VK_LCONTROL || key.vk == VK_RCONTROL || key.vk == VK_LMENU || key.vk == VK_RMENU || key.vk == VK_LWIN || key.vk == VK_RWIN) && key.extended != extended) continue;
            return &key;
        }
    }

    WORD vk = static_cast<WORD>(vkCode);
    if (vk == VK_SHIFT) vk = static_cast<WORD>(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
    if (vk == VK_CONTROL) vk = extended ? VK_RCONTROL : VK_LCONTROL;
    if (vk == VK_MENU) vk = extended ? VK_RMENU : VK_LMENU;

    for (const auto& key : keys_) {
        if (key.vk == vk) return &key;
    }
    return nullptr;
}
