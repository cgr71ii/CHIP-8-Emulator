
#include "cpu.h"

#include <iostream>
#include <cstring>
#include <fstream>
#include <ctime>
#include <cstdlib>

using std::string;

CPU::CPU()
{
    
}

void CPU::initializate()
{
    // Reset all registers and buffers
    this->opcode = 0;
    this->I = 0;
    this->pc = 0x200;
    this->sp = 0;
    this->draw_flag = true; // Clear screen once
    this->halt = false;

    memset(this->memory, 0, CPU::MEMORY_LENGTH_B * sizeof(byte));
    memset(this->V, 0, CPU::GENERAL_PURPOSE_REGISTERS * sizeof(byte));
    memset(this->gfx, 0, CPU::GFX_LENGTH * sizeof(byte));
    memset(this->stack, 0, CPU::STACK_DEEPNESS * sizeof(WORD));
    memset(this->key, 0, CPU::KEY_MAPPING_SIZE * sizeof(byte));

    // Load fontset
    for (size_t i = CPU::FONTSET_MEMORY_BEGIN; i < CPU::FONTSET_MEMORY_BEGIN + CPU::FONTSET_SIZE; i++)
    {
        this->memory[i] = CPU::FONTSET[i - CPU::FONTSET_MEMORY_BEGIN];
    }

    // Reset timers
    this->delay_timer = 0;
    this->sound_timer = 0;

    // Set a random seed
    srand(time(NULL));
}

std::streampos CPU::get_file_length(const string& path) const
{
    std::ifstream file;

    file.open(path, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "The file (" << path << ") couldn't be opened or found." << std::endl;

        return -1;
    }

    std::streampos begin, end;

    begin = file.tellg();
    file.seekg(0, std::ios::end);
    end = file.tellg();
    file.close();

    return end - begin;
}

bool CPU::load_rom(const string& path)
{
    unsigned file_length = this->get_file_length(path);
    std::ifstream rom;

    if (file_length == -1)
    {
        // Couldn't open the file or doesn't exist

        return false;
    }

    rom.open(path, std::ios::binary);

    if (!rom.is_open())
    {
        std::cerr << "The file (" << path << ") couldn't be opened or found." << std::endl;

        return false;
    }

    unsigned free_space_after_fit = CPU::MEMORY_LENGTH_B - CPU::ROM_MEMORY_BEGIN - file_length;

    if (free_space_after_fit < 0)
    {
        std::cerr << "ERROR: the whole game doesn't fit. Aborting the loading." << std::endl;

        return false;
    }

    rom.read((char*)this->memory + CPU::ROM_MEMORY_BEGIN, file_length);

    #ifdef CHIP8_CPU_DEBUG_LOAD_ROM_VERBOSE
    std::cout << "CPU::MEMORY_LENGTH_B = " << CPU::MEMORY_LENGTH_B << std::endl;
    std::cout << "CPU::ROM_MEMORY_BEGIN = " << CPU::ROM_MEMORY_BEGIN << std::endl;
    std::cout << "ROM length (Bytes) = " << file_length << std::endl;
    std:: cout << "free_space_after_fit = " << free_space_after_fit << std::endl;
    this->print_memory();
    #endif

    rom.close();

    return true;
}

void CPU::push(WORD value)
{
    if (this->sp == CPU::STACK_DEEPNESS)
    {
        // In case this situation happens, it can be solved increasing the stack deepness.

        std::cout << "WARNING: stack overflow detected (stack)!" << std::endl;
    }

    this->stack[this->sp] = value;
    this->sp++;
}

WORD CPU::pop()
{
    if (this->sp == 0)
    {
        // In case this situation happens, it'd be due to a logic failure or a coding failure.

        std::cout << "WARNING: stack underflow detected (stack)!" << std::endl;
    }

    WORD value;

    value = this->stack[this->sp - 1];
    this->sp--;

    return value;
}

byte* CPU::get_gfx()
{
    return this->gfx;
}

void CPU::emulate_cycle()
{
    if (this->halt)
    {
        // Halt -> skipping cycle

        return;
    }

    // We disable the draw flag and at the end of this function might be enabled
    this->draw_flag = false;

    // Fetch opcode
    if (this->pc + 1 >= CPU::MEMORY_LENGTH_B)
    {
        std::cout << "WARNING: stack overflow detected (memory)!." << std::endl;
    }

    this->opcode = this->memory[this->pc] << 8 | this->memory[this->pc + 1];

    #ifdef CHIP8_CPU_DEBUG_OPCODE_VERBOSE
    std::cout << "Current opcode: 0x" << std::hex << this->opcode << std::dec << std::endl;
    #endif

    #ifdef CHIP8_CPU_DEBUG_NEXT_STEP
    std::string foo;

    std::cout << "Press enter... ";
    getline(std::cin, foo);
    #endif

    // Decode and execute instruction
    this->execute_instruction();
}

void CPU::execute_instruction()
{
    switch(this->opcode & 0xF000)
    {
        case 0x0000:
            switch(this->opcode)
            {
                //   0x00E0 -> Clears the screen.
                case 0x00E0:
                    this->draw_flag = true;

                    /*
                    for (size_t i = 0; i < CPU::GFX_LENGTH; i++)
                    {
                        this->gfx[i] = CPU::COLOR_BLACK;
                    }
                    */
                    
                    memset(this->gfx, CPU::COLOR_BLACK, CPU::GFX_LENGTH * sizeof(byte));

                    this->pc += 2;

                    break;
                //   0x00EE -> Returns from a subroutine.
                case 0x00EE:
                    this->pc = this->pop();
                    this->pc += 2;
                    
                    break;
                //   0x0NNN -> Calls RCA 1802 program at address NNN. Not necessary for most ROMs.
                default:
                    this->print_unknown_opcode("RCA 1802 program invoked.");
                    this->pc += 2;

                    break;
            }

            break;
        //   0x1NNN -> Jumps to address NNN.
        case 0x1000:
            this->pc = this->opcode & 0x0FFF;

            break;
        //   0x2NNN -> Calls subroutine at NNN.
        case 0x2000:
            this->push(this->pc);
            this->pc = this->opcode & 0x0FFF;

            break;
        //   0x3XNN -> Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
        case 0x3000:
        {
            byte index = (this->opcode & 0x0F00) >> 8;
            byte value = this->opcode & 0x00FF;

            if (this->V[index] == value)
            {
                this->pc += 2;
            }

            this->pc += 2;
        }

            break;
        //   0x4XNN -> Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block)
        case 0x4000:
        {
            byte index = (this->opcode & 0x0F00) >> 8;
            byte value = this->opcode & 0x00FF;

            if (this->V[index] != value)
            {
                this->pc += 2;
            }

            this->pc += 2;
        }

            break;
        //   0x5XY0 -> Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block)
        case 0x5000:
        {
            if ((this->opcode & 0x000F) != 0)
            {
                this->print_unknown_opcode();
                this->pc += 2;

                break;
            }

            byte x_index = (this->opcode & 0x0F00) >> 8;
            byte y_index = (this->opcode & 0x00F0) >> 4;

            if (this->V[x_index] == this->V[y_index])
            {
                this->pc += 2;
            }

            this->pc += 2;
        }

            break;
        //   0x6XNN -> Sets VX to NN.
        case 0x6000:
        {
            byte index = (this->opcode & 0x0F00) >> 8;
            byte value = this->opcode & 0x00FF;

            this->V[index] = value;

            this->pc += 2;
        }

            break;
        //   0x7XNN -> Adds NN to VX. (Carry flag is not changed)
        case 0x7000:
        {
            byte index = (this->opcode & 0x0F00) >> 8;
            byte value = this->opcode & 0x00FF;

            this->V[index] += value;

            this->pc += 2;
        }

            break;
        case 0x8000:
            switch(this->opcode & 0x000F)
            {
                //   0x8XY0 -> Sets VX to the value of VY.
                case 0x0000:
                {
                    byte x_index = (this->opcode & 0x0F00) >> 8;
                    byte y_index = (this->opcode & 0x00F0) >> 4;

                    this->V[x_index] = this->V[y_index];

                    this->pc += 2;
                }

                    break;
                //   0x8XY1 -> Sets VX to VX or VY. (Bitwise OR operation)
                case 0x0001:
                {
                    byte x_index = (this->opcode & 0x0F00) >> 8;
                    byte y_index = (this->opcode & 0x00F0) >> 4;

                    this->V[x_index] |= this->V[y_index];

                    this->pc += 2;
                }

                    break;
                //   0x8XY2 -> Sets VX to VX and VY. (Bitwise AND operation)
                case 0x0002:
                {
                    byte x_index = (this->opcode & 0x0F00) >> 8;
                    byte y_index = (this->opcode & 0x00F0) >> 4;

                    this->V[x_index] &= this->V[y_index];

                    this->pc += 2;
                }

                    break;
                //   0x8XY3 -> Sets VX to VX xor VY.
                case 0x0003:
                {
                    byte x_index = (this->opcode & 0x0F00) >> 8;
                    byte y_index = (this->opcode & 0x00F0) >> 4;

                    this->V[x_index] ^= this->V[y_index];

                    this->pc += 2;
                }

                    break;
                //   0x8XY4 -> Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
                case 0x0004:
                {
                    byte x_index = (this->opcode & 0x0F00) >> 8;
                    byte y_index = (this->opcode & 0x00F0) >> 4;

                    if (this->V[x_index] > 0xFF - this->V[y_index])
                    {
                        this->V[0xF] = 1;
                    }
                    else
                    {
                        this->V[0xF] = 0;
                    }

                    this->V[x_index] += this->V[y_index];

                    this->pc += 2;
                }

                    break;
                //   0x8XY5 -> VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                case 0x0005:
                {
                    byte x_index = (this->opcode & 0x0F00) >> 8;
                    byte y_index = (this->opcode & 0x00F0) >> 4;

                    if (this->V[x_index] < this->V[y_index])
                    {
                        // There is a borrow
                        this->V[0xF] = 0;
                    }
                    else
                    {
                        this->V[0xF] = 1;
                    }

                    this->V[x_index] -= this->V[y_index];

                    this->pc += 2;
                }

                    break;
                //   0x8XY6 -> Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                case 0x0006:
                {
                    byte x_index = (this->opcode & 0x0F00) >> 8;

                    this->V[0xF] = this->V[x_index] & 0x01;
                    this->V[x_index] >>= 1;

                    this->pc += 2;
                }

                    break;
                //   0x8XY7 -> Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                case 0x0007:
                {
                    byte x_index = (this->opcode & 0x0F00) >> 8;
                    byte y_index = (this->opcode & 0x00F0) >> 4;

                    if (this->V[y_index] < this->V[x_index])
                    {
                        // There is a borrow
                        this->V[0xF] = 0;
                    }
                    else
                    {
                        this->V[0xF] = 1;
                    }

                    this->V[x_index] = this->V[y_index] - this->V[x_index];

                    this->pc += 2;
                }
                    break;
                //   0x8XYE -> Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                case 0x000E:
                {
                    byte x_index = (this->opcode & 0x0F00) >> 8;

                    this->V[0xF] = this->V[x_index] >> 7;
                    this->V[x_index] <<= 1;

                    this->pc += 2;
                }

                    break;
                default:
                    this->print_unknown_opcode();
                    this->pc += 2;

                    break;
            }

            break;
        //   0x9XY0 -> Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
        case 0x9000:
            if ((this->opcode & 0x000F) != 0)
            {
                this->print_unknown_opcode();
                this->pc += 2;

                break;
            }

            {
                byte x_index = (this->opcode & 0x0F00) >> 8;
                byte y_index = (this->opcode & 0x00F0) >> 4;

                if (this->V[x_index] != this->V[y_index])
                {
                    this->pc += 2;
                }

                this->pc += 2;
            }

            break;
        //   0xANNN -> Sets I to the address NNN.
        case 0xA000:
            this->I = this->opcode & 0x0FFF;

            this->pc += 2;

            break;
        //   0xBNNN -> Jumps to the address NNN plus V0.
        case 0xB000:
            this->pc = (this->opcode & 0x0FFF) + this->V[0];

            break;
        //   0xCXNN -> Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
        case 0xC000:
        {
            byte index = (this->opcode & 0x0F00) >> 8;
            byte NN = (this->opcode & 0x00FF);

            this->V[index] = (rand() % 0x100) & NN;

            this->pc += 2;
        }

            break;
        //   0xDXYN -> Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
        // Each row of 8 pixels is read as bit-coded starting from memory location I; 
        // I value doesn’t change after the execution of this instruction. As described above, 
        // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen.
        case 0xD000:
        {
            this->draw_flag = true;
            
            this->V[0xF] = 0;

            byte x_index = (this->opcode & 0x0F00) >> 8;
            byte y_index = (this->opcode & 0x00F0) >> 4;
            byte n = (this->opcode & 0x000F);

            // Each row
            for (size_t row = 0; row < n; row++)
            {
                // Each column
                for (size_t col = 0; col < 8; col++)
                {
                    if (this->V[y_index] + row >= CPU::HEIGHT || 
                        this->V[x_index] + col >= CPU::WIDTH)
                    {
                        std::cout << "WARNING: screen buffer overflow (might not be dangerous if is not close the end of the buffer)." << std::endl;
                    }

                    if ((this->memory[this->I + row] & (0x80 >> col)) != 0)   // WARNING: if memory's declaration was not unsigned, this might fail when col = 0 (memory = 1000 0000 (in C2's complement, if signed, is -0) or memory = 0000 0000)
                    {
                        // We draw the current pixel because it is set in the sprite

                        //                         ROW                               COL
                        //        -------------------------------------   ----------------------
                        this->gfx[(this->V[y_index] + row) * CPU::WIDTH + this->V[x_index] + col] ^= 1;

                        // Collision test
                        if (this->gfx[(this->V[y_index] + row) * CPU::WIDTH + this->V[x_index] + col] == 0)
                        {
                            // Collision detected
                            this->V[0xF] = 1;
                        }
                    }
                }
            }
            
            this->pc += 2;
        }
            
            break;
        case 0xE000:
            switch(this->opcode & 0x00FF)
            {
                //   0xEX9E -> Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
                case 0x009E:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;

                    if (this->key[this->V[index]] != 0)
                    {
                        // Key at V[index] is pressed -> skip next instruction

                        this->pc += 2;
                    }

                    this->pc += 2;
                }

                    break;
                //   0xEXA1 -> Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
                case 0x00A1:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;

                    if (this->key[this->V[index]] == 0)
                    {
                        // Key at V[index] is not pressed -> skip next instruction

                        this->pc += 2;
                    }

                    this->pc += 2;
                }

                    break;
                default:
                    this->print_unknown_opcode();
                    this->pc += 2;

                    break;
            }

            break;
        case 0xF000:
            switch (this->opcode & 0x00FF)
            {
                //   0xFX07 -> Sets VX to the value of the delay timer.
                case 0x0007:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;

                    this->V[index] = this->delay_timer;

                    this->pc += 2;
                }

                    break;
                //   0xFX0A -> A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
                case 0x000A:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;
                    bool key_pressed = false;

                    for (size_t index = 0; index < CPU::KEY_MAPPING_SIZE; index++)
                    {
                        if (this->key[index] != 0)
                        {
                            this->V[index] = index;
                            key_pressed = true;

                            break;
                        }
                    }
                    
                    if (key_pressed)
                    {
                        this->pc += 2;
                    }

                    // If the key was not pressed, the PC would not be increased, and this instruction will be executed until a key is pressed
                }

                    break;
                //   0xFX15 -> Sets the delay timer to VX.
                case 0x0015:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;

                    this->delay_timer = this->V[index];

                    this->pc += 2;
                }

                    break;
                //   0xFX18 -> Sets the sound timer to VX.
                case 0x0018:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;

                    this->sound_timer = this->V[index];

                    this->pc += 2;
                }

                    break;
                //   0xFX1E -> Adds VX to I.
                case 0x001E:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;

                    if (this->I > 0xFF - this->V[index])
                    {
                        this->V[0xF] = 1;
                    }
                    else
                    {
                        this->V[0xF] = 0;
                    }

                    this->I += this->V[index];

                    this->pc += 2;
                }

                    break;
                //   0xFX29 -> Sets I to the location of the sprite for the character in VX. 
                // Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                case 0x0029:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;

                    if (this->V[index] > 0xF ||
                        this->V[index] * 5 >= CPU::FONTSET_SIZE)    // Each character sprite is 5 bytes long
                    {
                        std::cout << "WARNING: fontset overflow." << std::endl;
                    }

                    this->I = CPU::FONTSET_MEMORY_BEGIN + this->V[index] * 5;

                    this->pc += 2;
                }

                    break;
                //   0xFX33 -> Stores the binary-coded decimal representation of VX, 
                // with the most significant of three digits at the address in I, 
                // the middle digit at I plus 1, and the least significant digit at I plus 2. 
                // In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, 
                // the tens digit at location I+1, and the ones digit at location I+2.
                case 0x0033:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;

                    this->memory[this->I + 0] =  this->V[index] / 100;          // Most significant BCD digit
                    this->memory[this->I + 1] = (this->V[index] / 10 ) % 10;    // Middle BCD digit
                    this->memory[this->I + 2] = (this->V[index] % 100) % 10;    // Least significant BCD digit

                    this->pc += 2;
                }

                    break;
                //   0xFX55 -> Stores V0 to VX (including VX) in memory starting at address I. 
                // The offset from I is increased by 1 for each value written, but I itself is left unmodified.
                case 0x0055:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;

                    for (size_t i = 0; i <= index; i++)
                    {
                        this->memory[this->I + i] = this->V[i];
                    }

                    this->pc += 2;
                }

                    break;
                //   0xFX65 -> Fills V0 to VX (including VX) with values from memory starting at address I. 
                // The offset from I is increased by 1 for each value written, but I itself is left unmodified.
                case 0x0065:
                {
                    byte index = (this->opcode & 0x0F00) >> 8;

                    for (size_t i = 0; i <= index; i++)
                    {
                        this->V[i] = this->memory[this->I + i];
                    }

                    this->pc += 2;
                }

                    break;
                default:
                    this->print_unknown_opcode();
                    this->pc += 2;

                    break;
            }

            break;
        default:
            this->print_unknown_opcode();
            this->pc += 2;

            break;
    }
}

bool CPU::is_draw_flag_set() const
{
    return this->draw_flag;
}

void CPU::update_pressed_keys()
{
    
}

void CPU::print_unknown_opcode(const string msg) const
{
    std::cerr << "ERROR: unknown opcode (0x" << std::hex << this->opcode << std::dec << ")." << std::endl;

    if (!msg.empty())
    {
        std::cerr << std::endl << msg << std::endl;
    }
}

#ifdef CHIP8_CPU_DEBUG

void CPU::print_memory() const
{
    for (size_t i = 0; i < CPU::MEMORY_LENGTH_B; i++)
    {
        std::cout << "0x";

        if ((unsigned)this->memory[i] < 0x10)
        {
            std::cout << "0";
        }

        std::cout << std::hex << (unsigned)this->memory[i] << " ";

        if ((i + 1) % 16 == 0)
        {
            std::cout << std::dec
                      << " <- {8 * [" << (i % 16) + 0 + 2 * (i / 16) - 15 << " | " 
                                      << (i % 16) + 1 + 2 * (i / 16) - 15
                                      << "]}";

            std::cout << "\n";
        }
        else if ((i + 1) % 8 == 0 && (i + 1) != CPU::MEMORY_LENGTH_B)
        {
            std:: cout << "| ";
        }
    }
}

void CPU::print_status() const
{
    std::cout << "General purpose registers:" << std::endl;
    std::cout << "--------------------------" << std::endl;
    
    for (size_t i = 0; i < CPU::GENERAL_PURPOSE_REGISTERS; i++)
    {
        std::cout << " V[" << i << "] = " << (unsigned)this->V[i] << std::endl;
    }

    std::cout << "--------------------------" << std::endl;
    std::cout << "Register I = " << (unsigned)this->I << std::endl;
    std::cout << "Current opcode = " << (unsigned)this->opcode << std::endl;
    std::cout << "Program Counter = " << (unsigned)this->pc << std::endl;
    std::cout << "Delay timer = " << (unsigned)this->delay_timer << std::endl;
    std::cout << "Sound timer = " << (unsigned)this->sound_timer << std::endl;
    std::cout << "Draw flag = " << std::boolalpha << draw_flag << std::endl;
    std::cout << "Stack Pointer = " << (unsigned)this->sp << std::endl;
    std::cout << "Stack:" << std::endl;
    std::cout << "------" << std::endl;

    for (size_t i = 0; i < CPU::STACK_DEEPNESS; i++)
    {
        std::cout << " stack[" << i << "] = " << (unsigned)this->stack[i] << std::endl;
    }

    std::cout << "------" << std::endl;
    std::cout << "Key mapping:" << std::endl;
    std::cout << "------------" << std::endl;

    for (size_t i = 0; i < CPU::KEY_MAPPING_SIZE; i++)
    {
        std::cout << " key[" << i << "] = " << (unsigned)this->key[i] << std::endl;
    }
    
    std::cout << "------------" << std::endl;
}

void CPU::print_screen() const
{
    for (size_t row = 0; row < CPU::HEIGHT; row++)
    {
        for (size_t col = 0; col < CPU::WIDTH; col++)
        {
            std::cout << (unsigned)this->gfx[row * CPU::HEIGHT + col];
        }

        std::cout << std::endl;
    }
}

void CPU::execute_instruction(const WORD& instruction)
{
    this->opcode = instruction;

    this->execute_instruction();
}

void CPU::store(const WORD& addr, const byte& value)
{
    this->memory[addr] = value;
}

#endif