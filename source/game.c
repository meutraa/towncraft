#include <stdio.h>
#include <stdlib.h>

#include "drawable.h"
#include "status.h"
#include "constants.h"
#include "file.h"
#include "math.h"
#include "options.h"
#include "SDL_mixer.h"

static void game_event_loop();

#define SCANCODE_COUNT 283
#define GRID_SIZE 1024
#define SQUISH_FACTOR 0.65
#define DEFAULT_ZOOM 16.0f

static Drawable chunk[GRID_SIZE][GRID_SIZE];
static SDL_Rect tile;
static SDL_Rect camera;
static int key_status[SCANCODE_COUNT];

static void zoom(float zoom)
{
	tile.w = ((int) (((float) DESIGN_WIDTH)/((float) GRID_SIZE))*zoom);
	tile.h = tile.w*SQUISH_FACTOR;
}

Status game_loop(SDL_Renderer* renderer)
{
	/* Camera should cover half (*2) of the full map. */
	zoom(DEFAULT_ZOOM);

	/* Create three generic color textures. */
	SDL_Surface* surface = SDL_CreateRGBSurface(0, tile.w, tile.h, 8, 0, 0, 0, 0);
	SDL_SetSurfaceColorMod(surface, 128, 255, 64);
	SDL_Texture* tex1 = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_SetSurfaceColorMod(surface, 128, 64, 255);
	SDL_Texture* tex2 = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_SetSurfaceColorMod(surface, 64, 128, 255);
	SDL_Texture* tex3 = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	/* Create the grid. */
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			SDL_Rect wide;
			wide.w = tile.w;
			wide.h = tile.h;
			wide.x = i*tile.w;
			wide.y = j*tile.h;
			chunk[i][j].widescreen = wide;
			chunk[i][j].rect = &chunk[i][j].widescreen;
			chunk[i][j].texture = ((i*j) % 3 == 0) ? tex1 : ((i*j) % 2 == 0) ? tex2: tex3;
		}
	}

	camera.w = DESIGN_WIDTH;
	camera.h = DESIGN_HEIGHT;
	camera.x = 0;
	camera.y = 0;

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	Status status = NORMAL;
	while(NORMAL == status)
	{
		/* If there are events in the event queue, process them. */
		game_event_loop();

		int x, y;
		SDL_GetMouseState(&x, &y);
		printf("%d, %d\n", x, y);

		if(1 == key_status[41])	// ESC - Close the program.
		{
			status = QUIT_PROGRAM;
			break;
		}
		if(1 == key_status[80]) // left
			camera.x -= tile.w >> 1;
		if(1 == key_status[79]) // right
			camera.x += tile.w >> 1;
		if(1 == key_status[82]) // up
			camera.y -= tile.h >> 1;
		if(1 == key_status[81]) // down
			camera.y += tile.h >> 1;

		if(0 != fullscreen)
		{
			if(0 == x) // left
				camera.x -= tile.w >> 1;
			if(1279 == x) // right
				camera.x += tile.w >> 1;
			if(0 == y) // up
				camera.y -= tile.h >> 1;
			if(719 == y) // down
				camera.y += tile.h >> 1;
		}


		/* Clear the screen for areas that do not have textures mapped to them. */
		/* Comment out for windows 95 mode. */
		SDL_RenderClear(renderer);

		for(int i = 0; i < GRID_SIZE; i++)
		{
			for(int j = 0; j < GRID_SIZE; j++)
			{
				/* Only render the drawable if it intersects with the current camera rect. */
				if(SDL_TRUE == SDL_HasIntersection(chunk[i][j].rect, &camera))
				{
					SDL_Rect new = {
						chunk[i][j].rect->x - camera.x,
						chunk[i][j].rect->y - camera.y,
						chunk[i][j].rect->w,
						chunk[i][j].rect->h
					};
					SDL_RenderCopy(renderer, chunk[i][j].texture, NULL, &new);
				}
			}
		}

		/* Draw the renderer. */
		SDL_RenderPresent(renderer);
	}

	/* Clean up and return to the main function. */
	SDL_DestroyTexture(tex1);
	SDL_DestroyTexture(tex2);
	SDL_DestroyTexture(tex3);
	return status;
}

static void game_event_loop()
{
	SDL_Event event;
	while(1 == SDL_PollEvent(&event))
	{
		int scancode = event.key.keysym.scancode;

		if(SDL_KEYDOWN == event.type)
		{
			key_status[scancode] = 1;
			printf("Key %d pressed\n", event.key.keysym.scancode);
		}
		else if(SDL_KEYUP == event.type)
		{
			key_status[scancode] = 0;
		}
	}
}
