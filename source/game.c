#include <stdio.h>
#include <stdlib.h>

#include "drawable.h"
#include "status.h"
#include "constants.h"
#include "file.h"
#include "math.h"
#include "SDL_mixer.h"

static Status game_event_loop(SDL_Renderer* renderer);

#define GRID_SIZE 128
static Drawable chunk[GRID_SIZE][GRID_SIZE];
static SDL_Rect tile;
static SDL_Rect camera;

Status game_loop(SDL_Renderer* renderer)
{
	/* Camera should cover half (*2) of the full map. */
	tile.w = ((int) ((float) DESIGN_WIDTH)/((float) GRID_SIZE))*2;
	tile.h = tile.w;

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
		status = game_event_loop(renderer);

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

static Status game_event_loop(SDL_Renderer* renderer)
{
	SDL_Event event;
	while(1 == SDL_PollEvent(&event))
	{
		if(SDL_KEYDOWN == event.type)
		{
			int scancode = event.key.keysym.scancode;
			if(41 == event.key.keysym.scancode) // ESC - Close the program.
				return QUIT_PROGRAM;
			else if(scancode <= 82 && scancode >= 79)
			{
				/* >> 1 (move half a tile at a time). */
				if(80 == scancode) // left
					camera.x -= tile.w >> 1;
				else if(79 == scancode) // right
					camera.x += tile.w >> 1;
				else if(82 == scancode) // up
					camera.y -= tile.h >> 1;
				else if(81 == scancode) // down
					camera.y += tile.h >> 1;
			}
			else
				printf("Key %d pressed\n", event.key.keysym.scancode);
		}
	}
	return NORMAL;
}
