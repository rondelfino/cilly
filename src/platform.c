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

    /* Create window the size of half the screen */
    window->w = XCreateSimpleWindow(window->dpy, RootWindow(window->dpy, window->scr), window_x, window_y,
                                    screen_width / 2, screen_height / 2, 0, window->black, window->black);

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

uint8_t platform_process_input(struct window *window, uint8_t *keypad)
{
    uint8_t running = 1;
    /* Set on key press */
    switch (window->e.type)
    {
    case KeyPress: {
        switch (window->e.xkey.keycode)
        {
        case KEY_1:
            keypad[1] = 1;
            break;
        case KEY_2:
            keypad[2] = 1;
            break;
        case KEY_3:
            keypad[3] = 1;
            break;
        case KEY_4:
            keypad[0xC] = 1;
            break;

        case KEY_Q:
            keypad[4] = 1;
            break;
        case KEY_W:
            keypad[5] = 1;
            break;
        case KEY_E:
            keypad[6] = 1;
            break;
        case KEY_R:
            keypad[0xD] = 1;
            break;

        case KEY_A:
            keypad[7] = 1;
            break;
        case KEY_S:
            keypad[8] = 1;
            break;
        case KEY_D:
            keypad[9] = 1;
            break;
        case KEY_F:
            keypad[0xE] = 1;
            break;

        case KEY_Z:
            keypad[0xA] = 1;
            break;
        case KEY_X:
            keypad[0] = 1;
            break;
        case KEY_C:
            keypad[0xB] = 1;
            break;
        case KEY_V:
            keypad[0xF] = 1;
            break;

        case KEY_ESCAPE:
            running = 0;
            break;
        }
    }
    break;

    case KeyRelease: {
        switch (window->e.xkey.keycode)
        {
        case KEY_1:
            keypad[1] = 0;
            break;
        case KEY_2:
            keypad[2] = 0;
            break;
        case KEY_3:
            keypad[3] = 0;
            break;
        case KEY_4:
            keypad[0xC] = 0;
            break;

        case KEY_Q:
            keypad[4] = 0;
            break;
        case KEY_W:
            keypad[5] = 0;
            break;
        case KEY_E:
            keypad[6] = 0;
            break;
        case KEY_R:
            keypad[0xD] = 0;
            break;

        case KEY_A:
            keypad[7] = 0;
            break;
        case KEY_S:
            keypad[8] = 0;
            break;
        case KEY_D:
            keypad[9] = 0;
            break;
        case KEY_F:
            keypad[0xE] = 0;
            break;

        case KEY_Z:
            keypad[0xA] = 0;
            break;
        case KEY_X:
            keypad[0] = 0;
            break;
        case KEY_C:
            keypad[0xB] = 0;
            break;
        case KEY_V:
            keypad[0xF] = 0;
            break;
        }
    }
    break;
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

    /* Maintain 64:32 aspect ratio */
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
