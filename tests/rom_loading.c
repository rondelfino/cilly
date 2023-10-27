#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  struct chip8 chip8; 

  chip8_init(&chip8, 0x200);
  chip8_load_rom(&chip8, "../roms/1dcell.ch8");
  // printf("PC: 0x%x 0x%x\n", chip8->PC, chip8->memory[chip8->PC]);
  chip8_cycle(&chip8);
  printf("PC: 0x%x\n", chip8.PC);

  

  for (int i = START_ADDRESS; i < START_ADDRESS + 20; i++) {
    printf("Address: 0x%x 0x%x\n", i, chip8.memory[i]);
  }
  return 0;
}
