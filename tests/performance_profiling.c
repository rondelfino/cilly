#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_CYCLES 1000000000

void test_chip8_fptr_cycle(struct chip8 *chip8)
{
    /* Fetch opcode */
    chip8->opcode = chip8->memory[chip8->PC];
    chip8->opcode <<= 8;
    chip8->opcode |= chip8->memory[chip8->PC + 1];

    /* Point to next opcode */
    chip8->PC += 2;

    /* Decode and execute - index with opcode group id (first nibble) */
    chip8->main_table[(chip8->opcode >> 12) & 0xF](chip8);

    /* Set released keys to idle */
    chip8_reset_released_keys(chip8);
}

void test_chip8_switch_cycle(struct chip8 *chip8)
{
    /* Fetch opcode */
    uint16_t opcode = chip8->memory[chip8->PC];
    opcode <<= 8;
    opcode |= chip8->memory[chip8->PC + 1];

    uint8_t group_id = (opcode >> 12) & 0xF;

    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t n = opcode & 0xF;
    uint8_t nn = opcode & 0xFF;
    uint16_t address = opcode & 0xFFF;

    /* Point to next opcode */
    chip8->PC += 2;

    /* Decode and execute */
    switch (group_id)
    {
    case 0x0:
        switch (nn)
        {
        /* 00E0 */
        case 0xE0:
            chip8_clear_display(chip8);
            break;

        /* 00EE */
        case 0xEE:
            chip8->SP--;
            chip8->PC = chip8->stack[chip8->SP];
            break;

        default:
            /* Unknown opcode */
            break;
        }
        break;

    /* 1NNN */
    case 0x1:
        chip8->PC = address;
        break;

    /* 2NNN */
    case 0x2:
        if (chip8->SP >= STACK_SIZE)
        {
            printf("Stack overflow detected. Terminating program.\n");
            exit(EXIT_FAILURE);
        }

        chip8->stack[chip8->SP] = chip8->PC;
        chip8->SP++;
        chip8->PC = address;
        break;

    /* 3XNN */
    case 0x3:
        if (chip8->V[x] == nn)
            chip8->PC += 2;
        break;

    /* 4XNN */
    case 0x4:
        if (chip8->V[x] != nn)
            chip8->PC += 2;
        break;

    /* 5XY0 */
    case 0x5:
        if (chip8->V[x] == chip8->V[y])
            chip8->PC += 2;
        break;

    /* 6XNN */
    case 0x6:
        chip8->V[x] = nn;
        break;

    /* 7XNN */
    case 0x7:
        chip8->V[x] += nn;
        break;

    case 0x8:
        switch (n)
        {
        /* 8XY0 */
        case 0x0:
            chip8->V[x] = chip8->V[y];
            break;

        /* 8XY1 */
        case 0x1:
            chip8->V[x] |= chip8->V[y];
            /* legacy */
            chip8->V[0xF] = 0;
            break;

        /* 8XY2 */
        case 0x2:
            chip8->V[x] &= chip8->V[y];
            /* legacy */
            chip8->V[0xF] = 0;
            break;

        /* 8XY3 */
        case 0x3:
            chip8->V[x] ^= chip8->V[y];
            /* legacy */
            chip8->V[0xF] = 0;
            break;

        /* 8XY4 */
        case 0x4: {
            uint16_t sum = chip8->V[x] + chip8->V[y];
            /* cap at 0xFF (255) */
            chip8->V[x] = sum;
            chip8->V[0xF] = (sum >= 0xFF) ? 1 : 0;
        }
        break;

        /* 8XY5 */
        case 0x5: {
            uint8_t temp = chip8->V[x];

            chip8->V[x] -= chip8->V[y];
            /* If there's no borrow */
            chip8->V[0xF] = (temp >= chip8->V[y]) ? 1 : 0;
        }
        break;

        /* 8XY6 */
        case 0x6: {
            /* COSMAC-VIP */
            /* TODO(nael): support legacy config/quirks */
            chip8->V[x] = chip8->V[y];
            uint8_t temp = chip8->V[x];

            chip8->V[x] >>= 1;
            chip8->V[0xF] = temp & 0x1;
        }
        break;

        /* 8XY7 */
        case 0x7:
            chip8->V[x] = chip8->V[y] - chip8->V[x];
            /* If there's no borrow */
            chip8->V[0xF] = (chip8->V[y] >= chip8->V[x]) ? 1 : 0;
            break;

        /* 8XYE */
        case 0xE: {
            /* COSMAC-VIP specific */
            chip8->V[x] = chip8->V[y];

            uint8_t temp = chip8->V[x];

            chip8->V[x] <<= 1;
            chip8->V[0xF] = temp >> 7;
        }
        break;

        default:
            /* Unknown opcode */
            break;
        }
        break;

    /* 9XY0 */
    case 0x9:
        if (chip8->V[x] != chip8->V[y])
            chip8->PC += 2;
        break;

    /* ANNN */
    case 0xA:
        chip8->I = address;
        break;

    /* BNNN */
    case 0xB: {
        /* uint8_t x = (chip8->opcode >> 8) & 0xF;
              uint16_t xnn = ((chip8->opcode >> 8) & 0xF) << 8) | chip8->opcode & 0xFF;
              chip8->PC = xnn + chip8->V[x];*/
        /* TODO(nael): support legacy config/quirks */
        chip8->PC = address + chip8->V[0];
    }
    break;

    /* CXNN */
    case 0xC: {
        uint8_t r = rand();

        chip8->V[x] = r & nn;
    }
    break;

    /* DXYN */
    case 0xD: {
        chip8->draw_flag = 1;
        uint8_t x_pos = chip8->V[x];
        uint8_t y_pos = chip8->V[y];

        chip8->V[0xF] = 0;
        /* Draw sprite from address in I to N */
        for (uint8_t row = 0; row < n; row++)
        {
            /* Stop drawing if bottom edge is reached */
            if (y_pos + row >= DISPLAY_HEIGHT)
                break;

            /* Extract byte at current row */
            uint8_t sprite_byte = chip8->memory[chip8->I + row];

            /* Iterate over extracted byte */
            for (uint8_t col = 0; col < 8; col++)
            {
                /* Stop drawing if right edge is reached */
                if (x_pos + col >= DISPLAY_WIDTH)
                    break;

                /* Extract current bit */
                uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 0x1;
                /* Get current pixel value from display buffer */
                uint8_t *display_pixel = &chip8->display[((y_pos + row) * DISPLAY_WIDTH) + (x_pos + col)];

                /* Extracted bit from sprite is set */
                if (sprite_pixel)
                {
                    /* Current pixel from display is set */
                    if (*display_pixel)
                        /* Collision: set VF to 1 */
                        chip8->V[0xF] = 1;

                    /* Set display pixel if it's currently 0, or unset it if it's currently 1 */
                    *display_pixel ^= 1;
                }
            }
        }
        break;
    }

    case 0xE:
        switch (nn)
        {
        /* EXA1 */
        case 0xA1: {
            uint8_t key = chip8->V[x];

            if (chip8->keypad[key] != 1)
                chip8->PC += 2;
        }
        break;

        /* EX9E */
        case 0x9E: {
            uint8_t key = chip8->V[x];

            if (chip8->keypad[key] == 1)
                chip8->PC += 2;
        }
        break;

        default:
            /* Unknown opcode */
            break;
        }
        break;

    case 0xF:
        switch (nn)
        {
        /* FX07 */
        case 0x07:
            chip8->V[x] = chip8->delay_timer;
            break;

        /* FX0A */
        case 0xA: {
            uint8_t key_released = 0;

            for (uint8_t i = 0; i < KEY_COUNT; i++)
            {
                if (chip8->keypad[i] == 2)
                {
                    chip8->V[x] = i;
                    key_released = 1;
                    break;
                }
            }

            if (!key_released)
            {
                chip8->PC -= 2;
            }
        }
        break;

        /* FX15 */
        case 0x15:
            chip8->delay_timer = chip8->V[x];
            break;

        /* FX18 */
        case 0x18:
            chip8->sound_timer = chip8->V[x];
            break;

        /* FX1E */
        case 0x1E:
            chip8->I += chip8->V[x];
            chip8->V[0xF] = chip8->I + chip8->V[x] > 0xFFF ? 1 : 0;
            break;

        /* FX29 */
        case 0x29: {
            /* Hexadecimal digit in V[X] */
            uint8_t digit = chip8->V[x];

            /* Set I to the memory address of the digit */
            /* 5 is the size of a row in the fontset */
            chip8->I = FONTSET_START_ADDRESS + (5 * digit);
        }
        break;

        /* FX33 */
        case 0x33: {
            uint8_t value = chip8->V[x];

            chip8->memory[chip8->I + 2] = value % 10;
            value /= 10;

            chip8->memory[chip8->I + 1] = value % 10;
            value /= 10;

            chip8->memory[chip8->I] = value % 10;
        }
        break;

        /* FX55 */
        case 0x55:
            for (size_t i = 0; i <= x; i++)
            {
                chip8->memory[chip8->I + i] = chip8->V[i];
            }
            /* TODO(nael): support legacy config/quirks */
            /* COSMAC-VIP specific */
            chip8->I += x + 1;
            break;

        /* FX65 */
        case 0x65:

            for (size_t i = 0; i <= x; i++)
            {
                chip8->V[i] = chip8->memory[chip8->I + i];
            }
            /* TODO(nael): support legacy config/quirks */
            /* COSMAC-VIP specific */
            chip8->I += x + 1;
            break;

        default:
            /* Unknown opcode */
            break;
        }
        break;

    default:
        /* Unknown opcode */
        break;
    }
    /* Set released keys to idle */
    chip8_reset_released_keys(chip8);
}

int main(int argc, char **argv)
{

    struct chip8 chip8;
    chip8_init(&chip8, START_ADDRESS);
    chip8_load_rom(&chip8, argv[1]);

    uint32_t i = 0;
    clock_t start_fptr_tables = clock();
    while (i < NUM_CYCLES)
    {
        test_chip8_fptr_cycle(&chip8);
        i++;
    }

    clock_t end_fptr_tables = clock();
    double ptr_elapsed_time = ((double)(end_fptr_tables - start_fptr_tables)) / CLOCKS_PER_SEC;

    // clock_t start_switch = clock();
    // while (i < NUM_CYCLES)
    // {
            // test_chip8_switch_cycle(&chip8);
            // i++;
    // }
    // clock_t end_switch = clock();
    // double switch_elapsed_time = ((double)(end_switch - start_switch)) / CLOCKS_PER_SEC;

    printf("Function pointer table execution time: %f seconds\n", ptr_elapsed_time);
    // printf("Switch statement execution time: %f seconds\n", switch_elapsed_time);
    // printf("Average time per cycle: %f seconds\n", ptr_elapsed_time / NUM_CYCLES);
}
