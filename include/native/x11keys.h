#pragma once

#include <stdint.h>
#include <X11/keysym.h>

enum
{
    KEY_BACKSPACE       = XK_BackSpace,
    KEY_ENTER           = XK_Return,
    KEY_PAUSE           = XK_Pause,
    KEY_ESCAPE          = XK_Escape,
    KEY_LEFT            = XK_Left,
    KEY_SPACE           = XK_space,
    KEY_UP              = XK_Up,
    KEY_RIGHT           = XK_Right,
    KEY_DOWN            = XK_Down,
    KEY_TAB             = XK_Tab,

    KEY_0               = XK_0,
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7,
    KEY_8, KEY_9,

    KEY_A               = XK_a,
    KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H,
    KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,
    KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V,
    KEY_W, KEY_X, KEY_Y, KEY_Z,

    KEY_NUMPAD0         = XK_KP_0,
    KEY_NUMPAD1, KEY_NUMPAD2, KEY_NUMPAD3, KEY_NUMPAD4,
    KEY_NUMPAD5, KEY_NUMPAD6, KEY_NUMPAD7, KEY_NUMPAD8,
    KEY_NUMPAD9,

    KEY_NUMPAD_PLUS     = XK_KP_Add,
    KEY_NUMPAD_MINUS    = XK_KP_Subtract,

    KEY_F1              = XK_F1,
    KEY_F2, KEY_F3, KEY_F4,  KEY_F5,  KEY_F6, KEY_F7,
    KEY_F8, KEY_F9, KEY_F10, KEY_F11,
    KEY_F12             = XK_F12,

    KEY_LSHIFT          = XK_Shift_L,
    KEY_RSHIFT          = XK_Shift_R,

    KEY_LCTRL           = XK_Control_L,
    KEY_RCTRL           = XK_Control_R,

    KEY_LALT            = XK_Alt_L,
    KEY_RALT            = XK_Alt_R,

    MAX_KEY             = 0xffffUL
};

enum
{
    MBUTTON0    = 0x01,
    MBUTTON1    = 0x02,
    MBUTTON2    = 0x04,
    MBUTTON3    = 0x08,
    MBUTTON4    = 0x10
};

enum KEYMOD
{
    KEYMOD_CTRL    = 0x0001,
    KEYMOD_SHIFT   = 0x0002,
    KEYMOD_ALT     = 0x0003,
    MAX_KEYMOD
};

enum KEYMOD_MASK
{
    MODMASK_CTRL = 1 << KEYMOD_CTRL,
    MODMASK_SHIFT = 1 << KEYMOD_SHIFT,
    MODMASK_ALT = 1 << KEYMOD_ALT
};

#define ismbuttondown(buttons, button) ((buttons) & (button))
#define ismbuttonup(buttons, button) (!((buttons) & (button)))
#define iskeydown(kbstate, key) ((kbstate)[key] & 0x80)
#define iskeymod(modstate, mod) ((modstate) & (mod))
