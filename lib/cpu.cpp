
#include "cpu.h"

#include <iostream>
#include <string.h>

Chip8_CPU::Chip8_CPU()
{
    this->opcode = 0;
    this->I = 0;
    this->pc = 0x200;
    this->delay_timer = 0;
    this->sound_timer = 0;
    this->sp = 0;

    memset(this->memory, 0, Chip8_CPU::MEMORY_LENGTH_B);
    memset(this->V, 0, Chip8_CPU::GENERAL_PURPOSE_REGISTERS);
    memset(this->gfx, 0, Chip8_CPU::WIDTH * Chip8_CPU::HEIGHT);
    memset(this->stack, 0, Chip8_CPU::STACK_DEEPNESS);
    memset(this->key, 0, Chip8_CPU::KEY_MAPPING_SIZE);
}

#ifdef CHIP8_CPU_DEBUG

void Chip8_CPU::print_memory()
{
    std::cout << " --- BEGIN MEMORY --- " << std::endl;

    for (size_t i = 0; i < Chip8_CPU::MEMORY_LENGTH_B; i++)
    {
        std::cout << (unsigned)this->memory[i] << " ";

        if ((i + 1) % 32 == 0)
        {
            std::cout << " <- {8 * [" << (i % 32) + 0 + 4 * (i / 32) - 31 << " | " 
                                      << (i % 32) + 1 + 4 * (i / 32) - 31 << " | "
                                      << (i % 32) + 2 + 4 * (i / 32) - 31 << " | "
                                      << (i % 32) + 3 + 4 * (i / 32) - 31
                                      << "]}";

            std::cout << "\n";
        }
        else if ((i + 1) % 8 == 0)
        {
            std:: cout << "| ";
        }
    }

    std::cout << " --- END MEMORY --- " << std::endl;
}

void Chip8_CPU::print_status()
{
    std::cout << "General purpose registers:" << std::endl;
    
    for (size_t i = 0; i < Chip8_CPU::GENERAL_PURPOSE_REGISTERS; i++)
    {
        std::cout << " V[" << i << "] = " << (unsigned)this->V[i] << std::endl;
    }

    std::cout << "--------------------------" << std::endl;
    std::cout << "Register I = " << (unsigned)this->I << std::endl;
    std::cout << "Current opcode = " << (unsigned)this->opcode << std::endl;
    std::cout << "Program Counter = " << (unsigned)this->pc << std::endl;
    std::cout << "Delay timer = " << (unsigned)this->delay_timer << std::endl;
    std::cout << "Sound timer = " << (unsigned)this->sound_timer << std::endl;
    std::cout << "Stack Pointer = " << (unsigned)this->sp << std::endl;
    std::cout << "Stack:" << std::endl;

    for (size_t i = 0; i < Chip8_CPU::STACK_DEEPNESS; i++)
    {
        std::cout << "stack[" << i << "] = " << (unsigned)this->stack[i] << std::endl;
    }

    std::cout << "------" << std::endl;
    std::cout << "Key mapping:" << std::endl;

    for (size_t i = 0; i < Chip8_CPU::KEY_MAPPING_SIZE; i++)
    {
        std::cout << "key[" << i << "] = " << (unsigned)this->key[i] << std::endl;
    }
    
    std::cout << "------------" << std::endl;
}

#endif