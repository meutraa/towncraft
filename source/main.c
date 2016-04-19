#include "file.h"
#include "constants.h"
#include "main_menu.h"
#include "status.h"

#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#define GAME_NAME "Towncraft"

int main(/*int argc, char** argv*/)
{
	/* Ensures any return will call SDL_Quit first. */
	atexit(SDL_Quit);
	
	/* Load in relevant settings. */
	int vsync = 1;
	/* fullscreen = 0, use fallback/window res,
	 * fullscreen = 1, use fullscreen at fallback/window res,
	 * fullscreen = 2, use fullscreen at current desktop res
	 */
	int fullscreen = 0;
	int resolution_width  = DESIGN_WIDTH;
	int resolution_height = DESIGN_HEIGHT;
	
	/* Initialise the video and timer subsystem. */
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER))
	{
		fprintf(stderr, "\nUnable to initialize SDL Subsystem: %s\n", SDL_GetError());
		return 1;
	}
	
	/* Initialise the (optional) audio subsystem. */
	if(SDL_InitSubSystem(SDL_INIT_AUDIO))
	{
		fprintf(stderr, "\nUnable to initialize SDL Audio Subsystem: %s\n", SDL_GetError());
	}
	
	if(TTF_Init())
	{
		fprintf(stderr, "\nUnable to initialize SDL_ttf Subsystem\n");
		return 1;
	}
	
	if(Mix_Init(MIX_INIT_OGG))
	{
		fprintf(stderr, "\nUnable to initialize SDL_Mix Subsystem: %s\n", Mix_GetError());
	}
	
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096))
	{
		fprintf(stderr, "\nUnable to open audio mixer\n");
	}
	
	/* Create window */
	SDL_Window* window = SDL_CreateWindow(
		GAME_NAME,                                       // Window title
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  // Position (x,y)
		resolution_width, resolution_height,             // Size (x,y)
		SDL_WINDOW_RESIZABLE|
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
	
	/* Set the virtual resolution used for scaling. */
	SDL_RenderSetLogicalSize(renderer, resolution_width, resolution_height);
	
	/* Set renderer colour to black and clear window. */
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	/* Start the main loop. */
	while(1)
	{
		Return status = main_menu_loop(renderer);
		if(QUIT_PROGRAM == status)
		{
			break;
		}
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);	
	return 0;
}
