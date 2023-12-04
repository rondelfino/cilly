#include "platform.h"
#include <stdio.h>
#include <stdlib.h>

#define INVALID_KEY -1

void platform_init(struct window *window)
{
    // int8_t success = 0;
    window->w = NULL;

    /* return -1 if fails */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL initialization failed. Error: %s\n", SDL_GetError());
    }
    else
    {
        /* get native resolution */
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);
        uint16_t screen_width = DM.w;

        /* adjust window resolution to 2:1 */
        window->w = SDL_CreateWindow("Cilly", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width / 3,
                                     (screen_width / 3) / 2, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

        if (!window->w)
        {
            fprintf(stderr, "Window could not be created. Error: %s\n", SDL_GetError());
        }

        window->renderer = SDL_CreateRenderer(window->w, -1, 0);
        if (!window->renderer)
        {
            fprintf(stderr, "Renderer could not be created. Error: %s\n", SDL_GetError());
        }
    }
}

void platform_close(struct window *window)
{
    SDL_DestroyWindow(window->w);
    SDL_DestroyRenderer(window->renderer);
    SDL_Quit();
}

uint8_t platform_get_key_from_keycode(SDL_KeyCode keycode)
{
    switch (keycode)
    {
    case SDLK_1:
        return 0x1;
        break;
    case SDLK_2:
        return 0x2;
        break;
    case SDLK_3:
        return 0x3;
        break;
    case SDLK_4:
        return 0xC;
        break;

    case SDLK_q:
        return 0x4;
        break;
    case SDLK_w:
        return 0x5;
        break;
    case SDLK_e:
        return 0x6;
        break;
    case SDLK_r:
        return 0xD;
        break;

    case SDLK_a:
        return 0x7;
        break;
    case SDLK_s:
        return 0x8;
        break;
    case SDLK_d:
        return 0x9;
        break;
    case SDLK_f:
        return 0xE;
        break;

    case SDLK_z:
        return 0xA;
        break;
    case SDLK_x:
        return 0x0;
        break;
    case SDLK_c:
        return 0xB;
        break;
    case SDLK_v:
        return 0xF;
        break;
    default:
        return INVALID_KEY;
        break;
    }
}

int8_t platform_process_input(uint8_t *keypad)
{
    int8_t running = 1;

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            running = -1;
            break;
        case SDL_KEYDOWN: {
            /* Get the corresponding key for the current keycode */
            SDL_Keycode keycode = e.key.keysym.sym;
            int8_t key = platform_get_key_from_keycode(keycode);

            if (keycode == SDLK_ESCAPE)
                running = 0;
            else if (key != INVALID_KEY)
                /* key pressed */
                keypad[key] = 1;
        }
        break;
        case SDL_KEYUP: {
            /* Get the corresponding key for the current keycode */
            int8_t key = platform_get_key_from_keycode(e.key.keysym.sym);
            if (key != INVALID_KEY)
                keypad[key] = 2;

            break;
        }
        }
    }
    return running;
}

void platform_update(struct window *window, uint8_t *display_buffer, uint8_t display_width, uint8_t display_height)
{
    SDL_Rect rect;
    uint16_t min_scale;

    /* TODO: change this to calculate only when window is resized. might need SDL_Event and min_scale in the window struct */
    /* calculate minimum scale */
    if (SDL_GetWindowFlags(window->w) & SDL_WINDOW_RESIZABLE)
    {
        int w, h;
        SDL_GetWindowSize(window->w, &w, &h);
        min_scale = SDL_min(w / display_width, h / display_height);
    }
    else
    {
        min_scale = 1;
    }

    for (uint8_t y = 0; y < display_height; y++)
    {
        uint16_t y_scaled_pos = y * min_scale;
        uint16_t rect_h = (y + 1) * min_scale - y_scaled_pos;

        for (uint8_t x = 0; x < display_width; x++)
        {
            uint16_t x_scaled_pos = x * min_scale;
            uint16_t rect_w = (x + 1) * min_scale - x_scaled_pos;

            rect.x = x_scaled_pos;
            rect.y = y_scaled_pos;
            rect.w = rect_w;
            rect.h = rect_h;

            if (display_buffer[y * display_width + x])
                SDL_SetRenderDrawColor(window->renderer, 255, 255, 255, 255);
            else
                SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 255);

            SDL_RenderFillRect(window->renderer, &rect);
        }
    }

    SDL_RenderPresent(window->renderer);
}

#ifdef WIN
void get_current_time(LARGE_INTEGER *time)
{
    QueryPerformanceCounter(time);
}

double get_elapsed_time(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER frequency)
{
    return (double)(end.QuadPart - start.QuadPart) * 1000000.0 / frequency.QuadPart;
}
#else
void get_current_time(struct timespec *time)
{
    clock_gettime(CLOCK_MONOTONIC, time);
}

double get_elapsed_time(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000.0;
}
#endif
