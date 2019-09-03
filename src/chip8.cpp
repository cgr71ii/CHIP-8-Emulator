#include "cpu.h"

#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Hello Emulator World" << std::endl;

    Chip8_CPU chip8_cpu;

    chip8_cpu.print_memory();

    return 0;
}