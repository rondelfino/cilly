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
    double cycle_time = 1000000.0 / clock_speed;
    const char *filename = argv[2];

    struct chip8 chip8;
    chip8_init(&chip8, START_ADDRESS);

    chip8_load_rom(&chip8, filename);

    struct timespec current_time;
    struct timespec new_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    struct timespec current_time_timer;
    struct timespec new_time_timer;
    clock_gettime(CLOCK_MONOTONIC, &current_time_timer);

    struct timespec r_s;
    struct timespec r_e;
    clock_gettime(CLOCK_MONOTONIC, &r_s);

    double dt = 0;
    double dt_timer = 0;
    double dt_refresh = 0;
    uint8_t running = 1;
    while (running)
    {
        clock_gettime(CLOCK_MONOTONIC, &new_time);

        clock_gettime(CLOCK_MONOTONIC, &new_time_timer);

        clock_gettime(CLOCK_MONOTONIC, &r_e);

        /* Calculate dt in microseconds */
        dt += (new_time.tv_sec - current_time.tv_sec) * 1000000 + (new_time.tv_nsec - current_time.tv_nsec) / 1000.0;

        dt_timer += (new_time_timer.tv_sec - current_time_timer.tv_sec) * 1000000 +
                    (new_time_timer.tv_nsec - current_time_timer.tv_nsec) / 1000.0;

        dt_refresh += (r_e.tv_sec - r_s.tv_sec) * 1000000 + (r_e.tv_nsec - r_s.tv_nsec) / 1000.0;

        current_time = new_time;
        while (dt >= cycle_time)
        {
            chip8_cycle(&chip8);

            dt -= cycle_time;
        }

        r_s = r_e;
        if (dt_refresh >= 1000000.0 / 60.0)
        {
            running = platform_process_input(&window, chip8.keypad);
            dt_refresh -= 1000000.0 / 60.0;
            if (chip8.draw_flag)
            {
                platform_update(&window, chip8.display, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                chip8.draw_flag = 0;
            }
        }

        /* Decrement by 1, 60 times per second */
        /* if dt > 1/60th of a second, decrement timers */
        current_time_timer = new_time_timer;
        if (dt_timer >= 1000000.0 / 60.0)
        {
            dt_timer -= 1000000.0 / 60.0;
            if (chip8.delay_timer > 0)
            {
                chip8.delay_timer--;
            }
        }
    }
    platform_destroy_window(&window);
    return EXIT_SUCCESS;
}
