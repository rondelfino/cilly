#include "chip8.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
    struct window window;
    platform_create_window(&window);

    if (argc != 3)
    {
        printf("Usage: <clock speed> <path/to/rom>\n");
        return EXIT_FAILURE;
    }

    uint16_t clock_speed = atoi(argv[1]);
    uint64_t cycle_time = (1.0 / clock_speed) * 1000000;
    const char *filename = argv[2];

    struct chip8 chip8;
    chip8_init(&chip8, START_ADDRESS);
    chip8_load_rom(&chip8, filename);

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    uint64_t acc = 0;
    chip8.running = 1;
    while (chip8.running)
    {
        chip8.running = platform_process_input(&window, chip8.keypad);

        struct timespec new_time;
        clock_gettime(CLOCK_MONOTONIC, &new_time);

        /* Calculate dt in milliseconds */
        uint64_t dt =
            (new_time.tv_sec - current_time.tv_sec) * 1000000 + (new_time.tv_nsec - current_time.tv_nsec) / 1000;

        if (dt > cycle_time)
        {
            current_time = new_time;
            chip8_cycle(&chip8);

            if (chip8.delay_timer > 0)
            {
                acc += dt;
                if (acc >= 1000000 / 60)
                {
                    chip8.delay_timer--;
                    acc = 0;
                }
            }
            if (chip8.draw_flag)
            {
                platform_update(&window, chip8.display, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                chip8.draw_flag = 0;
            }
        }
    }
    platform_destroy_window(&window);
    return EXIT_SUCCESS;
}
