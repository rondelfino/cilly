#pragma once

#ifndef PLATFORM_LAYER_H
#define PLATFORM_LAYER_H

#include <stdint.h>

/* TODO: fix redundancy */
#if defined(_WIN32) && !defined(__MINGW64__)
#define WIN 1
#include <Windows.h>
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#include <time.h>
#endif

// #ifdef __MINGW64__
// #include <SDL2/SDL.h>
// #include <time.h>
// #endif

/* SDL window */
struct window
{
    SDL_Window *w;
    SDL_Renderer *renderer;
    SDL_Event e;
};

/* Setup window */
void platform_init(struct window *window);
/* Cleanup window */
void platform_close(struct window *window);
/* Processes input
  @return return 0 if an escape key is pressed */
int8_t platform_process_input(uint8_t *keypad);
/* Get corresponding keypad key from given keycode
 * @return return keypad hex value if input is valid */
uint8_t platform_get_key_from_keycode(SDL_KeyCode keycode);
/* Copies data from given buffer to screen */
void platform_update(struct window *window, uint8_t *display_buffer, uint8_t display_width, uint8_t display_height);

/* Timers */
#ifdef WIN
void get_current_time(LARGE_INTEGER *time);
/* elapsed time in microseconds */
double get_elapsed_time(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER frequency);
#else
void get_current_time(struct timespec *time);
double get_elapsed_time(struct timespec start, struct timespec end);
#endif /* WIN */

#endif /* PLATFORM_LAYER_H */
