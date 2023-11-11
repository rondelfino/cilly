#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Time interval between each cycle at 60 hz in microseconds
 * used for timers */
#define SIXTY_HZU (1.0 / 60.0) * 1000000

void chip8_init(struct chip8 *chip8, uint16_t pc_start_address)
{
    /* Init chip8 fields:
        Clear display
        Clear stack
        Clear registers V0-VF
        Clear memory */
    // memset(chip8->stack, 0, STACK_SIZE * sizeof(uint16_t));
    memset(chip8->V, 0, REGISTER_COUNT * sizeof(uint8_t));
    // memset(chip8->memory, 0, MAX_MEMORY * sizeof(uint8_t));
    memset(chip8->keypad, 0, KEY_COUNT * sizeof(uint8_t));
    chip8_clear_display(chip8);

    chip8->I = 0;
    chip8->SP = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->draw_flag = 0;
    chip8->delay_timer_acc = 0;
    chip8->sound_timer_acc = 0;

    // memset(chip8, 0, sizeof(struct chip8));
    chip8->PC = pc_start_address;

    chip8_load_fontset(chip8);

    /* Init seed */
    srand(time(NULL));
    // printf("Seed: %ld\n", time(NULL));

    /* Setup function pointer tables */
    // chip8->main_table[0x0] = table0;
    // chip8->main_table[0x1] = op_1NNN;
    // chip8->main_table[0x2] = op_2NNN;
    // chip8->main_table[0x3] = op_3XNN;
    // chip8->main_table[0x4] = op_4XNN;
    // chip8->main_table[0x5] = op_5XY0;
    // chip8->main_table[0x6] = op_6XNN;
    // chip8->main_table[0x7] = op_7XNN;
    // chip8->main_table[0x8] = table8;
    // chip8->main_table[0x9] = op_9XY0;
    // chip8->main_table[0xA] = op_ANNN;
    // chip8->main_table[0xB] = op_BNNN;
    // chip8->main_table[0xC] = op_CXNN;
    // chip8->main_table[0xD] = op_DXYN;
    // chip8->main_table[0xE] = tableE;
    // chip8->main_table[0xF] = tableF;

    // for (size_t i = 0; i <= 0xE; i++)
    // {
    //     chip8->table0[i] = op_NULL;
    //     chip8->table8[i] = op_NULL;
    //     chip8->tableE[i] = op_NULL;
    // }

    // chip8->table0[0x0] = op_00E0;
    // chip8->table0[0xE] = op_00EE;

    // chip8->table8[0x0] = op_8XY0;
    // chip8->table8[0x1] = op_8XY1;
    // chip8->table8[0x2] = op_8XY2;
    // chip8->table8[0x3] = op_8XY3;
    // chip8->table8[0x4] = op_8XY4;
    // chip8->table8[0x5] = op_8XY5;
    // chip8->table8[0x6] = op_8XY6;
    // chip8->table8[0x7] = op_8XY7;
    // chip8->table8[0xE] = op_8XYE;

    // chip8->tableE[0x1] = op_EXA1;
    // chip8->tableE[0xE] = op_EX9E;

    // for (size_t i = 0; i <= 0x65; i++)
    // {
    //     chip8->tableF[i] = op_NULL;
    // }

    // chip8->tableF[0x07] = op_FX07;
    // chip8->tableF[0x0A] = op_FX0A;
    // chip8->tableF[0x15] = op_FX15;
    // chip8->tableF[0x18] = op_FX18;
    // chip8->tableF[0x1E] = op_FX1E;
    // chip8->tableF[0x29] = op_FX29;
    // chip8->tableF[0x33] = op_FX33;
    // chip8->tableF[0x55] = op_FX55;
    // chip8->tableF[0x65] = op_FX65;
}

void chip8_load_rom(struct chip8 *chip8, const char *filename)
{
    FILE *rom = fopen(filename, "rb");

    if (rom)
    {
        /* Find the ROM file size */
        fseek(rom, 0L, SEEK_END);
        uint64_t rom_size = ftell(rom);
        fseek(rom, 0L, SEEK_SET);

        if (rom_size > 0)
        {
            if (rom_size <= MAX_MEMORY)
            {
                /* Pointer to buffer starting at start address
                   Size of the object to be stored
                   Count of the object
                   The stream/source to read */
                fread(chip8->memory + chip8->PC, 1, rom_size, rom);
            }
            else
            {
                /* Handle ROM too large for memory */
                printf("Error: ROM size exceeds memory bounds.\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            /* Handle empty ROM file */
            printf("Error: ROM file is empty.\n");
            exit(EXIT_FAILURE);
        }
        fclose(rom);
    }
    else
    {
        /* Handle file opening error */
        printf("Error: Failed to open the ROM file.\n");
        exit(EXIT_FAILURE);
    }
}

void chip8_load_fontset(struct chip8 *chip8)
{
    /* Represents hex characters as 5x4 sprites
     * Each byte represents a row for its repsective hex character
     * F:
     * 11110000
     * 10000000
     * 11110000
     * 10000000
     * 10000000 */
    uint8_t fontset[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    uint8_t size = sizeof(fontset);
    for (uint8_t i = 0; i < 80; i++)
    {
        chip8->memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
}

void chip8_reset_released_keys(struct chip8 *chip8)
{
    for (uint8_t key = 0; key < KEY_COUNT; key++)
    {
        if (chip8->keypad[key] == 2)
            chip8->keypad[key] = 0;
    }
}

void chip8_cycle(struct chip8 *chip8)
{
    /* Fetch opcode */
    uint16_t opcode = chip8->memory[chip8->PC];
    opcode <<= 8;
    opcode |= chip8->memory[chip8->PC + 1];
    printf("0x%04x\n", opcode);

    uint8_t group_id = (opcode >> 12) & 0xF;

    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t n = opcode & 0xF;
    uint8_t nn = opcode & 0xFF;
    uint16_t address = opcode & 0xFFF;

    /* Point to next opcode */
    chip8->PC += 2;

    /* Decode and execute */
    // chip8->main_table[(chip8->opcode >> 12) & 0xF]();
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
            if (y_pos + row >= DISPLAY_HEIGHT)
                break;

            /* Extract byte at current row */
            uint8_t sprite_byte = chip8->memory[chip8->I + row];

            /* Iterate over extracted byte */
            for (uint8_t col = 0; col < 8; col++)
            {
                if (x_pos + col >= DISPLAY_WIDTH)
                    break;

                /* Extract current bit */
                uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 0x1;
                /* Get current pixel value from display buffer */

                /* Extracted bit from sprite is set */
                if (sprite_pixel)
                {
                    /* Current pixel from display is set */
                    if (chip8->display[((y_pos + row) * DISPLAY_WIDTH) + (x_pos + col)])
                        /* Collision: set VF to 1 */
                        chip8->V[0xF] = 1;

                    /* Set display pixel if it's currently 0, or unset it if it's currently 1 */
                    chip8->display[((y_pos + row) * DISPLAY_WIDTH) + (x_pos + col)] ^= 1;
                }

                /* Stop drawing if right edge is reached */
            }
            /* Stop drawing if bottom edge is reached */
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

    // printf("0x%04x\n", chip8->opcode);

    /* Decrement by 1, 60 times per second */
    /* if dt > 1/60th of a second, decrement timers */
    // if (chip8->delay_timer > 0)
    // {
    //     chip8->delay_timer_acc += dt;
    //     if (dt >= SIXTY_HZU)
    //     {
    //         chip8->delay_timer--;
    //         chip8->delay_timer_acc = 0;
    //     }
    // }

    // if (chip8->sound_timer > 0)
    // {
    //     chip8->sound_timer_acc += dt;
    //     if (dt >= SIXTY_HZU)
    //     {
    //         printf("BEEP!\n");
    //         chip8->sound_timer--;
    //         chip8->sound_timer_acc = 0;
    //     }
    // }
}

void chip8_clear_display(struct chip8 *chip8)
{
    memset(chip8->display, 0, (DISPLAY_WIDTH * DISPLAY_HEIGHT) * sizeof(uint8_t));
}
