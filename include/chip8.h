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

/* opcode function pointer type for tables */
typedef void (*opcode_fn)();

struct chip8
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
     * pushes PC when subroutine (function) is called
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
    uint8_t V[REGISTER_COUNT];

    /* entire opcode */
    uint16_t opcode;

    /* nibbles */
    /*
        x -> second nibble used to lookup one of the registers V[x] (V[0]->V[F])
        y -> third nibble used to lookup one of the registers V[y] (V[0]->V[F])
        n -> fourth nibble. 4-bit number
        nn -> third and fourth nibbles (2nd byte). 8-bit immediate number
        nnn -> second, third, and fourth nibbles. 12-bit immediate memory address
    */
    uint8_t x, y, n, nn, nnn;

    uint8_t keypad[KEY_COUNT];

    /* opcode function pointer tables 
        the opcode id (the first nibble) indexes into the corresponding table
        If it's 0, 8, E, or F, the id will be used to index into the corresponding table through the main_table
        i.e.
            opcode = 0x8XY7, id = 8
            index into main_table using '8'

            main_table[0x8] = table8();

            call table8();
            table8() indexes into table8[] using the fourth nibble (opcodes starting with 8 only differ by the fourth nibble)

            the function for the opcode is then called
            table8[0x7] = op_8XY7();
    */
    opcode_fn main_table[0xF + 1];
    opcode_fn table0[0xE + 1];
    opcode_fn table8[0xE + 1];
    opcode_fn tableE[0xE + 1];
    opcode_fn tableF[0x65 + 1];
};


void chip8_init(struct chip8 *chip8, uint16_t pc_start_address);

void chip8_load_rom(struct chip8 *chip8, const char *filename);

void chip8_load_fontset(struct chip8 *chip8);

/* idk if i should make this */
uint16_t chip8_fetch_opcode(struct chip8 chip8);

void chip8_decode_and_execute(struct chip8 *chip8, opcode_fn opcode_table);

void chip8_execute_instruction(struct chip8 *chip8);

void chip8_cycle(struct chip8 *chip8);


/* opcode functions */
/* handles opcodes starting with 0 */
void table0(struct chip8 *chip8);
/* handles opcodes starting with 8 */
void table8(struct chip8 *chip8);
/* handles opcodes starting with E */
void tableE(struct chip8 *chip8);
/* handles opcodes starting with F */
void tableF(struct chip8 *chip8);

void op_1NNN();
void op_2NNN();
void op_3XNN();
void op_4XNN();
void op_5XY0();
void op_6XNN();
void op_7XNN();
void op_9XY0();
void op_ANNN();
void op_BNNN();
void op_CXNN();
void op_DXYN();

void op_00E0();
void op_00EE();
void op_8XY0();
void op_8XY1();
void op_8XY2();
void op_8XY3();
void op_8XY4();
void op_8XY5();
void op_8XY6();
void op_8XY7();
void op_8XYE();

void op_EXA1();
void op_EX9E();

void op_FX07();
void op_FX0A();
void op_FX15();
void op_FX18();
void op_FX1E();
void op_FX29();
void op_FX33();
void op_FX55();
void op_FX65();

#endif // !chip8_H