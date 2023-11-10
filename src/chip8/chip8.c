#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// #define SIXTY_HZ (1.0 / 60.0) * 1000

void chip8_init(struct chip8 *chip8, uint16_t pc_start_address)
{
    /* Init chip8 fields:
        Clear display
        Clear stack
        Clear registers V0-VF
        Clear memory */
    // chip8_clear_display(chip8);
    memset(chip8, 0, sizeof(struct chip8));
    // memset(chip8->stack, 0x0000, STACK_SIZE * sizeof(uint16_t));
    // memset(chip8->V, 0x00, REGISTER_COUNT * sizeof(uint8_t));
    // memset(chip8->memory, 0x00, MAX_MEMORY * sizeof(uint8_t));
    // memset(chip8->keypad, 0x00, KEY_COUNT * sizeof(uint8_t));
    // chip8->I = 0x00;
    // chip8->SP = 0;
    // chip8->delay_timer = 0;
    // chip8->sound_timer = 0;
    // chip8->draw_flag = 0;
    // chip8_load_fontset(chip8);
    chip8->PC = pc_start_address;

    /* Init seed */
    srand(time(NULL));
    printf("Seed: %ld\n", time(NULL));

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

    for (size_t i = 0; i <= 0xE; i++)
    {
        chip8->table0[i] = op_NULL;
        chip8->table8[i] = op_NULL;
        chip8->tableE[i] = op_NULL;
    }

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

    for (size_t i = 0; i <= 0x65; i++)
    {
        chip8->tableF[i] = op_NULL;
    }

    chip8->tableF[0x7] = op_FX07;
    chip8->tableF[0xA] = op_FX0A;
    chip8->tableF[0x15] = op_FX15;
    chip8->tableF[0x18] = op_FX18;
    chip8->tableF[0x1E] = op_FX1E;
    chip8->tableF[0x29] = op_FX29;
    chip8->tableF[0x33] = op_FX33;
    chip8->tableF[0x55] = op_FX55;
    chip8->tableF[0x65] = op_FX65;
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
            if (rom_size <= MAX_MEMORY - START_ADDRESS)
            {
                /* Pointer to buffer starting at start address
                   Size of the object to be stored
                   Count of the object
                   The stream/source to read */
                fread(chip8->memory + START_ADDRESS, MAX_MEMORY - START_ADDRESS, 1, rom);
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
        // Handle file opening error
        printf("Error: Failed to open the ROM file.\n");
        exit(EXIT_FAILURE);
    }
}

void chip8_load_fontset(struct chip8 *chip8)
{
    /* Represents hex characters as 5x4 sprites
    Each byte represents a row for its repsective hex character
      F:
      11110000
      10000000
      11110000
      10000000
      10000000 */
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
    for (uint8_t i = 0; i < size; i++)
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
    chip8->opcode = chip8->memory[chip8->PC];
    chip8->opcode <<= 8;
    chip8->opcode |= chip8->memory[chip8->PC + 1];

    /* Point to next opcode */
    if (chip8->PC >= START_ADDRESS && chip8->PC <= 0xFFF)
        chip8->PC += 2;
    else
    {
        printf("Error: Invalid memory access at address 0x%04X\n", chip8->PC);
        exit(EXIT_FAILURE);
    }

    /* Decode and execute */
    chip8->main_table[(chip8->opcode >> 12) & 0xF]();
    // printf("Executing opcode 0x%04X at memory address 0x%04X\n", chip8->opcode, chip8->PC);

    // printf("0x%04x\n", chip8->opcode);

    /* Decrement by 1, 60 times per second */
    /* if dt > 1/60th of a second, decrement timers */
    // if (chip8->delay_timer > 0)
    //     chip8->delay_timer--;

    // if (chip8->sound_timer > 0)
    // {
    // printf("BEEP!\n");
    // chip8->sound_timer--;
    // }

    /* Set released keys to idle */
    chip8_reset_released_keys(chip8);
}

void chip8_clear_display(struct chip8 *chip8)
{
    memset(chip8->display, 0, (DISPLAY_WIDTH * DISPLAY_HEIGHT) * sizeof(uint8_t));
}
