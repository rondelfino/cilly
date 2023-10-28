#include "chip8.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void chip8_init(struct chip8 *chip8, uint16_t pc_start_address)
{
    chip8->PC = pc_start_address;
    memset(chip8->stack, 0, STACK_SIZE);
    chip8->I = 0x0;

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

    chip8->tableF[0x07] = op_FX07;
    chip8->tableF[0x0A] = op_FX0A;
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

void chip8_cycle(struct chip8 *chip8)
{

    /* fetch instruction */
    chip8->opcode = chip8->memory[chip8->PC];
    chip8->opcode <<= 8;
    chip8->opcode |= chip8->memory[chip8->PC + 1];

    /* point to next chip8->opcode */
    chip8->PC += 2;
    printf("chip8->opcode: 0x%x\n", chip8->opcode);

    /* first nibble */
    uint8_t opcode_id = (chip8->opcode >> 12) & 0xF;
    printf("first_nibble: 0x%x\n", opcode_id);
    /* second nibble*/
    chip8->x = (chip8->opcode >> 8) & 0xF;
    printf("x: 0x%x\n", chip8->x);
    /* third nibble*/
    chip8->y = (chip8->opcode >> 4) & 0xF;
    printf("y: 0x%x\n", chip8->y);
    /* fourth nibble */
    chip8->n = chip8->opcode & 0xF;
    printf("n: 0x%x\n", chip8->n);
    /* third and fourth nibbles*/
    chip8->nn = chip8->opcode & 0xFF;
    printf("nn: 0x%x\n", chip8->nn);
    /* second, third, and fourth nibbles */
    uint16_t nnn = chip8->opcode & 0xFFF;
    printf("nnn: 0x%x\n", nnn);

    /* decode */
    chip8->main_table[opcode_id]();
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

void chip8_clear_display(struct chip8 *chip8)
{
    for (size_t i = 0; i < DISPLAY_WIDTH; i++)
    {
        for (size_t j = 0; j < DISPLAY_HEIGHT; j++)
        {
            chip8->display[i][j] = 0;
        }
    }
}

/* chip8->opcode function tables */
void table0(struct chip8 *chip8)
{
    /* index into table using fourth nibble */
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
    /* index into table using 3rd, 4th nibble */
    chip8->tableF[chip8->opcode & 0xFF]();
}

/* chip8->opcodes */
void op_1NNN(struct chip8 *chip8)
{
    /* jumps (sets PC) to memory address NNN */
    uint16_t address = chip8->opcode & 0xFFF;
    chip8->PC = address;
}
void op_2NNN(struct chip8 *chip8)
{
    /* calls subroutine at memory location NNN */
    uint16_t address = chip8->opcode & 0xFFF;
    chip8->stack[chip8->SP++] = chip8->PC;
    chip8->PC = address;
}
void op_3XNN(struct chip8 *chip8)
{
    /* skip instruction if value of register V[X] == NN
    (PC += 2),
    or do nothing*/
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t nn = chip8->opcode & 0xFF;

    if (chip8->V[x] == nn)
        chip8->PC += 2;
}
void op_4XNN(struct chip8 *chip8)
{
    /* skip instruction if value of register V[X] == NN
    (PC += 2),
    or do nothing*/
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t nn = chip8->opcode & 0xFF;

    if (chip8->V[x] == nn)
        chip8->PC += 2;
}
void op_5XY0(struct chip8 *chip8)
{
    /* skip instruction if value of register V[X] == V[Y]
    (PC += 2),
    or do nothing*/
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    if (chip8->V[x] == chip8->V[y])
        chip8->PC += 2;
}
void op_6XNN(struct chip8 *chip8)
{
    /* sets V[x] to NN */
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t nn = chip8->opcode & 0xFF;

    chip8->V[x] = nn;
}
void op_7XNN(struct chip8 *chip8)
{
    /* adds nn to V[x] */
    uint8_t nn = chip8->opcode & 0xFF;
    uint8_t x = (chip8->opcode >> 8) & 0xF;

    chip8->V[x] += nn;
}
void op_9XY0(struct chip8 *chip8)
{
}
void op_ANNN(struct chip8 *chip8)
{
    /* Sets the index register, I, to NNN */
    uint16_t nnn = chip8->opcode & 0xFFF;

    chip8->I = nnn; 
}
void op_BNNN(struct chip8 *chip8)
{
    
}
void op_CXNN(struct chip8 *chip8)
{
}
void op_DXYN(struct chip8 *chip8)
{
}

void op_00E0(struct chip8 *chip8)
{
    chip8_clear_display(chip8);
}
void op_00EE(struct chip8 *chip8)
{
    /* returns from subroutine */
    chip8->SP--;
    chip8->stack[chip8->SP] = chip8->PC;
}

void op_8XY0(struct chip8 *chip8)
{
    /* set V[x] to V[y]*/
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] = chip8->V[y];
}
void op_8XY1(struct chip8 *chip8)
{
    /* set V[X] to V[X] | V[Y]*/
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] |= chip8->V[y];
}
void op_8XY2(struct chip8 *chip8)
{
    /* set V[X] to V[X] & V[Y]*/
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] &= chip8->V[y];
}
void op_8XY3(struct chip8 *chip8)
{
    /* set V[X] to V[X] ^ V[Y]*/
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    chip8->V[x] ^= chip8->V[y];
}
void op_8XY4(struct chip8 *chip8)
{
    /*
        set V[X] to V[X] + V[Y]
        if it overflows (255) set V[F] to 1
        otherwise set V[F] to 0
    */
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    uint16_t sum = chip8->V[x] + chip8->V[y];
    chip8->V[REGISTER_COUNT - 1] = (sum > 0xFF) ? 1 : 0;
    /* cap at 0xFF (255) */
    chip8->V[x] = sum & 0xFF;
}
void op_8XY5(struct chip8 *chip8)
{
    /*
        set V[X] to V[X] - V[Y]
        if it undeflows set V[F] to 0
        otherwise set V[F] to 1
    */
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;
    chip8->V[REGISTER_COUNT - 1] = (chip8->V[x] > chip8->V[y]) ? 1 : 0;

    chip8->V[x] -= chip8->V[y];
}
void op_8XY6(struct chip8 *chip8)
{
    /*
        Changes depending on interpreter
        COSMAC-VIP: set V[X] to V[Y]
        All other interpreters: don't set V[X] to V[Y]

        Shift the value of V[X] one bit to the right
        Set V[F] to 1 if that bit was 1
                    0 if that bit was 0
     */
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
    /*
        Set V[X] to V[Y] - V[X]
        If it undeflows set V[F] to 0,
        otherwise set V[F] to 1
    */
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;
    chip8->V[REGISTER_COUNT - 1] = (chip8->V[y] > chip8->V[x]) ? 1 : 0;

    chip8->V[x] = chip8->V[y] - chip8->V[x];
}
void op_8XYE(struct chip8 *chip8)
{
    /*
        Changes depending on interpreter
        COSMAC-VIP: set V[X] to V[Y]
        All other interpreters: don't set V[X] to V[Y]

        Shift the value of V[X] one bit to the right
        Set V[F] to 1 if that bit was 1
                    0 if that bit was 0
     */
    uint8_t x = (chip8->opcode >> 8) & 0xF;
    uint8_t y = (chip8->opcode >> 4) & 0xF;

    /* COSMAC-VIP */
    /* TODO(nael): support legacy config/quirks */
    chip8->V[x] = chip8->V[y];

    chip8->V[REGISTER_COUNT - 1] = (chip8->V[x] >> 12) & 0x1;
    chip8->V[x] <<= 1;
}
void op_EXA1(struct chip8 *chip8)
{
}
void op_EX9E(struct chip8 *chip8)
{
}

void op_FX07(struct chip8 *chip8)
{
}
void op_FX0A(struct chip8 *chip8)
{
}
void op_FX15(struct chip8 *chip8)
{
}
void op_FX18(struct chip8 *chip8)
{
}
void op_FX1E(struct chip8 *chip8)
{
}
void op_FX29(struct chip8 *chip8)
{
}
void op_FX33(struct chip8 *chip8)
{
}
void op_FX55(struct chip8 *chip8)
{
}
void op_FX65(struct chip8 *chip8)
{
}