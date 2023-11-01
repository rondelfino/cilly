#include "chip8.h"
#include <stdint.h>
#include <stdlib.h>

/* Opcode function tables */
void table0(struct chip8 *chip8)
{
    /* Index into table using fourth nibble */
    chip8->table0[chip8->opcode & 0xF]();
}

void table8(struct chip8 *chip8)
{
    chip8->table8[chip8->opcode & 0xF]();
}

void tableE(struct chip8 *chip8)
{
    chip8->tableE[chip8->opcode & 0xF]();
}
void tableF(struct chip8 *chip8)
{
    /* Index into table using 3rd, 4th nibble */
    chip8->tableF[chip8->opcode & 0xFF]();
}

/* Opcodes */
void op_1NNN(struct chip8 *chip8)
{
    uint16_t address = chip8->opcode & 0xFFF;
    chip8->PC = address;
}
void op_2NNN(struct chip8 *chip8)
{
    uint16_t address = chip8->opcode & 0xFFF;
    chip8->stack[chip8->SP++] = chip8->PC;
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

    if (chip8->V[x] == nn)
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
    uint8_t nn = chip8->opcode & 0xFF;
    uint8_t x = (chip8->opcode >> 8) & 0xF;

    chip8->V[x] += nn;
}
void op_9XY0(struct chip8 *chip8)
{
}
void op_ANNN(struct chip8 *chip8)
{
    uint16_t nnn = chip8->opcode & 0xFFF;

    chip8->I = nnn;
}
void op_BNNN(struct chip8 *chip8)
{
    /*
         uint8_t x = (chip8->opcode >> 8) & 0xF;
         uint16_t xnn = ((chip8->opcode >> 8) & 0xF) << 8) | chip8->opcode & 0xFF;
         chip8->PC = xnn + chip8->V[x];
     */
    /* TODO(nael): support legacy config/quirks */
    uint16_t nnn = chip8->opcode & 0xFFF;

    chip8->PC = nnn + chip8->V[0];
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
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;
    uint8_t n = chip8->opcode & 0xF;

    /* Cap at max width and height to prevent drawing out of bounds */
    uint8_t x_pos = chip8->V[x] % DISPLAY_WIDTH;
    uint8_t y_pos = chip8->V[y] % DISPLAY_HEIGHT;

    chip8->V[REGISTER_COUNT - 1] = 0;

    for (size_t row = 0; row < n; row++)
    {
        /* If bottom edge is reached stop drawing this column */
        if (x_pos + row >= DISPLAY_WIDTH)
            break;

        uint8_t sprite_byte = chip8->memory[chip8->I + row];
        for (size_t col = 0; col < 8; col++)
        {
            uint8_t sprite_pixel = (sprite_byte >> (8 - col - 1)) & 0xF;
            uint8_t *display_pixel = &chip8->display[y_pos + row][x_pos + col];

            /* If right edge is reached stop drawing this row */
            if (x_pos + col >= DISPLAY_HEIGHT)
                break;

            /* Collision */
            if (sprite_pixel)
            {
                if (*display_pixel)
                    chip8->V[REGISTER_COUNT - 1] = 1;
            }
            *display_pixel ^= 1;
        }
    }
}

void op_00E0(struct chip8 *chip8)
{
    chip8_clear_display(chip8);
}
void op_00EE(struct chip8 *chip8)
{
    chip8->SP--;
    chip8->stack[chip8->SP] = chip8->PC;
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
}
void op_8XY2(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] &= chip8->V[y];
}
void op_8XY3(struct chip8 *chip8)
{

    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] ^= chip8->V[y];
}
void op_8XY4(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    uint16_t sum = chip8->V[x] + chip8->V[y];
    chip8->V[REGISTER_COUNT - 1] = (sum > 0xFF) ? 1 : 0;
    /* cap at 0xFF (255) */
    chip8->V[x] = sum & 0xFF;
}
void op_8XY5(struct chip8 *chip8)
{

    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;
    chip8->V[REGISTER_COUNT - 1] = (chip8->V[x] > chip8->V[y]) ? 1 : 0;

    chip8->V[x] -= chip8->V[y];
}
void op_8XY6(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    /* COSMAC-VIP */
    /* TODO(nael): support legacy config/quirks */
    chip8->V[x] = chip8->V[y];

    chip8->V[REGISTER_COUNT - 1] = chip8->V[x] & 0x1;
    chip8->V[x] >>= 1;
}
void op_8XY7(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;
    chip8->V[REGISTER_COUNT - 1] = (chip8->V[y] > chip8->V[x]) ? 1 : 0;

    chip8->V[x] = chip8->V[y] - chip8->V[x];
}
void op_8XYE(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    /* COSMAC-VIP */
    /* TODO(nael): support legacy config/quirks */
    // chip8->V[x] = chip8->V[y];

    chip8->V[REGISTER_COUNT - 1] = (chip8->V[x] >> 12) & 0x1;
    chip8->V[x] <<= 1;
}

void op_EXA1(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t key = chip8->V[x];

    if (!chip8->keypad[key])
        chip8->PC += 2;
}
void op_EX9E(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t key = chip8->V[x];

    if (chip8->keypad[key])
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

    if (chip8->keypad[0])
    {
        chip8->V[x] = 0x0;
    }
    else if (chip8->keypad[1])
    {
        chip8->V[x] = 0x1;
    }
    else if (chip8->keypad[2])
    {
        chip8->V[x] = 0x2;
    }
    else if (chip8->keypad[3])
    {
        chip8->V[x] = 0x3;
    }
    else if (chip8->keypad[4])
    {
        chip8->V[x] = 0x4;
    }
    else if (chip8->keypad[5])
    {
        chip8->V[x] = 0x5;
    }
    else if (chip8->keypad[6])
    {
        chip8->V[x] = 0x6;
    }
    else if (chip8->keypad[7])
    {
        chip8->V[x] = 0x7;
    }
    else if (chip8->keypad[8])
    {
        chip8->V[x] = 0x8;
    }
    else if (chip8->keypad[9])
    {
        chip8->V[x] = 0x9;
    }
    else if (chip8->keypad[10])
    {
        chip8->V[x] = 0xA;
    }
    else if (chip8->keypad[11])
    {
        chip8->V[x] = 0xB;
    }
    else if (chip8->keypad[11])
    {
        chip8->V[x] = 0xC;
    }
    else if (chip8->keypad[11])
    {
        chip8->V[x] = 0xD;
    }
    else if (chip8->keypad[11])
    {
        chip8->V[x] = 0xE;
    }
    else if (chip8->keypad[11])
    {
        chip8->V[x] = 0xF;
    }
    else
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
}
void op_FX29(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    /* Hexadecimal digit in V[X] */
    uint8_t digit = chip8->V[x];

    /* Set I to the memory address of the digit */
    /* 5 is the size of a row in the fontset */
    chip8->I = FONT_START_ADDR + (5 * digit);
}
void op_FX33(struct chip8 *chip8)
{
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t value = chip8->V[x];

    size_t i_counter = 0;
    while (value > 0)
    {
        uint8_t digit = value % 10;
        chip8->memory[chip8->I + i_counter++] = digit;
        value /= 10;
    }
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
    /* chip8->I += x + 1; */
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
    /* chip8->I += x + 1; */
}
