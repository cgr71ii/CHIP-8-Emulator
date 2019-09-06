#include "cpu.h"

#include <iostream>
#include <string>

void setup_graphics()
{

}

void setup_input()
{

}

void draw_graphics()
{

}

int main(int argc, char** argv)
{
    CPU chip8_cpu;
    std::string rom_path("");

    setup_graphics();
    setup_input();

    chip8_cpu.initializate();

    std::cout << "Type the ROM you want to load: ";
    //std::cin >> rom_path; // It doesn't allow spaces
    std::getline(std::cin, rom_path);   // It allows spaces

    bool loaded = chip8_cpu.load_rom(rom_path);

    if (loaded)
    {
        std::cout << "ROM loaded successfully!" << std::endl;
    }
    else
    {
        std::cerr << "Couldn't load the ROM... aborting emulation." << std::endl;

        return -1;
    }
    
    for (;;)
    {
        chip8_cpu.emulate_cycle();

        if (chip8_cpu.is_draw_flag_set())
        {
            draw_graphics();
        }

        chip8_cpu.update_pressed_keys();
    }

    return 0;
}