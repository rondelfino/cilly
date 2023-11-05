#pragma once

#include <X11/Xlib.h>
#include <stdint.h>

#ifndef PLATFORM_LAYER_H
#define PLATFORM_LAYER_H
/* X11 window */
struct window
{
    Window w;
    Display *dpy;
    GC gc;
    uint16_t scr;
    XEvent e;
    uint16_t width;
    uint16_t height;
    uint32_t black;
    uint32_t white;
};
/* Setup window */
void platform_create_window(struct window *window);
void platform_destroy_window(struct window *window);
/* Processes input
  @return return 0 if an escape key is pressed */
uint8_t platform_process_input(struct window *window, uint8_t *keypad);
/* Redraws the screen */
void platform_update(struct window *window, uint8_t *display_buffer, uint16_t display_width, uint16_t display_height);

#endif
