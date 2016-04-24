#include "options.h"
#include "main_menu.h"
#include "options_menu.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

int main(/*int argc, char** argv*/)
{
	/* Ensures any return will call SDL_Quit first. */
	atexit(SDL_Quit);

	/* Initialise the video and timer subsystem. */
	if(0 != SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO))
	{
		fprintf(stderr, "\nUnable to initialize SDL Subsystem: %s\n", SDL_GetError());
		return 1;
	}

	if(0 != TTF_Init())
	{
		fprintf(stderr, "\nUnable to initialize SDL_ttf Subsystem: %s\n", TTF_GetError());
		TTF_Quit();
		return 1;
	}

	if(MIX_INIT_OGG != Mix_Init(MIX_INIT_OGG))
	{
		fprintf(stderr, "\nUnable to initialize SDL_Mix Subsystem: %s\n", Mix_GetError());
	}

	if(0 != Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096))
	{
		fprintf(stderr, "\nUnable to open audio mixer: %s\n", Mix_GetError());
	}
	int freq, channels;
	Uint16 format;
	Mix_QuerySpec(&freq, &format, &channels);
	printf("Device: frequency=%d, format=%d, channels=%d\n", freq, format, channels);

	/* Create window */
	SDL_Window* window = SDL_CreateWindow(
		GAME_NAME,                                       // Window title
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  // Position (x,y)
		resolution_width, resolution_height,             // Size (x,y)
		((fullscreen == 1) ? SDL_WINDOW_FULLSCREEN : 0)|        // If custom_resolution is set use size x,y fullscreen
		((fullscreen == 2) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) // If fullscreen is set use desktop resolution
		// Window flags http://wiki.libsdl.org/SDL_WindowFlags
	);

	if(NULL == window)
	{
		fprintf(stderr, "\nCould not create window: %s\n", SDL_GetError());
		return 1;
	}

	/* Create the renderer for the SDL_Window. */
	SDL_Renderer* renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED|
		(vsync ? SDL_RENDERER_PRESENTVSYNC : 0) /* http://wiki.libsdl.org/SDL_RendererFlags */
	);

	if(NULL == renderer)
	{
		fprintf(stderr, "\nCould not create renderer: %s\n", SDL_GetError());
		if(NULL != window) SDL_DestroyWindow(window);
		return 1;
	}

	Status status = SWITCHTO_MAINMENU;
	Mix_Music* chiptune = Mix_LoadMUS("resources/audio/music/rolemusic_the_pirate_and_the_dancer.ogg");

	/* Start the main loop. */
	while(status != QUIT_PROGRAM)
	{
		switch(status)
		{
			case SWITCHTO_MAINMENU:
				Mix_FadeInMusicPos(chiptune, -1, 2000, 0.5);
				status = main_menu_loop(renderer);
				break;
			case SWITCHTO_OPTIONS:
				Mix_FadeOutMusic(1000);
				status = options_menu_loop(renderer);
				break;
			case SWITCHTO_GAME:
				Mix_FadeOutMusic(100);
				status = game_loop(renderer);
				break;
			default:
				break;
		}
	}

	Mix_HaltMusic();
	Mix_FreeMusic(chiptune);
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	return 0;
}
