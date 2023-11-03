#include "chip8.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

int main(int argc, char **argv)
{
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy)
    {
        return -1;
    }
    uint32_t scr = DefaultScreen(dpy);

    uint32_t black = BlackPixel(dpy, scr);
    uint32_t white = WhitePixel(dpy, scr);

    Window window = XCreateSimpleWindow(dpy, RootWindow(dpy, scr), 0, 0, 100, 100, 1, black, black);
    XMapWindow(dpy, window);

    XSelectInput(dpy, window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);

    GC gc = XCreateGC(dpy, window, 0, NULL);

    XSetForeground(dpy, gc, white);

    if (argc != 3)
        printf("Usage: <ROM> <CPU clock>");

    const char *filename = argv[1];
    uint8_t cpu_clock = atoi(argv[2]);

    struct chip8 chip8;
    chip8_init(&chip8, 0x200);
    chip8_load_rom(&chip8, filename);

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    uint8_t running = 1;
    while (running)
    {
        XEvent event;
        if (XPending(dpy))
        {
            XNextEvent(dpy, &event);

            /* Set on key press */
            if (event.type == KeyPress)
            {
                printf("Keypress event: %d\n", event.xkey.keycode);
                switch (event.xkey.keycode)
                {
                case KEY_1:
                    chip8.keypad[0] = 1;
                    printf("KEY_1\n");
                    break;
                case KEY_2:
                    chip8.keypad[1] = 1;
                    printf("KEY_2\n");
                    break;
                case KEY_3:
                    chip8.keypad[2] = 1;
                    printf("KEY_3\n");
                    break;
                case KEY_4:
                    chip8.keypad[3] = 1;
                    printf("KEY_4\n");
                    break;

                case KEY_Q:
                    chip8.keypad[4] = 1;
                    printf("KEY_Q\n");
                    break;
                case KEY_W:
                    chip8.keypad[5] = 1;
                    printf("KEY_W\n");
                    break;
                case KEY_E:
                    chip8.keypad[6] = 1;
                    printf("KEY_E\n");
                    break;
                case KEY_R:
                    chip8.keypad[7] = 1;
                    printf("KEY_R\n");
                    break;

                case KEY_A:
                    chip8.keypad[8] = 1;
                    printf("KEY_A\n");
                    break;
                case KEY_S:
                    chip8.keypad[9] = 1;
                    printf("KEY_S\n");
                    break;
                case KEY_D:
                    chip8.keypad[10] = 1;
                    printf("KEY_D\n");
                    break;
                case KEY_F:
                    chip8.keypad[11] = 1;
                    printf("KEY_F\n");
                    break;

                case KEY_Z:
                    chip8.keypad[12] = 1;
                    printf("KEY_Z\n");
                    break;
                case KEY_X:
                    chip8.keypad[13] = 1;
                    printf("KEY_X\n");
                    break;
                case KEY_C:
                    chip8.keypad[14] = 1;
                    printf("KEY_C\n");
                    break;
                case KEY_V:
                    chip8.keypad[15] = 1;
                    printf("KEY_V: %d\n", chip8.keypad[15]);
                    break;
                case KEY_ESCAPE:
                    printf("KEY_ESCAPE\n");
                    running = 0;
                    break;
                }
            }
            /* Reset after key release */
            if (event.type == KeyRelease)
            {
                switch (event.xkey.keycode)
                {
                case KEY_1:
                    chip8.keypad[0] = 0;
                    printf("KEY_1 released\n");
                    break;
                case KEY_2:
                    chip8.keypad[1] = 0;
                    printf("KEY_2 released\n");
                    break;
                case KEY_3:
                    chip8.keypad[2] = 0;
                    printf("KEY_3 released\n");
                    break;
                case KEY_4:
                    chip8.keypad[3] = 0;
                    printf("KEY_4 released\n");
                    break;

                case KEY_Q:
                    chip8.keypad[4] = 0;
                    printf("KEY_Q released\n");
                    break;
                case KEY_W:
                    chip8.keypad[5] = 0;
                    printf("KEY_W released\n");
                    break;
                case KEY_E:
                    chip8.keypad[6] = 0;
                    printf("KEY_E released\n");
                    break;
                case KEY_R:
                    chip8.keypad[7] = 0;
                    printf("KEY_R released\n");
                    break;

                case KEY_A:
                    chip8.keypad[8] = 0;
                    printf("KEY_A released\n");
                    break;
                case KEY_S:
                    chip8.keypad[9] = 0;
                    printf("KEY_S released\n");
                    break;
                case KEY_D:
                    chip8.keypad[10] = 0;
                    printf("KEY_D released\n");
                    break;
                case KEY_F:
                    chip8.keypad[11] = 0;
                    printf("KEY_F released\n");
                    break;

                case KEY_Z:
                    chip8.keypad[12] = 0;
                    printf("KEY_Z released\n");
                    break;
                case KEY_X:
                    chip8.keypad[13] = 0;
                    printf("KEY_X released\n");
                    break;
                case KEY_C:
                    chip8.keypad[14] = 0;
                    printf("KEY_C released\n");
                    break;
                case KEY_V:
                    chip8.keypad[15] = 0;
                    printf("KEY_V released\n");
                    break;
                }
            }
        }
        struct timespec new_time;
        clock_gettime(CLOCK_MONOTONIC, &new_time);

        float dt = (new_time.tv_sec - current_time.tv_sec) + (new_time.tv_nsec - current_time.tv_nsec) / 1e9;

        if (dt > cpu_clock)
        {
            printf("%f\n", dt);
            current_time = new_time;

            chip8_cycle(&chip8, dt);
        }
    }
    XFreeGC(dpy, gc);
    XDestroyWindow(dpy, window);
    XCloseDisplay(dpy);
}
