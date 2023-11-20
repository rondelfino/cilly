#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>

/* Opcode function tables */
void table0(struct chip8 *chip8)
{
    /* Index into table using fourth nibble */
    chip8->table0[chip8->opcode & 0xF](chip8);
}
void table8(struct chip8 *chip8)
{
    chip8->table8[chip8->opcode & 0xF](chip8);
}
void tableE(struct chip8 *chip8)
{
    chip8->tableE[chip8->opcode & 0xF](chip8);
}
void tableF(struct chip8 *chip8)
{
    /* Index into table using 3rd, 4th nibble */
    chip8->tableF[chip8->opcode & 0xFF](chip8);
}

/* Opcodes */
void op_NULL(struct chip8 *chip8)
{
    printf("Fatal Error: Opcode 0x%04x is unknown or incompatible with the chosen platform. Please load a compatible "
           "rom.\n",
           chip8->opcode);
    exit(EXIT_FAILURE);
}
void op_00E0(struct chip8 *chip8)
{
    chip8_clear_display(chip8);
}
void op_00EE(struct chip8 *chip8)
{
    chip8->SP--;
    chip8->PC = chip8->stack[chip8->SP];
}
void op_1NNN(struct chip8 *chip8)
{
    uint16_t address = chip8->opcode & 0xFFF;

    chip8->PC = address;
}
void op_2NNN(struct chip8 *chip8)
{
    uint16_t address = chip8->opcode & 0xFFF;

    if (chip8->SP >= STACK_SIZE)
    {
        // Debug information for stack overflow
        printf("Stack overflow detected. Terminating program.\n");
        exit(EXIT_FAILURE);
    }

    chip8->stack[chip8->SP] = chip8->PC;
    chip8->SP++;
    chip8->PC = address;
}
void op_3XNN(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t nn = chip8->opcode & 0xFF;

    if (chip8->V[x] == nn)
        chip8->PC += 2;
}
void op_4XNN(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t nn = chip8->opcode & 0xFF;

    if (chip8->V[x] != nn)
        chip8->PC += 2;
}
void op_5XY0(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    if (chip8->V[x] == chip8->V[y])
        chip8->PC += 2;
}
void op_6XNN(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t nn = chip8->opcode & 0xFF;

    chip8->V[x] = nn;
}
void op_7XNN(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t nn = chip8->opcode & 0xFF;

    chip8->V[x] += nn;
}

void op_8XY0(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] = chip8->V[y];
}
void op_8XY1(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] |= chip8->V[y];
    /* legacy */
    chip8->V[0xF] = 0;
}
void op_8XY2(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] &= chip8->V[y];
    /* legacy */
    chip8->V[0xF] = 0;
}
void op_8XY3(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] ^= chip8->V[y];
    /* legacy */
    chip8->V[0xF] = 0;
}
void op_8XY4(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    uint16_t sum = chip8->V[x] + chip8->V[y];
    /* cap at 0xFF (255) */
    chip8->V[x] = sum;
    chip8->V[0xF] = (sum >= 0xFF) ? 1 : 0;
}
void op_8XY5(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;
    uint8_t temp = chip8->V[x];

    chip8->V[x] -= chip8->V[y];
    /* If there's no borrow */
    chip8->V[0xF] = (temp >= chip8->V[y]) ? 1 : 0;
}
void op_8XY6(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    /* COSMAC-VIP */
    /* TODO(nael): support legacy config/quirks */
    chip8->V[x] = chip8->V[y];
    uint8_t temp = chip8->V[x];

    chip8->V[x] >>= 1;
    chip8->V[0xF] = temp & 0x1;
}
void op_8XY7(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] = chip8->V[y] - chip8->V[x];
    /* If there's no borrow */
    chip8->V[0xF] = (chip8->V[y] >= chip8->V[x]) ? 1 : 0;
}
void op_8XYE(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    /* COSMAC-VIP */
    /* TODO(nael): support legacy config/quirks */
    chip8->V[x] = chip8->V[y];
    uint8_t temp = chip8->V[x];

    chip8->V[x] <<= 1;
    chip8->V[0xF] = temp >> 7;
}

void op_9XY0(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    if (chip8->V[x] != chip8->V[y])
        chip8->PC += 2;
}
void op_ANNN(struct chip8 *chip8)
{
    uint16_t address = chip8->opcode & 0xFFF;

    chip8->I = address;
}
void op_BNNN(struct chip8 *chip8)
{
    /* uint8_t x = (chip8->opcode >> 8) & 0xF;
      uint16_t xnn = ((chip8->opcode >> 8) & 0xF) << 8) | chip8->opcode & 0xFF;
      chip8->PC = xnn + chip8->V[x];*/
    /* TODO(nael): support legacy config/quirks */
    uint16_t address = chip8->opcode & 0xFFF;

    chip8->PC = address + chip8->V[0];
}
void op_CXNN(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t nn = chip8->opcode & 0xFF;
    uint8_t r = rand();

    chip8->V[x] = r & nn;
}
void op_DXYN(struct chip8 *chip8)
{
    chip8->draw_flag = 1;
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;
    uint8_t n = chip8->opcode & 0xF;

    /* Cap to the dimensions of the display buffer */
    uint8_t x_pos = chip8->V[x] % DISPLAY_WIDTH;
    uint8_t y_pos = chip8->V[y] % DISPLAY_HEIGHT;

    chip8->V[0xF] = 0;
    /* Draw sprite from address in I to N */
    for (size_t row = 0; row < n; row++)
    {
        /* Stop drawing if bottom edge is reached */
        if (y_pos + row >= DISPLAY_HEIGHT)
            break;

        /* Extract byte at current row */
        uint8_t sprite_byte = chip8->memory[chip8->I + row];

        /* Iterate over extracted byte */
        for (size_t col = 0; col < 8; col++)
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
}

void op_EXA1(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t key = chip8->V[x];

    if (chip8->keypad[key] != 1)
        chip8->PC += 2;
}
void op_EX9E(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t key = chip8->V[x];

    if (chip8->keypad[key] == 1)
        chip8->PC += 2;
}

void op_FX07(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;

    chip8->V[x] = chip8->delay_timer;
}
void op_FX0A(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
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
void op_FX15(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;

    chip8->delay_timer = chip8->V[x];
}
void op_FX18(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;

    chip8->sound_timer = chip8->V[x];
}
void op_FX1E(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;

    chip8->I += chip8->V[x];
    chip8->V[0xF] = chip8->I + chip8->V[x] > 0xFFF ? 1 : 0;
}
void op_FX29(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    /* Hexadecimal digit in V[X] */
    uint8_t digit = chip8->V[x];

    /* Set I to the memory address of the digit */
    /* 5 is the size of a row in the fontset */
    chip8->I = FONTSET_START_ADDRESS + (5 * digit);
}
void op_FX33(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t value = chip8->V[x];

    chip8->memory[chip8->I + 2] = value % 10;
    value /= 10;

    chip8->memory[chip8->I + 1] = value % 10;
    value /= 10;

    chip8->memory[chip8->I] = value % 10;
}
void op_FX55(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;

    for (size_t i = 0; i <= x; i++)
    {
        chip8->memory[chip8->I + i] = chip8->V[i];
    }
    /* TODO(nael): support legacy config/quirks */
    /* COSMAC-VIP specific */
    chip8->I += x + 1;
}
void op_FX65(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;

    for (size_t i = 0; i <= x; i++)
    {
        chip8->V[i] = chip8->memory[chip8->I + i];
    }
    /* TODO(nael): support legacy config/quirks */
    /* COSMAC-VIP specific */
    chip8->I += x + 1;
}
