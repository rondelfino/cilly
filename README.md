# cilly
> A CHIP8 interpreter

*Currently only compatible with games written in the original CHIP8 instruction set*
## Build From Source
### Building Prequisites
<details>
    <summary>Windows</summary>

- [MSYS2 MinGW Toolchain](https://www.msys2.org/)
- gcc or clang
- SDL2
- Make

## How to use MSYS to build
After installation, launch the environment for your chosen compiler and target architecture, and install the listed packages.
| Compiler | Architecture | Environment | Packages |
| -------- | ------------ | ----------- | -------- |
| gcc  |  32-bit  | MINGW32 | `pacman -S mingw-w64-i686-gcc mingw-w64-i686-SDL2`|
| gcc  |  64-bit  |  UCRT64 | `pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-SDL2`|
| clang | 32-bit | CLANG32 | `pacman -S mingw-w64-clang-i686-clang mingw-w64-clang-i686-SDL2`|
| clang | 64-bit | CLANG64/UCRT64 | UCRT64: </br> `pacman -S mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-SDL2`</br> CLANG64: </br> `pacman -S mingw-w64-x86_64-clang mingw-w64-clang-x86_64-SDL2`|

Also install git and make:
```
pacman -S git make
```
</details>

<details>
    <summary>Linux</summary>    

- gcc or clang
- SDL2
- Make

</details>

### Compiling
Clone the repo and navigate to the directory:
```
git clone https://github.com/rondelfino/cilly.git
cd cilly
```
Default compilation uses gcc, and is in debug 64-bit mode:
```
make
```
Compile in release mode:
```
make release=1 
```
Compile in 32-bit mode:
```
make arch=32
```
If you want to use clang:
```
make CC=clang
```
### Usage
```
./bin/[OS]/[build-mode]/cilly [clock-speed-in-Hz] [path/to/rom]
```
Or:
```
make run [clock-speed-in-hz] [path/to/rom]
```

`make help` to list available commands.
## TODO
- [x] Wayland/Win32 or SDL
- [Quirks](https://chip8.gulrak.net/#quirk11)
- [ ] Makefile: add option to change packaged executable destination
- [ ] Accurate COSMAC-VIP support ([opcode timings](https://jackson-s.me/2019/07/13/Chip-8-Instruction-Scheduling-and-Frequency.html))
- [ ] SCHIP 1.1 support
- [ ] XO-CHIP support
    - These platforms will automatically set the respective quirks
- [ ] Flags to change change/set quirks manually
- [ ] Debugger
- [ ] GUI for rom loading, setting clock, manually setting quirks, and choosing platform

## Resources
- [Benchmark Program](https://github.com/Milk-Cool/chip8-benchmark)
- https://github.com/mattmikolay/chip-8/wiki/Mastering-CHIP%E2%80%908#chip-8-instructions
- https://johnearnest.github.io/Octo/docs/SuperChip.html
- https://chip-8.github.io/links/#testing
- https://github.com/JohnEarnest/Octo
- https://github.com/tobiasvl/awesome-chip-8
- https://chip-8.github.io/extensions/#chip-8

