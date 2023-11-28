#include "chip8.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
    struct window window;
    platform_init(&window);

    if (argc != 3)
    {
        printf("Usage: <clock speed> <path/to/rom>\n");
        return EXIT_FAILURE;
    }

    /* get args */
    uint16_t clock_speed = atoi(argv[1]);
    /* convert given clock speed to microseconds */
    double cycle_time = 1000000.0 / clock_speed;
    const char *filename = argv[2];

    /* TODO: figure out a way to use less timers */
    /* setup chip8 */
    struct chip8 chip8;
    chip8_init(&chip8, START_ADDRESS);
    chip8_load_rom(&chip8, filename);

    /* setup cycle timers */
    struct timespec current_time;
    struct timespec new_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    double dt = 0;

    /* setup delay/sound timer timers */
    struct timespec current_time_timer;
    struct timespec new_time_timer;
    clock_gettime(CLOCK_MONOTONIC, &current_time_timer);
    double dt_timer = 0;

    /* setup refresh timers */
    struct timespec update_start;
    struct timespec update_end;
    clock_gettime(CLOCK_MONOTONIC, &update_start);
    double dt_refresh = 0;

    /* TODO: maybe add running to the window struct? */
    uint8_t running = 1;
    while (running)
    {
        clock_gettime(CLOCK_MONOTONIC, &new_time);

        clock_gettime(CLOCK_MONOTONIC, &new_time_timer);

        clock_gettime(CLOCK_MONOTONIC, &update_end);

        /* Calculate dt in microseconds */
        dt += (new_time.tv_sec - current_time.tv_sec) * 1000000 + (new_time.tv_nsec - current_time.tv_nsec) / 1000.0;

        dt_timer += (new_time_timer.tv_sec - current_time_timer.tv_sec) * 1000000 +
                    (new_time_timer.tv_nsec - current_time_timer.tv_nsec) / 1000.0;

        dt_refresh +=
            (update_end.tv_sec - update_start.tv_sec) * 1000000 + (update_end.tv_nsec - update_start.tv_nsec) / 1000.0;

        current_time = new_time;
        while (dt >= cycle_time)
        {
            chip8_cycle(&chip8);

            dt -= cycle_time;
        }

        update_start = update_end;
        /* TODO: symbolic constant 60 hz */
        if (dt_refresh >= 1000000.0 / 60.0)
        {
            running = platform_process_input(chip8.keypad);
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
    platform_close(&window);
    return EXIT_SUCCESS;
}
