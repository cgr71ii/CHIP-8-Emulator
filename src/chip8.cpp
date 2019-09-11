#include "cpu.h"

#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <csignal>

//#define CHIP8_VIDEO_DEBUG

const unsigned SCREEN_FACTOR = 10;

struct SDL2Graphics
{
    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL2Graphics() :
        window(NULL),
        renderer(NULL)
    {
    }
};

SDL2Graphics* _graphics = NULL;
std::string rom_path("");

void delete_graphics(SDL2Graphics* graphics)
{
    if (graphics != NULL)
    {
        if (graphics->window != NULL)
        {
            SDL_DestroyWindow(graphics->window);
        }
        if (graphics->renderer != NULL)
        {
            SDL_DestroyRenderer(graphics->renderer);
        }
        
        SDL_Quit();

        delete graphics;
    }
}

void signal_handler(int signal_num)
{
    std::cout << std::endl << "Interrupt signal (" << signal_num << ") received." << std::endl;

    delete_graphics(_graphics);

    exit(signal_num);
}

int sdl2_test()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Window* win = SDL_CreateWindow("Hello World!", 100, 100, 620, 387, SDL_WINDOW_SHOWN);

    if (win == NULL)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Renderer* ren
    = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (ren == NULL)
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Surface* bmp = SDL_LoadBMP("img/grumpy-cat.bmp");

    if (bmp == NULL)
    {
        std::cerr << "SDL_LoadBMP Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, bmp);
    SDL_FreeSurface(bmp);

    if (tex == NULL)
    {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 20; i++)
    {
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);
        SDL_Delay(100);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return EXIT_SUCCESS;
}

SDL2Graphics* setup_graphics()
{
    if (_graphics != NULL)
    {
        // If it is initialized, with this code we can use this function multiple times if needed.
        delete _graphics;

        _graphics = NULL;
    }

    _graphics = new SDL2Graphics;

    if (_graphics == NULL)
    {
        std::cerr << "ERROR: couldn't alloc memory for the graphics. Aborting." << std::endl;

        exit(-1);
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return NULL;
    }

    _graphics->window = SDL_CreateWindow("CHIP 8 - Emulator", 100, 100, CPU::WIDTH * SCREEN_FACTOR, CPU::HEIGHT * SCREEN_FACTOR, SDL_WINDOW_SHOWN);

    if (_graphics->window == NULL)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return NULL;
    }

    _graphics->renderer = 
    SDL_CreateRenderer(_graphics->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (_graphics->renderer == NULL)
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return NULL;
    }

    // Set the color
    SDL_SetRenderDrawColor(_graphics->renderer, 0, 0, 0, 255);

    // Reset the graphics
    SDL_RenderClear(_graphics->renderer);

    // Render the results
    SDL_RenderPresent(_graphics->renderer);

    return _graphics;
}

void setup_input()
{

}

void draw_graphics(SDL2Graphics* graphics, byte* gfx)
{
    for (size_t row = 0; row < CPU::HEIGHT; row++)
    {
        for (size_t col = 0; col < CPU::WIDTH; col++)
        {
            SDL_Rect rectangle;

            rectangle.x = col * SCREEN_FACTOR;
            rectangle.y = row * SCREEN_FACTOR;
            rectangle.w = SCREEN_FACTOR;
            rectangle.h = SCREEN_FACTOR;

            if (gfx[row * CPU::WIDTH + col] == CPU::COLOR_BLACK)
            {
                SDL_SetRenderDrawColor(graphics->renderer, 0, 0, 0, 255);
            }
            else if (gfx[row * CPU::WIDTH + col] == CPU::COLOR_WHITE)
            {
                SDL_SetRenderDrawColor(graphics->renderer, 255, 255, 255, 255);
            }
            else
            {
                std::cout << "WARNING: unknown color. Using black by default." << std::endl;

                SDL_SetRenderDrawColor(graphics->renderer, 0, 0, 0, 255);
            }

            SDL_RenderFillRect(graphics->renderer, &rectangle);

            //SDL_RenderPresent(graphics->renderer);    // It updates the screen at every rectangle that is created
        }
    }

    SDL_RenderPresent(graphics->renderer);
}

void print_syntax_and_exit(char** argv)
{
    std::cerr << "Syntax: " << argv[0] << " [path_to_file | --env_var=ENV_VAR]" << std::endl;

    exit(-1);
}

void handle_args(int argc, char** argv)
{
    if (argc > 1)
    {
        if (argc == 2)
        {
            if (strncmp(argv[1], "--env_var=", 10) == 0)
            {
                if (strlen(argv[1]) == 10)
                {
                    print_syntax_and_exit(argv);
                }
                else
                {
                    rom_path = string(getenv(argv[1] + 10));
                }
            }
            else
            {
                rom_path = string(argv[1]);
            }
        }
        else
        {
            print_syntax_and_exit(argv);
        }
    }
}

int main(int argc, char** argv)
{
    handle_args(argc, argv);

    #ifdef CHIP8_VIDEO_DEBUG
    int sdl2_test_result = sdl2_test();

    if (sdl2_test_result != EXIT_SUCCESS)
    {
        std::cerr << "ERROR: something failed with SDL2. Aborting." << std::endl;

        return -1;
    }
    #endif

    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);

    CPU chip8_cpu;
    byte* gfx = chip8_cpu.get_gfx();

    SDL2Graphics* graphics = setup_graphics();

    if (graphics == NULL)
    {
        std::cerr << "ERROR: something wrong happened while initializing SDL2. Aborting." << std::endl;

        delete_graphics(graphics);

        return -1;
    }

    setup_input();

    chip8_cpu.initializate();

    if (rom_path.empty())
    {
        std::cout << "Type the ROM you want to load: ";
        //std::cin >> rom_path; // It doesn't allow spaces
        std::getline(std::cin, rom_path);   // It allows spaces
    }

    bool loaded = chip8_cpu.load_rom(rom_path);

    if (loaded)
    {
        std::cout << "ROM loaded successfully!" << std::endl;
    }
    else
    {
        std::cerr << "Couldn't load the ROM... aborting emulation." << std::endl;
        
        delete_graphics(graphics);

        return -1;
    }
    
    for (;;)
    {
        chip8_cpu.emulate_cycle();

        if (chip8_cpu.is_draw_flag_set())
        {
            draw_graphics(graphics, gfx);
        }

        chip8_cpu.update_pressed_keys();
    }

    delete_graphics(graphics);

    return 0;
}