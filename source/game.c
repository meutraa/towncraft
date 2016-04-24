#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tile.h"
#include "status.h"
#include "constants.h"
#include "file.h"
#include "math.h"
#include "options.h"
#include "drawable.h"
#include "text.h"
#include "SDL_mixer.h"
#include "SDL_image.h"

static void game_event_loop();

#define round(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

/* Milliseconds per frame .*/
static const unsigned int MSPF = (int) (((float) 1000) / ((float) 60));

#define SCANCODE_COUNT 283
#define GRID_SIZE 1024
#define SQUISH_FACTOR 0.65
static float zoom_factor = 16.0f;

static Tile chunk[GRID_SIZE][GRID_SIZE];
static int key_status[SCANCODE_COUNT];

static int tile_width;
static int tile_height;

static float camera_x = 0;
static float camera_y = 0;

static Drawable* drawables;
static int count;
static char* layout = "resources/layouts/game_ui.csv";

static SDL_Color text_color = {255,255,255,0};

static void zoom(float zoom)
{
	zoom_factor *= zoom;
	tile_width = round((((float) DESIGN_WIDTH)/((float) GRID_SIZE))*zoom_factor);
	tile_height = round(tile_width*SQUISH_FACTOR);

	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			float scale = 0.5;
			chunk[i][j].x = (j*tile_width*scale) - (i*tile_width*scale);
			chunk[i][j].y = (j*tile_height*scale) + (i*tile_height*scale);
		}
	}

	if(0 == zoom_mode)
	{
		camera_x += DESIGN_WIDTH * (zoom - 1) * 0.5;
		camera_y += DESIGN_HEIGHT * (zoom - 1) * 0.5;
	}
}

Status game_loop(SDL_Renderer* renderer)
{
	/* Create three generic color textures. */
	SDL_Surface* blue = IMG_Load("resources/images/rhombus-blue.tga");
	SDL_Surface* green = IMG_Load("resources/images/rhombus-green.tga");
	SDL_Texture* tex1 = SDL_CreateTextureFromSurface(renderer, blue);
	SDL_Texture* tex2 = SDL_CreateTextureFromSurface(renderer, green);
	SDL_FreeSurface(blue);
	SDL_FreeSurface(green);

	srand(time(NULL));

	/* Create the grid. */
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			int k = rand() % 2;
			chunk[i][j].texture = k == 0 ? tex1 : tex2;
		}
	}

	zoom(1.0);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	count = load_drawables(renderer, &drawables, layout);

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
			camera_x -= tile_width * 0.5;
		if(1 == key_status[79] || (0 != fullscreen && 1279 == x)) // right
			camera_x += tile_width * 0.5;
		if(1 == key_status[82] || (0 != fullscreen && 0 == y)) // up
			camera_y -= tile_height * 0.5;
		if(1 == key_status[81] || (0 != fullscreen && 719 == y)) // down
			camera_y += tile_height * 0.5;


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
					new.x = round(chunk[i][j].x - camera_x);
					new.y = round(chunk[i][j].y - camera_y);
					SDL_RenderCopy(renderer, chunk[i][j].texture, NULL, &new);
				}
			}
		}
		render_drawables(renderer, drawables, count);

		char text_string[128];
		sprintf(text_string, "%.1f, %.1f", camera_x, camera_y);
		render_text(renderer, text_string, 16, text_color, 150, 1);

		/* Draw the renderer. */
		SDL_RenderPresent(renderer);

		dt = SDL_GetTicks() - start_time;
        if(dt < MSPF)
        {
			//printf("Finished frame early (%d/%d)\n", dt, MSPF);
            SDL_Delay(MSPF - dt);
        }
	}

	/* Clean up and return to the main function. */
	SDL_DestroyTexture(tex1);
	SDL_DestroyTexture(tex2);

	destroy_drawables(&drawables, count);

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
			zoom(event.wheel.y < 0 ? 0.9 : 1.1);
		}
	}
}
