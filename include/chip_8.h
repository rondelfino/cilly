#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdint.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

#define NUM_REGISTERS 16

#define MAX_MEMORY 4096
#define STACK_SIZE 16

#define START_ADDRESS 0x200
#define FONT_START_ADDR 0x50

typedef struct Chip_8_State
{
    uint8_t memory[MAX_MEMORY];

    uint32_t display[DISPLAY_HEIGHT][DISPLAY_WIDTH];

    /*
     * I register
     * stores memory addresses to be used in operations
     */
    uint16_t I;

    /*
     * program counter
     * holds the address for the next instruction
     */
    uint16_t PC;

    /*
     * stack
     * pushes PC when subroutine is called
     * popped when it returns
     */
    uint16_t stack[STACK_SIZE];

    /*
     * stack pointer
     * points to top of stack
     * stores addresses that the interpreter should return to when finished with a
     * subroutine
     */
    uint8_t SP;

    /*
     * delay timer register
     */
    uint8_t delay_timer;
    uint8_t sound_timer;

    /*
     * general purpose registers
     * V0 -> VF
     */
    uint8_t V[NUM_REGISTERS];

    uint16_t opcode;
} chip_8;

void chip_8_init(chip_8 *chip_8, uint16_t pc_start_address);

void chip_8_load_rom(chip_8 *chip_8, const char *filename);

void chip_8_load_font(chip_8 *chip_8);

void chip_8_execute_instruction(chip_8 *chip_8);

void chip_8_instruction_cycle(chip_8 *chip_8);


#endif // !CHIP_8_H
