
#include <iostream>

#include "cpu.h"

int main()
{
    #ifndef CHIP8_CPU_DEBUG

    std::cerr << "Preprocessor directive CHIP8_CPU_DEBUG is not set and is necessary for execute the test. Aborting." << std::endl;

    return -1;

    #else

    CPU cpu;

    cpu.initializate();

    cpu.store(0x0000, 0xE7);
    cpu.execute_instruction(0x1111);

    cpu.print_status();
    cpu.print_memory();
    cpu.print_screen();

    #endif

    return 0;
}