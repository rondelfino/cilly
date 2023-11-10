#include "platform.h"
#include <stdio.h>

#define KEY_ESCAPE 9
#define KEY_1 10
#define KEY_2 11
#define KEY_3 12
#define KEY_4 13

#define KEY_Q 24
#define KEY_W 25
#define KEY_E 26
#define KEY_R 27

#define KEY_A 38
#define KEY_S 39
#define KEY_D 40
#define KEY_F 41

#define KEY_Z 52
#define KEY_X 53
#define KEY_C 54
#define KEY_V 55

#define INVALID_KEY 0x69

void platform_create_window(struct window *window)
{
    /* Establish connect to the X server */
    window->dpy = XOpenDisplay(NULL);
    if (!window->dpy)
    {
        /* handle error */
    }

    window->scr = DefaultScreen(window->dpy);

    window->black = BlackPixel(window->dpy, window->scr);
    window->white = WhitePixel(window->dpy, window->scr);

    uint16_t screen_width = DisplayWidth(window->dpy, window->scr);
    uint16_t screen_height = DisplayHeight(window->dpy, window->scr);

    /* Default window start pos in the center of the screen */
    uint16_t window_x = (screen_width - (screen_width / 2)) / 2;
    uint16_t window_y = (screen_height - (screen_height / 2)) / 2;

    window->w = XCreateSimpleWindow(window->dpy, RootWindow(window->dpy, window->scr), window_x, window_y,
                                    screen_width / 3, (screen_width / 3) / 2, 0, window->black, window->black);

    XStoreName(window->dpy, window->w, "Cilly");
    XMapWindow(window->dpy, window->w);

    /* Events window will receive */
    XSelectInput(window->dpy, window->w,
                 ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
                     StructureNotifyMask);

    window->gc = XCreateGC(window->dpy, window->w, 0, NULL);
    /* Get dimensions of window */
    XWindowAttributes wa;
    XGetWindowAttributes(window->dpy, window->w, &wa);
    window->width = wa.width;
    window->height = wa.height;
}

void platform_destroy_window(struct window *window)
{
    XFreeGC(window->dpy, window->gc);
    XDestroyWindow(window->dpy, window->w);
    XCloseDisplay(window->dpy);
}

uint8_t platform_get_key_from_keycode(KeyCode keycode)
{
    switch (keycode)
    {
    case KEY_1:
        return 0x1;
        break;
    case KEY_2:
        return 0x2;
        break;
    case KEY_3:
        return 0x3;
        break;
    case KEY_4:
        return 0xC;
        break;

    case KEY_Q:
        return 0x4;
        break;
    case KEY_W:
        return 0x5;
        break;
    case KEY_E:
        return 0x6;
        break;
    case KEY_R:
        return 0xD;
        break;

    case KEY_A:
        return 0x7;
        break;
    case KEY_S:
        return 0x8;
        break;
    case KEY_D:
        return 0x9;
        break;
    case KEY_F:
        return 0xE;
        break;

    case KEY_Z:
        return 0xA;
        break;
    case KEY_X:
        return 0x0;
        break;
    case KEY_C:
        return 0xB;
        break;
    case KEY_V:
        return 0xF;
        break;
    default:
        return INVALID_KEY;
        break;
    }
}

uint8_t platform_process_input(struct window *window, uint8_t *keypad)
{
    uint8_t running = 1;
    while (XPending(window->dpy))
    {
        XNextEvent(window->dpy, &window->e);

        /* Get the symbolic representation of the current key */
        KeySym keysym = XLookupKeysym(&window->e.xkey, 0);
        /* Convert it to a keycode */
        KeyCode keycode = XKeysymToKeycode(window->dpy, keysym);
        /* Get the corresponding key for the current keycode */
        uint8_t key = platform_get_key_from_keycode(keycode);

        /* Set on key press */
        switch (window->e.type)
        {
        case KeyPress:
            /* Ensure the key pressed is a valid key */
            if (key != INVALID_KEY)
                keypad[key] = 1;
            else if (keycode == KEY_ESCAPE)
                running = 0;
            break;

        case KeyRelease:
            if (XEventsQueued(window->dpy, QueuedAfterReading))
            {
                XEvent next_e;
                XPeekEvent(window->dpy, &next_e);

                /* Check if key is being held */
                /* This is necessary to prevent auto repeating keys
                 * Ommitting this  will cause the X server to send KeyRelease events after KeyPress events while the key
                 * is held */
                if (next_e.type == KeyPress && next_e.xkey.time == window->e.xkey.time &&
                    next_e.xkey.keycode == window->e.xkey.keycode)
                    /* Key wasn’t actually released */
                    break;
            }
            if (key != INVALID_KEY)
                keypad[key] = 2;
            break;
        }
    }
    return running;
}

void platform_update(struct window *window, uint8_t *display_buffer, uint16_t display_width, uint16_t display_height)
{
    /* If window size changes get new dimensions */
    if (window->e.type == ConfigureNotify || window->e.type == Expose)
    {
        /* Get window attribute to scale sprites based on window size */
        XWindowAttributes wa;
        XGetWindowAttributes(window->dpy, window->w, &wa);

        window->width = wa.width;
        window->height = wa.height;
    }

    uint16_t x_scale = window->width / display_width;
    uint16_t y_scale = window->height / display_height;

    /* Maintain 2:1 aspect ratio */
    uint16_t min_scale = (x_scale < y_scale) ? x_scale : y_scale;

    for (uint8_t y = 0; y < display_height; y++)
    {
        uint16_t y_scaled_pos = y * min_scale;
        uint16_t rect_h = (y + 1) * min_scale - y_scaled_pos;

        for (uint8_t x = 0; x < display_width; x++)
        {
            if (display_buffer[y * display_width + x])
                XSetForeground(window->dpy, window->gc, window->white);
            else
                XSetForeground(window->dpy, window->gc, window->black);

            uint16_t x_scaled_pos = x * min_scale;
            uint16_t rect_w = (x + 1) * min_scale - x_scaled_pos;

            XFillRectangle(window->dpy, window->w, window->gc, x_scaled_pos, y_scaled_pos, rect_w, rect_h);
        }
    }
    XFlush(window->dpy);
}
