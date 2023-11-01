#pragma once

#ifndef chip8_H
#define chip8_H

#include <stdint.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

#define KEY_COUNT 16
#define REGISTER_COUNT 16

#define MAX_MEMORY 4096
#define STACK_SIZE 16

#define START_ADDRESS 0x200
#define FONT_START_ADDR 0x50

/* Opcode function pointer type for opcode tables */
typedef void (*opcode_func)();

/* State the CHIP8 machine */
struct chip8
{
    /* Memory:
       Default number of unique addresses is 2^12(4096) in a CHIP8 interpreter because it used 12-bit addressing */
    uint8_t memory[MAX_MEMORY];

    uint8_t display[DISPLAY_HEIGHT][DISPLAY_WIDTH];

    /* I register:
      Stores memory addresses to be used in operations */
    uint16_t I;

    /* Program counter:
      Holds the memory address for the next instruction */
    uint16_t PC;

    /* Stack:
      Pushes PC when subroutine (function) is called,
      pops when it returns */
    uint16_t stack[STACK_SIZE];

    /* Stack pointer:
      Points to top of stack
      Stores memory addresses that the interpreter should return to when finished with a
      subroutine */
    uint8_t SP;

    uint8_t delay_timer; /* Delay timer register */
    uint8_t sound_timer; /* Sound timer register */

    uint8_t V[REGISTER_COUNT]; /* General purpose registers: V0 -> VF */

    uint16_t opcode; /* Entire opcode */

    /* Nibbles */
    /*  X -> second nibble used to lookup one of the registers VX (V0->VF)
        Y -> third nibble used to lookup one of the registers VY (V0->VF)
        N -> fourth nibble. 4-bit number
        NN -> third and fourth nibbles (2nd byte). 8-bit immediate number
        NNN -> second, third, and fourth nibbles. 12-bit immediate memory address */
    uint8_t x, y, n, nn, nnn;

    uint8_t keypad[KEY_COUNT];

    /*  Opcode function pointer tables
        the opcode id (the first nibble) indexes into the corresponding table
        If it's 0, 8, E, or F, the id will be used to index into the corresponding table through the main_table
        i.e.
            opcode = 0x8XY7, id = 8
            index into main_table using '8'

            main_table[0x8] = table8;

            call table8();
            table8() indexes into table8[] using the fourth nibble (opcodes starting with 8 only differ by the fourth
       nibble)

            the function for the opcode is then called
            table8[0x7] = op_8XY7(); */
    opcode_func main_table[0xF + 1];
    opcode_func table0[0xE + 1];
    opcode_func table8[0xE + 1];
    opcode_func tableE[0xE + 1];
    opcode_func tableF[0x65 + 1];
};

/* Initializes CHIP8 state
   @param pc_start_address Set memory address where the game is located; default is COSMAC-VIP at 0x200 */
void chip8_init(struct chip8 *chip8, uint16_t pc_start_address);

/* Load a ROM to memory
   @param filename Name or path of a compatible *.ch8 ROM */
void chip8_load_rom(struct chip8 *chip8, const char *filename);

/* Load default fontset to memory */
void chip8_load_fontset(struct chip8 *chip8);

/* void chip8_decode_and_execute_opcode(struct chip8 *chip8, opcode_func main_table); */

/* CHIP8 instruction cycle. Also handle sound and delay timers. */
void chip8_cycle(struct chip8 *chip8);

/* Set all pixels on screen to 0 */
void chip8_clear_display(struct chip8 *chip8);

/* Opcode functions */

/* Handle opcodes starting with 0 */
void table0(struct chip8 *chip8);
/* Handle opcodes starting with 8 */
void table8(struct chip8 *chip8);
/* Handle opcodes starting with E */
void tableE(struct chip8 *chip8);
/* Handle opcodes starting with F */
void tableF(struct chip8 *chip8);

/* Jump to memory address NNN */
void op_1NNN(struct chip8 *chip8);
/* Call subroutine at memory address NNN */
void op_2NNN(struct chip8 *chip8);
/* Skip the following instruction if the value of register VX equals NN (PC += 2), otherwise do nothing */
void op_3XNN(struct chip8 *chip8);
/* Skip the following instruction if the value of register VX is not equal to NN (PC+= 2), otherwise do nothing */
void op_4XNN(struct chip8 *chip8);
/* Skip the following instruction if the value of register VX is equal to the value of register VY (PC += 2), otherwise
 * do nothing */
void op_5XY0(struct chip8 *chip8);
/* Set VX to NN */
void op_6XNN(struct chip8 *chip8);
/* Add NN to VX */
void op_7XNN(struct chip8 *chip8);
/* Skip the following instruction if the value of register VX is not equal to the value of register VY */
void op_9XY0(struct chip8 *chip8);
/* Set the index register, I, to memory address NNN */
void op_ANNN(struct chip8 *chip8);
/* Jump to memory address NNN plus V0 - COSMAC-VIP
   Jump to memory address XNN plus VX - SUPER-CHIP */
void op_BNNN(struct chip8 *chip8);

/* Set VX to a random number with a mask of NN (random number AND NN) */
void op_CXNN(struct chip8 *chip8);

/* Draw a sprite at position (VX, VY) with N bytes of sprite data starting at the address stored
   in I. Set VF to 1 if any set pixels are changed to unset, and 0 otherwise */
void op_DXYN(struct chip8 *chip8);

/* Clear the screen */
void op_00E0(struct chip8 *chip8);
/* Returns from subroutine */
void op_00EE(struct chip8 *chip8);
/* Set VX to value in VY */
void op_8XY0(struct chip8 *chip8);
/* Set VX to VX OR VY */
void op_8XY1(struct chip8 *chip8);
/* Set VX to VX AND VY */
void op_8XY2(struct chip8 *chip8);
/* Set VX to VX XOR VY */
void op_8XY3(struct chip8 *chip8);

/* Add the value of register VY to register VX. Set VF to 1 if carry occurs (overflow), otherwise set VF to 0 */
void op_8XY4(struct chip8 *chip8);

/* Subtract the value of register VY from register VX. Set VF to 0 if a borrow occur (underflow), otherwise set VF to 1
 */
void op_8XY5(struct chip8 *chip8);

/* Changes depending on interpreter
  COSMAC-VIP: Set VX to VY.
  All other interpreters: Don't set VX to VY. */

/* Store the value of register VY shifted right one bit in register VX. Set register VF to the least significant bit
 * prior to the shift */
void op_8XY6(struct chip8 *chip8);

/* Set register VX to the value of VY minus VX. Set VF to 0 if a borrow occurs (underflow), otherwise set VF to 1 */
void op_8XY7(struct chip8 *chip8);

/* Changes depending on interpreter
  COSMAC-VIP: Set VX to VY.
  All other interpreters: Don't set VX to VY. */

/* Store the value of register VY shifted left one bit in register VX. Set register VF to the most significant bit prior
 * to the shift */
void op_8XYE(struct chip8 *chip8);

/* Skip the following instruction if the key corresponding to the hex value currently stored in register VX is not pressed */
void op_EXA1(struct chip8 *chip8);
/* Skip the following instruction if the key corresponding to the hex value currently stored in register VX is pressed */
void op_EX9E(struct chip8 *chip8);

/* Store the current value of the delay timer in register VX */
void op_FX07(struct chip8 *chip8);
/* Wait for a keypress and store the result in register VX */
void op_FX0A(struct chip8 *chip8);
/* Set the delay timer to the value of register VX */
void op_FX15(struct chip8 *chip8);
/* Set the sound timer to the value of register VX */
void op_FX18(struct chip8 *chip8);
/* Add the value stored in register VX to register I */
void op_FX1E(struct chip8 *chip8);
/* Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX */
void op_FX29(struct chip8 *chip8);
/* Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I+1, and I+2 */
void op_FX33(struct chip8 *chip8);
/* Store the values of registers V0 to VX inclusive in memory starting at address I. I is set to I + X + 1 after operation */
/* COSMAC-VIP: I gets set to I + X + 1 after storing the values in memory */
void op_FX55(struct chip8 *chip8);
/* Fill registers V0 to VX inclusive with the values stored in memory starting at address I. I is set to I + X + 1 after operation */
/* COSMAC-VIP: I gets set to I + X + 1 after loading the values from memory */
void op_FX65(struct chip8 *chip8);

#endif // !chip8_H
