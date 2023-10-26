#include "chip_8.h"
#include <stdio.h>

void chip_8_init(chip_8 *chip_8, uint16_t pc_start_address)
{
    chip_8->PC = START_ADDRESS;
}

void chip_8_load_rom(chip_8 *chip_8, const char *filename)
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
            if (rom_size <= sizeof(chip_8->memory))
            {
                fread(chip_8->memory + START_ADDRESS, MAX_MEMORY - START_ADDRESS, 1, rom);
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

void chip_8_load_font(chip_8 *chip_8)
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
        chip_8->memory[FONT_START_ADDR + i] = fontset[i];
    }
}

void chip_8_instruction_cycle(chip_8 *chip_8)
{
    /* fetch instruction */
    uint16_t opcode = chip_8->memory[chip_8->PC];
    opcode <<= 8;
    opcode |= chip_8->memory[chip_8->PC + 1];
    chip_8->PC += 2;
    printf("opcode: 0x%x\n", opcode);

    /* first nibble*/
    uint8_t first_nibble = (opcode >> 12) & 0xF;
    printf("first_nibble: 0x%x\n", first_nibble);
    /* second nibble*/
    uint8_t x = (opcode >> 8) & 0xF;
    printf("x: 0x%x\n", x);
    /* third nibble*/
    uint8_t y = (opcode >> 4) & 0xF;
    printf("y: 0x%x\n", y);
    /* fourth nibble */
    uint8_t n = opcode & 0xF;
    printf("n: 0x%x\n", n);
    /* third and fourth nibbles*/
    uint8_t nn = y << 4 | n;
    printf("nn: 0x%x\n", nn);
    /* second, third, and fourth nibbles */
    uint16_t nnn = x << 8 | y << 4 | n;
    printf("nnn: 0x%x\n", nnn);

}