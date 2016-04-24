#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tile.h"
#include "status.h"
#include "constants.h"
#include "file.h"
#include "math.h"
#include "options.h"
#include "SDL_mixer.h"

static void game_event_loop();

/* Milliseconds per frame .*/
static const int MSPF = (int) (((float) 1000) / ((float) 60));

#define SCANCODE_COUNT 283
#define GRID_SIZE 1024
#define SQUISH_FACTOR 0.65
static float zoom_factor = 16.0f;

static Tile chunk[GRID_SIZE][GRID_SIZE];
static int key_status[SCANCODE_COUNT];

static int tile_width;
static int tile_height;

static int camera_x = 0;
static int camera_y = 0;

static void zoom(float zoom)
{
	tile_width = (int) ((((float) DESIGN_WIDTH)/((float) GRID_SIZE))*zoom);
	tile_height = tile_width*SQUISH_FACTOR;

	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			chunk[i][j].x = i*tile_width;
			chunk[i][j].y = j*tile_height;
		}
	}
}

Status game_loop(SDL_Renderer* renderer)
{
	/* Create three generic color textures. */
	SDL_Surface* surface = SDL_CreateRGBSurface(0, 1, 1, 8, 0, 0, 0, 0);
	SDL_SetSurfaceColorMod(surface, 128, 255, 64);
	SDL_Texture* tex1 = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_SetSurfaceColorMod(surface, 128, 64, 255);
	SDL_Texture* tex2 = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_SetSurfaceColorMod(surface, 64, 128, 255);
	SDL_Texture* tex3 = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);


	srand(time(NULL));

	/* Create the grid. */
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			int k = rand() % 3;
			chunk[i][j].texture = k == 0 ? tex1 : k == 1 ? tex2 : tex3;
		}
	}

	zoom(zoom_factor);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	Status status = NORMAL;
	unsigned int start_time, dt;
	while(NORMAL == status)
	{
		start_time = SDL_GetTicks();
		/* If there are events in the event queue, process them. */
		game_event_loop();

		int x, y;
		SDL_GetMouseState(&x, &y);

		if(1 == key_status[41])	// ESC - Close the program.
		{
			status = QUIT_PROGRAM;
			break;
		}
		if(1 == key_status[80] || (0 != fullscreen && 0 == x)) // left
			camera_x -= tile_width >> 1;
		if(1 == key_status[79] || (0 != fullscreen && 1279 == x)) // right
			camera_x += tile_width >> 1;
		if(1 == key_status[82] || (0 != fullscreen && 0 == y)) // up
			camera_y -= tile_height >> 1;
		if(1 == key_status[81] || (0 != fullscreen && 719 == y)) // down
			camera_y += tile_height >> 1;


		/* Clear the screen for areas that do not have textures mapped to them. */
		/* Comment out for windows 95 mode. */
		SDL_RenderClear(renderer);

		SDL_Rect new = { 0, 0, tile_width, tile_height };

		for(int i = 0; i < GRID_SIZE; i++)
		{
			for(int j = 0; j < GRID_SIZE; j++)
			{
				/* Only render the drawable if it intersects with the current camera rect. */
				if(chunk[i][j].x >= camera_x - tile_width &&
				   chunk[i][j].x <= camera_x + DESIGN_WIDTH + tile_width &&
				   chunk[i][j].y >= camera_y - tile_height &&
				   chunk[i][j].y <= camera_y + DESIGN_HEIGHT + tile_width)
				{
					new.x = chunk[i][j].x - camera_x;
					new.y = chunk[i][j].y - camera_y;
					SDL_RenderCopy(renderer, chunk[i][j].texture, NULL, &new);
				}
			}
		}

		/* Draw the renderer. */
		SDL_RenderPresent(renderer);

		dt = SDL_GetTicks() - start_time;
        if(dt < MSPF)
        {
			printf("Finished frame early (%d/%d)\n", dt, MSPF);
            SDL_Delay(MSPF - dt);
        }
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
		if(SDL_KEYDOWN == event.type)
		{
			key_status[event.key.keysym.scancode] = 1;
			//printf("Key %d pressed\n", event.key.keysym.scancode);
		}
		else if(SDL_KEYUP == event.type)
		{
			key_status[event.key.keysym.scancode] = 0;
		}
		if(SDL_MOUSEWHEEL == event.type)
		{
			zoom_factor *= event.wheel.y < 0 ? 0.8 : 1.2;
			zoom(zoom_factor);
		}
	}
}
