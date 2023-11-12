#pragma once

#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

#define KEY_COUNT 16
#define REGISTER_COUNT 16

#define MAX_MEMORY 4096
#define STACK_SIZE 16

#define START_ADDRESS 0x200
#define FONTSET_START_ADDRESS 0x50

typedef enum
{
    Pressed,
    Released,
    Idle
} keystate;

struct chip8
{
    /* Default number of unique addresses is 2^12(4096) in a CHIP8 interpreter because it used 12-bit addressing */
    uint8_t memory[MAX_MEMORY];

    uint8_t display[DISPLAY_HEIGHT * DISPLAY_WIDTH];

    /* Index register; store memory address to be used in operations */
    uint16_t I;

    /* Program counter; store memory address for the next instruction */
    uint16_t PC;

    /* Push PC address when subroutine (function) is called,
     * pop when it returns */
    uint16_t stack[STACK_SIZE];
    uint8_t SP; /* Point to top of stack */

    uint8_t delay_timer;      /* Delay timer register */
    uint8_t sound_timer;      /* Sound timer register */
    uint64_t delay_timer_acc; /* Tracks change in time to decrement delay timer at 60 Hz */
    uint64_t sound_timer_acc; /* Tracks change in time to decrement sound timer at 60 Hz */

    uint8_t V[REGISTER_COUNT]; /* General purpose registers: V0 -> VF */

    // uint16_t opcode; /* Current opcode to be decoded and executed */

    uint8_t keypad[KEY_COUNT]; /* Set to 0 if idle, 1 if key is pressed, 2 if key is released */

    uint8_t draw_flag; /* Update screen when not 0 */
};

/* Initializes CHIP8 state
 * @param pc_start_address Set memory address where the game is located; default is COSMAC-VIP at 0x200 */
void chip8_init(struct chip8 *chip8, uint16_t pc_start_address);
/* Load a ROM to memory
 * @param filename Name or path of a compatible *.ch8 ROM */
void chip8_load_rom(struct chip8 *chip8, const char *filename);
/* Load default fontset to memory */
void chip8_load_fontset(struct chip8 *chip8);
/* Emulate CHIP8 instruction cycle
 * @param dt */
void chip8_cycle(struct chip8 *chip8);
/* Set all pixels on screen to 0 */
void chip8_clear_display(struct chip8 *chip8);
/* Set all keys to idle/0 */
void chip8_reset_released_keys(struct chip8 *chip8);

#endif // !chip8_H
