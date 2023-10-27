#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  struct chip8 *chip8 = calloc(0, sizeof(struct chip8));
  if (chip8 == NULL)
    printf("kill me");

  chip8_init(chip8, 0x200);
  chip8_load_rom(chip8, argv[1]);
  chip8_cycle(chip8);

  for (int i = START_ADDRESS; i < START_ADDRESS + 20; i++) {
    printf("Address: 0x%x 0x%x\n", i, chip8->memory[i]);
  }
  return 0;
}
