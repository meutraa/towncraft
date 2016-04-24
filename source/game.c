#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "SDL_image.h"
#include "tile.h"
#include "status.h"
#include "constants.h"
#include "options.h"
#include "drawable.h"
#include "text.h"

static void game_event_loop(void);

/*! If you would like not to kill yourself in this file.
    Cx = Tcx * Tw - (0.5 * Dw)

    Cx is camera_x is the pixel offset from 0,0 on the grid.
    Tcx is the tile [0-256][0-256] at the center of the screen.
    Tw is tile_width is the width of each tile in pixels.
    Dw is DESIGN_WIDTH is the virtual co-ordinate system.
*/

/* Milliseconds per frame .*/
static const unsigned int MSPF = (int) (((float) 1000) / ((float) 60));

#define SCANCODE_COUNT 283
#define GRID_SIZE 256
#define SQUISH_FACTOR 0.7
static float zoom_factor = 16.0f;

static Tile chunk[GRID_SIZE][GRID_SIZE];
static int key_status[SCANCODE_COUNT];

static int tile_width;
static int tile_height;

static int camera_x = 0;
static int camera_y = 0;

static Drawable* drawables;
static int count;
static const char* layout = "resources/layouts/game_ui.csv";

static TTF_Font* debug_font;
static const SDL_Color white = { 255, 255, 255, 0 };
static char fps_string[128];

static void zoom(float zoom)
{
	/* Tiles for centre we want to retain. Using floats to retain accuracy. */
	/* Get more accurate versions of tile dimensions first. */
	float acc_old_tile_width  = ((float) DESIGN_WIDTH) / ((float) GRID_SIZE) * zoom_factor;
	float acc_old_tile_height = acc_old_tile_width * (float) SQUISH_FACTOR;

	/* Cx = Tcx * Tw - (0.5 * Dw). */
	float centre_x = (camera_x + (DESIGN_WIDTH >> 1))  / acc_old_tile_width;
	float centre_y = (camera_y + (DESIGN_HEIGHT >> 1)) / acc_old_tile_height;

	/* Calculate new tile dimensions and positions. */
	zoom_factor *= zoom;
	float acc_new_tile_width  = ((float) DESIGN_WIDTH) / ((float) GRID_SIZE) * zoom_factor;
	float acc_new_tile_height = acc_new_tile_width * (float) SQUISH_FACTOR;

	/* Save int versions. */
	tile_width = (int) floor(acc_new_tile_width);
	tile_height = (int) floor(acc_new_tile_height);

	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			chunk[i][j].x = (int) round((j*(tile_width>>1)) - (i*(tile_width>>1)));
			chunk[i][j].y = (int) round((j*(tile_height>>1)) + (i*(tile_height>>1)));
		}
	}

	if(0 == zoom_mode)
	{
		camera_x = (int) floor(centre_x*acc_new_tile_width -  (0.5f * DESIGN_WIDTH ));
		camera_y = (int) floor(centre_y*acc_new_tile_height - (0.5f * DESIGN_HEIGHT));
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

	strncpy(fps_string, "0", 1);

	srand((unsigned int) time(NULL));

	debug_font = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", 16);

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
	int frames = 0;
	unsigned int start_time, dt, total_time = 0;
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
			camera_x -= (int) round(DESIGN_WIDTH * scroll_speed);
		if(1 == key_status[79] || (0 != fullscreen && 1279 == x)) // right
			camera_x += (int) round(DESIGN_WIDTH * scroll_speed);
		if(1 == key_status[82] || (0 != fullscreen && 0 == y)) // up
			camera_y -= (int) round(DESIGN_HEIGHT * scroll_speed / SQUISH_FACTOR);
		if(1 == key_status[81] || (0 != fullscreen && 719 == y)) // down
			camera_y += (int) round(DESIGN_HEIGHT * scroll_speed / SQUISH_FACTOR);


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
		render_drawables(renderer, drawables, count);

		/* Calculate the tile (x,y) in grid that is at the top left of the window. */
		char grid_pos[128];
		/* CORRECT FINALLY. */
		int tile_xpos = (int) round(camera_x / (float) tile_width);
		int tile_ypos = (int) round(camera_y / (float) tile_height);
		sprintf(grid_pos, "%d, %d", tile_xpos, tile_ypos);
		render_text(renderer, debug_font, grid_pos, white, 150, 4);

		/* Calculate the tile (x,y) in grid that is centered on screen. */
		char centre_string[128];
		int centre_x = (int) round((camera_x + (DESIGN_WIDTH >> 1)) / (float) tile_width);
		int centre_y = (int) round((camera_y + (DESIGN_HEIGHT >> 1)) / (float) tile_height);
		sprintf(centre_string, "%d, %d", centre_x , centre_y);
		render_text(renderer, debug_font, centre_string, white, 450, 4);

		/* Render FPS count. */
		render_text(renderer, debug_font, fps_string, white, 1200, 4);

		/* Draw the renderer. */
		SDL_RenderPresent(renderer);

		dt = SDL_GetTicks() - start_time;
		total_time += dt;
		if(total_time >= 1000)
		{
			// printf frames to screen;
			sprintf(fps_string, "%d", frames);
			frames = 0;
			total_time = 0;
		}
        if(0 == vsync && dt < MSPF)
        {
			//printf("Finished frame early (%d/%d)\n", dt, MSPF);
            SDL_Delay(MSPF - dt);
        }
		frames++;
	}

	/* Clean up and return to the main function. */
	SDL_DestroyTexture(tex1);
	SDL_DestroyTexture(tex2);
	TTF_CloseFont(debug_font);

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
			zoom(event.wheel.y < 0 ? 0.8f : 1.25f);
		}
	}
}
