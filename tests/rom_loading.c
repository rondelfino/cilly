#include "chip_8.h"
#include <stdio.h>

int main(int argc, char **argv) {
  struct Chip_8_State *chip_8;

  chip_8_load_rom(chip_8, argv[1]);

  for (int i = START_ADDRESS; i < START_ADDRESS + 20; i++) {
    printf("Address: 0x%x 0x%x\n", i, chip_8->memory[i]);
  }
  return 0;
}
