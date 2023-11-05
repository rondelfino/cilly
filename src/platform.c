#include "platform.h"
#include <X11/X.h>
#include <X11/Xlib.h>
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
    XMapWindow(window->dpy, window->w);

    /* Events window will receive */
    XSelectInput(window->dpy, window->w,
                 ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);

    window->gc = XCreateGC(window->dpy, window->w, 0, NULL);
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
            keypad[0] = 1;
            break;
        case KEY_2:
            keypad[1] = 1;
            printf("KEY_2\n");
            break;
        case KEY_3:
            keypad[2] = 1;
            // printf("KEY_3\n");
            break;
        case KEY_4:
            keypad[3] = 1;
            // printf("KEY_4\n");
            break;

        case KEY_Q:
            keypad[4] = 1;
            // printf("KEY_Q\n");
            break;
        case KEY_W:
            keypad[5] = 1;
            // printf("KEY_W\n");
            break;
        case KEY_E:
            keypad[6] = 1;
            // printf("KEY_E\n");
            break;
        case KEY_R:
            keypad[7] = 1;
            // printf("KEY_R\n");
            break;

        case KEY_A:
            keypad[8] = 1;
            // printf("KEY_A\n");
            break;
        case KEY_S:
            keypad[9] = 1;
            // printf("KEY_S\n");
            break;
        case KEY_D:
            keypad[0xA] = 1;
            // printf("KEY_D\n");
            break;
        case KEY_F:
            keypad[0xB] = 1;
            // printf("KEY_F\n");
            break;

        case KEY_Z:
            keypad[0xC] = 1;
            // printf("KEY_Z\n");
            break;
        case KEY_X:
            keypad[0xD] = 1;
            // printf("KEY_X\n");
            break;
        case KEY_C:
            keypad[0xE] = 1;
            // printf("KEY_C\n");
            break;
        case KEY_V:
            keypad[0xF] = 1;
            printf("KEY_V: %d\n", keypad[0xF]);
            break;
        case KEY_ESCAPE:
            // printf("KEY_ESCAPE\n");
            running = 0;
            break;
        }
    }
    break;

    case KeyRelease: {
        switch (window->e.xkey.keycode)
        {
        case KEY_1:
            keypad[0] = 0;
            // printf("KEY_1 released\n");
            break;
        case KEY_2:
            keypad[1] = 0;
            // printf("KEY_2 released\n");
            break;
        case KEY_3:
            keypad[2] = 0;
            // printf("KEY_3 released\n");
            break;
        case KEY_4:
            keypad[3] = 0;
            // printf("KEY_4 released\n");
            break;

        case KEY_Q:
            keypad[4] = 0;
            // printf("KEY_Q released\n");
            break;
        case KEY_W:
            keypad[5] = 0;
            // printf("KEY_W released\n");
            break;
        case KEY_E:
            keypad[6] = 0;
            // printf("KEY_E released\n");
            break;
        case KEY_R:
            keypad[7] = 0;
            // printf("KEY_R released\n");
            break;

        case KEY_A:
            keypad[8] = 0;
            // printf("KEY_A released\n");
            break;
        case KEY_S:
            keypad[9] = 0;
            // printf("KEY_S released\n");
            break;
        case KEY_D:
            keypad[10] = 0;
            // printf("KEY_D released\n");
            break;
        case KEY_F:
            keypad[11] = 0;
            // printf("KEY_F released\n");
            break;

        case KEY_Z:
            keypad[12] = 0;
            // printf("KEY_Z released\n");
            break;
        case KEY_X:
            keypad[13] = 0;
            // printf("KEY_X released\n");
            break;
        case KEY_C:
            keypad[14] = 0;
            // printf("KEY_C released\n");
            break;
        case KEY_V:
            keypad[15] = 0;
            // printf("KEY_V released\n");
            break;
        }
    }
    break;
    }
    return running;
}

void platform_update(struct window *window, uint8_t *display_buffer, uint16_t display_width, uint16_t display_height)
{
    XWindowAttributes w_attributes;
    XGetWindowAttributes(window->dpy, window->w, &w_attributes);

    double x_scale = ((double)XDisplayWidth(window->dpy, window->scr) / 2) / display_width;
    double y_scale = ((double)XDisplayHeight(window->dpy, window->scr) / 2) / display_height;

    for (int y = 0; y < display_height; y++)
    {
        for (int x = 0; x < display_width; x++)
        {
            if (display_buffer[y * display_width + x])
            {
                XSetForeground(window->dpy, window->gc, window->white);
            }
            else
            {
                XSetForeground(window->dpy, window->gc, window->black);
            }
            int x_scaled = (int)(x * x_scale);
            int y_scaled = (int)(y * y_scale);
            int width_scaled = (int)x_scale;
            int height_scaled = (int)y_scale;

            XFillRectangle(window->dpy, window->w, window->gc, x_scaled, y_scaled, width_scaled, height_scaled);
        }
    }
    XFlush(window->dpy);
}
