#include "chip8.h"
#include "platform.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    struct window window;
    platform_create_window(&window);

    if (argc != 3)
        printf("Usage: <CPU Clock> <path/to/rom>");

    uint16_t cpu_clock = atoi(argv[1]);
    double cpu_delay = 1.0 / cpu_clock * 1000;
    const char *filename = argv[2];

    struct chip8 chip8;
    chip8_init(&chip8, 0x200);
    chip8_load_rom(&chip8, filename);

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    uint8_t running = 1;
    while (running)
    {
        if (XPending(window.dpy))
        {
            XNextEvent(window.dpy, &window.e);
            running = platform_process_input(&window, chip8.keypad);
        }
        struct timespec new_time;
        clock_gettime(CLOCK_MONOTONIC, &new_time);

        double dt =
            (new_time.tv_sec - current_time.tv_sec) * 1000.0 + (new_time.tv_nsec - current_time.tv_nsec) / 1000000.0;
        // printf("dt: %lu\n", dt);

        if (dt > cpu_delay)
        {
            current_time = new_time;

            chip8_cycle(&chip8, dt);
            if (chip8.draw_flag)
            {
                platform_update(&window, chip8.display, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                chip8.draw_flag = 0;
            }
        }
    }
    platform_destroy_window(&window);
    return 0;
}
