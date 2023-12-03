#include "chip8.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>

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
#ifdef _WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER current_time, new_time;
    QueryPerformanceFrequency(&frequency);
#else
    struct timespec current_time, new_time;
#endif

    double dt = 0;
    double dt_refresh = 0;
    double dt_timer = 0;

    get_current_time(&current_time);

    /* TODO: maybe add running to the window struct? */
    uint8_t running = 1;
    while (running)
    {
        get_current_time(&new_time);

#ifdef _WIN32
        dt += get_elapsed_time(current_time, new_time, frequency);
        dt_refresh += get_elapsed_time(current_time, new_time, frequency);
        dt_timer += get_elapsed_time(current_time, new_time, frequency);
#else
        dt += get_elapsed_time(current_time, new_time);
        dt_refresh += get_elapsed_time(current_time, new_time);
        dt_timer += get_elapsed_time(current_time, new_time);
#endif

        current_time = new_time;
        while (dt >= cycle_time)
        {
            chip8_cycle(&chip8);
            dt -= cycle_time;
        }

        /* TODO: symbolic constant 60 hz */
        /* refresh rate 45 Hz */
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
