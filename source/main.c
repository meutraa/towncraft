#include "game.h"
#include "main_menu.h"
#include "options.h"
#include "options_menu.h"

#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include "constants.h"

static SDL_Window* window;
static SDL_Renderer* renderer;

static void Quit(void)
{
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(void)
{
    /* Ensures any exit will call cleanup. */
    atexit(Quit);

    /* Initialise the video and timer subsystem. */
    if (0 != SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO))
    {
        fprintf(stderr, "\nUnable to initialize SDL Subsystem: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if (0 != TTF_Init())
    {
        fprintf(stderr, "\nUnable to initialize SDL_ttf Subsystem: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    if (MIX_INIT_OGG != Mix_Init(MIX_INIT_OGG))
    {
        fprintf(stderr, "\nUnable to initialize SDL_Mix Subsystem: %s\n", Mix_GetError());
    }

    if (0 != Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096))
    {
        fprintf(stderr, "\nUnable to open audio mixer: %s\n", Mix_GetError());
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, render_scale_quality);

    /* Create window */
    window = SDL_CreateWindow(GAME_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        resolution_width, resolution_height,
        SDL_WINDOW_OPENGL
      | ((fullscreen == 1) ? SDL_WINDOW_FULLSCREEN : 0)
      | ((fullscreen == 2) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)
    );

    if(!window)
    {
        fprintf(stderr, "\nCould not create window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    /* Create the renderer for the SDL_Window. */
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | (vsync ? SDL_RENDERER_PRESENTVSYNC : 0));

    if(!renderer)
    {
        fprintf(stderr, "\nCould not create renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    Status status = SWITCHTO_MAINMENU;
    Mix_Music* chiptune = Mix_LoadMUS("resources/audio/music/rolemusic_the_pirate_and_the_dancer.ogg");

    /* Start the main loop. */
    while(status != QUIT_PROGRAM)
    {
        if(SWITCHTO_MAINMENU == status)
        {
            Mix_FadeInMusicPos(chiptune, -1, 2000, 0.5);
            status = main_menu_loop(renderer);
        }
        else if(SWITCHTO_OPTIONS == status)
        {
            status = options_menu_loop(renderer);
        }
        else if(SWITCHTO_GAME == status)
        {
            Mix_FadeOutMusic(100);
            status = game_loop(window, renderer);
        }
    }
    Mix_FreeMusic(chiptune);
    exit(EXIT_SUCCESS);
}
