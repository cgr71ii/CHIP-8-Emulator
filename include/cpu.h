#ifndef CHIP8_CPU
#define CHIP8_CPU

#define CHIP8_CPU_DEBUG

class Chip8_CPU
{
    public:
        static const unsigned MEMORY_LENGTH_B = 4096;
        static const unsigned GENERAL_PURPOSE_REGISTERS = 16;
        static const unsigned WIDTH = 64;
        static const unsigned HEIGHT = 32;
        static const unsigned STACK_DEEPNESS = 16;
        static const unsigned KEY_MAPPING_SIZE = 16;

        Chip8_CPU();

        #ifdef CHIP8_CPU_DEBUG
        void print_memory();
        void print_status();
        #endif

    private:
        // Current opcode
        unsigned short opcode;

        // CHIP-8 memory (4KB = 1024B * 4 = 4096b = 4KB)
        unsigned char memory[Chip8_CPU::MEMORY_LENGTH_B];

        /*
         * CPU registers (8-bit)
         * ---------------------
         * V[0] .. V[14] -> General purpose registers
         * V[15] -> Carry flag
         */
        unsigned char V[Chip8_CPU::GENERAL_PURPOSE_REGISTERS];

        // Index register (0x000 - 0xFFF)
        unsigned short I;

        // Program couter (0x000 - 0xFFF)
        unsigned short pc;

        // Graphics of the CHIP-8 (64 width x 32 height)
        unsigned char gfx[Chip8_CPU::WIDTH * Chip8_CPU::HEIGHT];

        /*
         * Timer registers (60 Hz) for sound.
         */

        // Time that system's buzzer sounds
        unsigned char delay_timer;

        // Timer that, when reaches 0, makes the system's buzzer sounds
        unsigned char sound_timer;

        /*
         * Stack -> subroutines
         * 
         * Max. deepness: 16
         */

        // Stack
        unsigned short stack[Chip8_CPU::STACK_DEEPNESS];

        // Stack pointer
        unsigned short sp;

        /*
         * Key mapping -> HEX based keyboard (0x0 - 0xF)
         */

        // Key mapping
        unsigned char key[Chip8_CPU::KEY_MAPPING_SIZE];

};

#endif