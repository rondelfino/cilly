# cilly
> A CHIP8 interpreter
*Currently only compatible with games written in the original CHIP8 instruction set*
## Build From Source
### Building Prequisites
- gcc or clang
- SDL2
- Make
### Compiling
Compile in debug mode:
```
make
```
Or compile in release mode:
```
make release=1 
```
The default compiler is gcc. If you want to use clang:
```
make CC=clang
```
### Usage
*only bash commands shown for brevity*
```
./bin/[OS]/[build mode]/cilly [clock speed in Hz] [path/to/rom]
```
## TODO
- [x] Wayland/Win32 or SDL
- [Quirks](https://chip8.gulrak.net/#quirk11)
- [ ] Accurate COSMAC-VIP support ([opcode timings](https://jackson-s.me/2019/07/13/Chip-8-Instruction-Scheduling-and-Frequency.html))
- [ ] SCHIP 1.1 support
- [ ] XO-CHIP support
    - These platforms will automatically set the respective quirks
- [ ] Flags to change change/set quirks manually
- [ ] Debugger
- [ ] GUI rom loading, setting clock, manually setting quirks, and choosing platform

## Resources
- [Benchmark Program](https://github.com/Milk-Cool/chip8-benchmark)
- https://github.com/mattmikolay/chip-8/wiki/Mastering-CHIP%E2%80%908#chip-8-instructions
- https://johnearnest.github.io/Octo/docs/SuperChip.html
- https://chip-8.github.io/links/#testing
- https://github.com/JohnEarnest/Octo
- https://github.com/tobiasvl/awesome-chip-8
- https://chip-8.github.io/extensions/#chip-8

