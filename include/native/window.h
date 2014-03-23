#pragma once

#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN

#include "win32keys.h"

#endif

#include "config.h"

#define WF_FULLSCREEN           0x0001
#define WF_HIDEN_CURSOR         0x0002
#define WF_VIDEO_SYNC           0x0004

enum window_event_type
{
    APPLICATION_QUIT            = 0,
    MOUSE_BUTTON_DOWN,
    MOUSE_BUTTON_UP,
    MOUSE_MOVE,
    KEYBOARD_KEY_DOWN,
    KEYBOARD_KEY_UP
};

struct key_event
{
    uint8_t *keys_state;
    uint16_t symbol;
};

struct button_event
{
    uint8_t buttons;
    int x, y;
};

struct motion_event
{
    uint8_t buttons;
    int x, y;
};

struct window_event
{
    enum window_event_type      type;

    union
    {
        struct button_event     button_down;
        struct button_event     button_up;
        struct key_event        key_up;
        struct key_event        key_down;
        struct motion_event     motion;
    };
};

int window_create(int _width, int _height, unsigned _flags);
int window_destroy();
int window_poll(struct window_event *_event);
int window_process();

int window_set_title(const char *title);
int window_set_size(int w, int h);
int window_get_size(int *w, int *h);

void cursor_set_position(int x, int y);

const uint8_t *input_get_keymap();

int opengl_init();
int openal_init();
void opengl_cleanup();
void openal_cleanup();
