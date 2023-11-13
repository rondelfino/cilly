#pragma once

#include <X11/X.h>
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
    uint32_t background_col;
    uint32_t foreground_col;
};
/* Setup window */
void platform_create_window(struct window *window);
/* Cleanup window */
void platform_destroy_window(struct window *window);
/* Processes input
  @return return 0 if an escape key is pressed */
uint8_t platform_process_input(struct window *window, uint8_t *keypad);
/* Get corresponding keypad key from given keycode 
 * @return return keypad hex value if input is valid */
uint8_t platform_get_key_from_keycode(KeyCode keycode);
/* Copies data from given buffer to screen */
void platform_update(struct window *window, uint8_t *display_buffer, uint8_t display_width, uint8_t display_height);

#endif
