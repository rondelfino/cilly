#include "chip8.h"
#include <stdint.h>
#include <stdio.h>

void chip8_init(struct chip8 *chip8, uint16_t pc_start_address)
{
    chip8->PC = pc_start_address;

    /* Setup function pointer tables */
    chip8->main_table[0x0] = table0;
    chip8->main_table[0x1] = op_1NNN;
    chip8->main_table[0x2] = op_2NNN;
    chip8->main_table[0x3] = op_3XNN;
    chip8->main_table[0x4] = op_4XNN;
    chip8->main_table[0x5] = op_5XY0;
    chip8->main_table[0x6] = op_6XNN;
    chip8->main_table[0x7] = op_7XNN;
    chip8->main_table[0x8] = table8;
    chip8->main_table[0x9] = op_9XY0;
    chip8->main_table[0xA] = op_ANNN;
    chip8->main_table[0xB] = op_BNNN;
    chip8->main_table[0xC] = op_CXNN;
    chip8->main_table[0xD] = op_DXYN;
    chip8->main_table[0xE] = tableE;
    chip8->main_table[0xF] = tableF;

    chip8->table0[0x0] = op_00E0;
    chip8->table0[0xE] = op_00EE;

    chip8->table8[0x0] = op_8XY0;
    chip8->table8[0x1] = op_8XY1;
    chip8->table8[0x2] = op_8XY2;
    chip8->table8[0x3] = op_8XY3;
    chip8->table8[0x4] = op_8XY4;
    chip8->table8[0x5] = op_8XY5;
    chip8->table8[0x6] = op_8XY6;
    chip8->table8[0x7] = op_8XY7;
    chip8->table8[0xE] = op_8XYE;

    chip8->tableE[0x1] = op_EXA1;
    chip8->tableE[0xE] = op_EX9E;

    chip8->tableF[0x7] = op_FX07;
    chip8->tableF[0xA] = op_FX0A;
    chip8->tableF[0x15] = op_FX15;
    chip8->tableF[0x18] = op_FX18;
    chip8->tableF[0x1E] = op_FX1E;
    chip8->tableF[0x29] = op_FX29;
    chip8->tableF[0x33] = op_FX33;
    chip8->tableF[0x55] = op_FX55;
    chip8->tableF[0x65] = op_FX65;

    // Clear display
    // Clear stack
    // Clear registers V0-VF
    // Clear memory

    chip8_load_fontset(chip8);
}

void chip8_load_rom(struct chip8 *chip8, const char *filename)
{
    FILE *rom = fopen(filename, "rb");

    if (rom)
    {
        // Find the ROM file size
        fseek(rom, 0L, SEEK_END);
        long rom_size = ftell(rom);
        fseek(rom, 0L, SEEK_SET);

        if (rom_size > 0)
        {
            if (rom_size <= sizeof(chip8->memory))
            {
                fread(chip8->memory + START_ADDRESS, MAX_MEMORY - START_ADDRESS, 1, rom);
            }
            else
            {
                // Handle ROM too large for memory
                printf("Error: ROM size exceeds memory bounds.\n");
            }
        }
        else
        {
            // Handle empty ROM file
            printf("Error: ROM file is empty.\n");
        }
        fclose(rom);
    }
    else
    {
        // Handle file opening error
        printf("Error: Failed to open the ROM file.\n");
    }
}

void chip8_load_fontset(struct chip8 *chip8)
{
    /*
    Represents hex characters as 5x4 sprites
    Each byte represents a row for its repsective hex character
      F:
      11110000
      10000000
      11110000
      10000000
      10000000
    */
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

    for (int i = 0; i <= sizeof(fontset); i++)
    {
        chip8->memory[FONT_START_ADDR + i] = fontset[i];
    }
}

uint16_t chip8_fetch_opcode(struct chip8 chip8)
{
    uint16_t opcode = chip8.memory[chip8.PC];
    opcode <<= 8;
    opcode |= chip8.memory[chip8.PC + 1];

    return opcode;
}

// void chip8_decode(struct chip8 *chip8)
// {
// }

void chip8_cycle(struct chip8 *chip8)
{
   
    /* fetch instruction */
    uint16_t opcode = chip8_fetch_opcode(*chip8);

    /* point to next opcode */
    chip8->PC += 2;
    printf("opcode: 0x%x\n", opcode);

    /* first nibble */
    uint8_t opcode_id = (opcode >> 12) & 0xF;
    printf("first_nibble: 0x%x\n", opcode_id);
    /* second nibble*/
    chip8->x = (opcode >> 8) & 0xF;
    printf("x: 0x%x\n", chip8->x);
    /* third nibble*/
    chip8->y = (opcode >> 4) & 0xF;
    printf("y: 0x%x\n", chip8->y);
    /* fourth nibble */
    chip8->n = opcode & 0xF;
    printf("n: 0x%x\n", chip8->n);
    /* third and fourth nibbles*/
    chip8->nn = chip8->y << 4 | chip8->n;
    printf("nn: 0x%x\n", chip8->nn);
    /* second, third, and fourth nibbles */
    chip8->nnn = chip8->x << 8 | chip8->y << 4 | chip8->n;
    printf("nnn: 0x%x\n", chip8->nnn);

    /* decode */
    /* handle stuff */

    // Decrement by 1 60 times per second
    if (chip8->delay_timer > 0)
        chip8->delay_timer--;

    if (chip8->sound_timer > 0)
    {
        printf("BEEP!\n");
        chip8->sound_timer--;
    }
}

/* opcode function tables */
void table0(struct chip8 *chip8)
{
    chip8->table0[chip8->n]();
}

void table8(struct chip8 *chip8)
{
}
void tableE(struct chip8 *chip8)
{
}
void tableF(struct chip8 *chip8)
{
}

/* opcodes */
void op_1NNN()
{
}
void op_2NNN()
{
}
void op_3XNN()
{
}
void op_4XNN()
{
}
void op_5XY0()
{
}
void op_6XNN()
{
}
void op_7XNN()
{
}
void op_9XY0()
{
}
void op_ANNN()
{
}
void op_BNNN()
{
}
void op_CXNN()
{
}
void op_DXYN()
{
}

void op_00E0()
{
}
void op_00EE()
{
}
void op_8XY0()
{
}
void op_8XY1()
{
}
void op_8XY2()
{
}
void op_8XY3()
{
}
void op_8XY4()
{
}
void op_8XY5()
{
}
void op_8XY6()
{
}
void op_8XY7()
{
}
void op_8XYE()
{
}

void op_EXA1()
{
}
void op_EX9E()
{
}

void op_FX07()
{
}
void op_FX0A()
{
}
void op_FX15()
{
}
void op_FX18()
{
}
void op_FX1E()
{
}
void op_FX29()
{
}
void op_FX33()
{
}
void op_FX55()
{
}
void op_FX65()
{
}