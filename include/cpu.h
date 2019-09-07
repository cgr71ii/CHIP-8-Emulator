#ifndef CHIP8_CPU
#define CHIP8_CPU

#define CHIP8_CPU_DEBUG
//#define CHIP8_CPU_DEBUG_LOAD_ROM_VERBOSE

#include <string>

using std::string;

using byte = unsigned char;
using WORD = unsigned short;
using DWORD = unsigned int;

class CPU
{
    public:
        static const unsigned MEMORY_LENGTH_B = 4096;
        static const unsigned GENERAL_PURPOSE_REGISTERS = 16;
        static const unsigned WIDTH = 64;
        static const unsigned HEIGHT = 32;
        static const unsigned STACK_DEEPNESS = 16;
        static const unsigned KEY_MAPPING_SIZE = 16;
        static const unsigned ROM_MEMORY_BEGIN = 0x0200;
        static const unsigned COLOR_BLACK = 0;
        static const unsigned COLOR_WHITE = 1;

        // Fontset
        static const unsigned FONTSET_MEMORY_BEGIN = 0x0050;
        static const unsigned FONTSET_SIZE = 80;
        const byte FONTSET[CPU::FONTSET_SIZE] = 
        {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

        CPU();

        void initializate();
        bool load_rom(const string&);
        void emulate_cycle();
        bool is_draw_flag_set() const;
        void update_pressed_keys();

        #ifdef CHIP8_CPU_DEBUG
        void print_memory() const;
        void print_status() const;
        void print_screen() const;
        void execute_instruction(const WORD&);
        void store(const WORD&, const byte&);
        #endif

    private:
        // Current opcode
        WORD opcode;

        // CHIP-8 memory (4KB = 1024B * 4 = 4096b = 4KB)
        byte memory[CPU::MEMORY_LENGTH_B];

        /*
         * CPU registers (8-bit)
         * ---------------------
         * V[0] .. V[14] -> General purpose registers
         * V[15] -> Carry flag
         */
        byte V[CPU::GENERAL_PURPOSE_REGISTERS];

        // Index register (0x000 - 0xFFF)
        WORD I;

        // Program couter (0x000 - 0xFFF)
        WORD pc;

        // Graphics of the CHIP-8 (64 width x 32 height)
        byte gfx[CPU::WIDTH * CPU::HEIGHT];

        /*
         * Timer registers (60 Hz) for sound.
         */

        // Time that system's buzzer sounds
        byte delay_timer;

        // Timer that, when reaches 0, makes the system's buzzer sounds
        byte sound_timer;

        /*
         * Stack -> subroutines
         * 
         * Max. deepness: 16
         */

        // Stack
        WORD stack[CPU::STACK_DEEPNESS];

        // Stack pointer
        WORD sp;

        /*
         * Key mapping -> HEX based keyboard (0x0 - 0xF)
         */

        // Key mapping
        byte key[CPU::KEY_MAPPING_SIZE];

        // Flag to know when we have to draw on the screen.
        bool draw_flag;

        // Variable to halt the program
        bool halt;

        // Private function
        std::streampos get_file_length(const string&) const;
        void print_unknown_opcode(const string = "") const;
        void push(WORD);
        WORD pop();
        void execute_instruction();
};

#endif