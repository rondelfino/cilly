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
    /* Init chip8 fields */
    memset(chip8, 0, sizeof(struct chip8));
    chip8->PC = pc_start_address;
    chip8_load_fontset(chip8);

    /* Init seed */
    srand(time(NULL));
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

void chip8_decode_and_execute(struct chip8 *chip8, uint16_t opcode)
{
    uint8_t instruction_group = (opcode >> 12) & 0xF;

    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t n = opcode & 0xF;
    uint8_t nn = opcode & 0xFF;
    uint16_t address = opcode & 0xFFF;

    /* Decode and execute */
    switch (instruction_group)
    {
    case 0x0:
        switch (nn)
        {
        /* 00E0:
         * Clear the screen */
        case 0xE0:
            chip8_clear_display(chip8);
            break;

        /* 00EE:
         * Return from subroutine */
        case 0xEE:
            chip8->SP--;
            chip8->PC = chip8->stack[chip8->SP];
            break;
        }
        break;

    /* 1NNN:
     * Jump to memory address */
    case 0x1:
        chip8->PC = address;
        break;

    /* 2NNN:
     * Call subroutine at memory address */
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

    /* 3XNN:
     * Skip the following instruction
     * if the value of register VX equals NN (PC += 2),
     * otherwise do nothing */
    case 0x3:
        if (chip8->V[x] == nn)
            chip8->PC += 2;
        break;

    /* 4XNN:
     * Skip the following instruction
     * if the value of register VX is not equal to NN (PC += 2),
     * otherwise do nothing */
    case 0x4:
        if (chip8->V[x] != nn)
            chip8->PC += 2;
        break;

    /* 5XY0:
     * Skip the following instruction
     * if the value of register VX equal to the value of register VY (PC += 2),
     * otherwise do nothing */
    case 0x5:
        if (chip8->V[x] == chip8->V[y])
            chip8->PC += 2;
        break;

    /* 6XNN:
     * Set VX to NN */
    case 0x6:
        chip8->V[x] = nn;
        break;

    /* 7XNN:
     * Add NN to VX */
    case 0x7:
        chip8->V[x] += nn;
        break;

    case 0x8:
        switch (n)
        {
        /* 8XY0:
         * Set VX to the value in VY */
        case 0x0:
            chip8->V[x] = chip8->V[y];
            break;

        /* 8XY1:
         * Set VX to VX OR VY */
        case 0x1:
            chip8->V[x] |= chip8->V[y];
            /* original chip8 quirk */
            chip8->V[0xF] = 0;
            break;

        /* 8XY2:
         * Set VX to VX AND VY */
        case 0x2:
            chip8->V[x] &= chip8->V[y];
            /* original chip8 quirk */
            chip8->V[0xF] = 0;
            break;

        /* 8XY3:
         * Set VX to VX XOR VY */
        case 0x3:
            chip8->V[x] ^= chip8->V[y];
            /* original chip8 quirk */
            chip8->V[0xF] = 0;
            break;

        /* 8XY4:
         * Add the value of register VY to register VX.
         * Set VF to 1 if carry occurs (overflow),
         * otherwise set VF to 0 */
        case 0x4: {
            uint16_t sum = chip8->V[x] + chip8->V[y];
            chip8->V[x] = sum;
            chip8->V[0xF] = (sum >= 0xFF) ? 1 : 0;
        }
        break;

        /* 8XY5:
         * Subtract the value of register VY from register VX.
         * Set VF to 0 if a borrow occur (underflow), otherwise set VF to 1 */
        case 0x5: {
            uint8_t temp = chip8->V[x];

            chip8->V[x] -= chip8->V[y];
            /* If there's no borrow */
            chip8->V[0xF] = (temp >= chip8->V[y]) ? 1 : 0;
        }
        break;

        /* 8XY6:
         * COSMAC-VIP: Set VX to VY
         * Store the value of register VY shifted right one bit in register VX.
         * Set register VF to the least significant bit prior to the shift */
        case 0x6: {
            /* COSMAC-VIP */
            chip8->V[x] = chip8->V[y];
            uint8_t temp = chip8->V[x];

            chip8->V[x] >>= 1;
            chip8->V[0xF] = temp & 0x1;
        }
        break;

        /* 8XY7:
         * Set register VX to the value of VY minus VX.
         *  Set VF to 0 if a borrow occurs (underflow),
         * otherwise set VF to 1 */
        case 0x7:
            chip8->V[x] = chip8->V[y] - chip8->V[x];
            /* If there's no borrow */
            chip8->V[0xF] = (chip8->V[y] >= chip8->V[x]) ? 1 : 0;
            break;

        /* 8XYE:
         * COSMAC-VIP: Set VX to VY.
         * Store the value of register VY shifted left one bit in register VX.
         * Set register VF to the most significant bit prior to the shift */
        case 0xE: {
            /* COSMAC-VIP specific */
            chip8->V[x] = chip8->V[y];

            uint8_t temp = chip8->V[x];

            chip8->V[x] <<= 1;
            chip8->V[0xF] = temp >> 7;
        }
        break;
        }
        break;

    /* 9XY0:
     * Skip the following instruction if the value of register VX
     * is not equal to the value of register VY */
    case 0x9:
        if (chip8->V[x] != chip8->V[y])
            chip8->PC += 2;
        break;

    /* ANNN:
     * Set the index register, I, to memory address NNN */
    case 0xA:
        chip8->I = address;
        break;

    /* BNNN:
     * COSMAC-VIP: Jump to memory address NNN plus V0
     * SCHIP: Jump to memory address XNN plus VX */
    case 0xB: {
        /* uint8_t x = (chip8->opcode >> 8) & 0xF;
              uint16_t xnn = ((chip8->opcode >> 8) & 0xF) << 8) | chip8->opcode & 0xFF;
              chip8->PC = xnn + chip8->V[x];*/
        chip8->PC = address + chip8->V[0];
    }
    break;

    /* CXNN:
     * Set VX to a random number with a mask of NN (random number AND NN) */
    case 0xC: {
        uint8_t r = rand();

        chip8->V[x] = r & nn;
    }
    break;

    /* DXYN:
     * Draw a sprite at position (VX, VY) with N bytes of sprite data
     * starting at the address storedin I.
     * Set VF to 1 if any set pixels are changed to unset, and 0 otherwise */
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
        /* EXA1:
         * Skip the following instruction if the key corresponding to the hex value
         * currently stored in register VX is not pressed  */
        case 0xA1: {
            uint8_t key = chip8->V[x];

            if (chip8->keypad[key] != 1)
                chip8->PC += 2;
        }
        break;

        /* EX9E:
         * Skip the following instruction if the key corresponding to the hex value
         * currently stored in register VX is pressed */
        case 0x9E: {
            uint8_t key = chip8->V[x];

            if (chip8->keypad[key] == 1)
                chip8->PC += 2;
        }
        break;
        }
        break;

    case 0xF:
        switch (nn)
        {
        /* FX07:
         * Store the current value of the delay timer in register VX */
        case 0x07:
            chip8->V[x] = chip8->delay_timer;
            break;

        /* FX0A:
         * Wait for a keypress and store the result in register VX */
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

        /* FX15:
         * Set the delay timer to the value of register VX */
        case 0x15:
            chip8->delay_timer = chip8->V[x];
            break;

        /* FX18:
         * Set the sound timer to the value of register VX */
        case 0x18:
            chip8->sound_timer = chip8->V[x];
            break;

        /* FX1E:
         * Add the value stored in register VX to register I */
        case 0x1E:
            chip8->I += chip8->V[x];
            chip8->V[0xF] = chip8->I + chip8->V[x] > 0xFFF ? 1 : 0;
            break;

        /* FX29:
         * Set I to the memory address of the sprite data
         * corresponding to the hexadecimal digit stored in register VX */
        case 0x29: {
            /* Hexadecimal digit in V[X] */
            uint8_t digit = chip8->V[x];

            /* Set I to the memory address of the digit */
            /* 5 is the size of a row in the fontset */
            chip8->I = FONTSET_START_ADDRESS + (5 * digit);
        }
        break;

        /* FX33:
         * Store the binary-coded decimal equivalent of the value
         * stored in register VX at addresses I, I+1, and I+2 */
        case 0x33: {
            uint8_t value = chip8->V[x];

            chip8->memory[chip8->I + 2] = value % 10;
            value /= 10;

            chip8->memory[chip8->I + 1] = value % 10;
            value /= 10;

            chip8->memory[chip8->I] = value % 10;
        }
        break;

        /* FX55:
         * COSMAC-VIP: I gets set to I + X + 1 after storing the values in memory
         * Store the values of registers V0 to VX inclusive in memory starting at address I.
         * I is set to I + X + 1 after operation */
        case 0x55:
            for (size_t i = 0; i <= x; i++)
            {
                chip8->memory[chip8->I + i] = chip8->V[i];
            }
            /* COSMAC-VIP specific */
            chip8->I += x + 1;
            break;

        /* FX65:
         * COSMAC-VIP: I gets set to I + X + 1 after loading the values from memory
         * Fill registers V0 to VX inclusive with the values stored in memory
         * starting at address I. I is set to I + X + 1 after operation */
        case 0x65:

            for (size_t i = 0; i <= x; i++)
            {
                chip8->V[i] = chip8->memory[chip8->I + i];
            }
            /* TODO(nael): support legacy config/quirks */
            /* COSMAC-VIP specific */
            chip8->I += x + 1;
            break;
        }
        break;

    default:
        /* Unknown opcode */
        fprintf(stderr, "Fatal Error: Opcode 0x%04x is unknown or incompatible. Please load a compatible rom.\n",
                opcode);
        exit(EXIT_FAILURE);
        break;
    }
}

void chip8_cycle(struct chip8 *chip8)
{
    /* Fetch opcode */
    uint16_t opcode = chip8->memory[chip8->PC];
    opcode <<= 8;
    opcode |= chip8->memory[chip8->PC + 1];

    /* Point to next opcode */
    chip8->PC += 2;

    chip8_decode_and_execute(chip8, opcode);

    /* Set released keys to idle */
    chip8_reset_released_keys(chip8);
}

void chip8_clear_display(struct chip8 *chip8)
{
    memset(chip8->display, 0, (DISPLAY_WIDTH * DISPLAY_HEIGHT) * sizeof(uint8_t));
}
