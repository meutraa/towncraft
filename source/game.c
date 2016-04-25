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

/* UI Drawables. */
static Drawable* drawables;
static int count;
static const char* layout = "resources/layouts/game_ui.csv";

/* Array for key states. */
#define SCANCODE_COUNT 283
static int key_status[SCANCODE_COUNT];

/* The grid of tiles. */
#define GRID_SIZE 256
#define TILE_WIDTH DESIGN_WIDTH / GRID_SIZE
static Tile tiles[GRID_SIZE][GRID_SIZE];

/* Function prototypes. */
static void game_event_loop(void);
static float calculate_tile_width(float scale);
static float calculate_tile_height(float scale);
static float centre_pixel_x(float left);
static float centre_pixel_y(float left);
static float centre_tile_x(float camera_x, float tile_width);
static float centre_tile_y(float camera_y, float tile_width);
static void calculate_tile_positions(Tile tiles[GRID_SIZE][GRID_SIZE], float tile_width, float tile_height);
static float left_pixel_x(float centre_tile_x, float tile_width);
static float top_pixel_y(float centre_tile_y, float tile_width);
static void zoom(float zoom);

/*! If you would like not to kill yourself in this file.
    Cx = Tcx * Tw - (0.5 * Dw)

    Cx is camera_x is the pixel offset from 0,0 on the grid.
    Tcx is the tile [0-256][0-256] at the center of the screen.
    Tw is tile_width is the width of each tile in pixels.
    Dw is DESIGN_WIDTH is the virtual co-ordinate system.
*/

/* Milliseconds per frame .*/
static const unsigned int MSPF = (int) (((float) 1000) / ((float) 60));

#define SQUISH_FACTOR 0.7f
static float current_scale = 16.0f;

static float tile_width;
static float tile_height;

static float camera_x = 0.0f;
static float camera_y = 0.0f;

static TTF_Font* debug_font;
static const SDL_Color white = { 255, 255, 255, 0 };
static char fps_string[128] = { [0] = '6', [1] = '0', [2] = '\0' };

Status game_loop(SDL_Renderer* renderer)
{
	/* Create three generic color textures. */
	SDL_Surface* blue = IMG_Load("resources/images/rhombus-blue.tga");
	SDL_Surface* green = IMG_Load("resources/images/rhombus-green.tga");
	SDL_Texture* tex1 = SDL_CreateTextureFromSurface(renderer, blue);
	SDL_Texture* tex2 = SDL_CreateTextureFromSurface(renderer, green);
	SDL_FreeSurface(blue);
	SDL_FreeSurface(green);

	srand((unsigned int) time(NULL));

	debug_font = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", 16);

	/* Create the grid. */
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			int k = rand() % 2;
			tiles[i][j].texture = k == 0 ? tex1 : tex2;
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
			camera_x -= tile_width * scroll_speed * 60.0f/atof(fps_string);
		if(1 == key_status[79] || (0 != fullscreen && 1279 == x)) // right
			camera_x += tile_width * scroll_speed * 60.0f/atof(fps_string);
		if(1 == key_status[82] || (0 != fullscreen && 0 == y)) // up
			camera_y -= tile_height * scroll_speed * 60.0f/atof(fps_string);
		if(1 == key_status[81] || (0 != fullscreen && 719 == y)) // down
			camera_y += tile_height * scroll_speed * 60.0f/atof(fps_string);

		/* Clear the screen for areas that do not have textures mapped to them. */
		/* Comment out for windows 95 mode. */
		SDL_RenderClear(renderer);

		/* Adding is a quick hack that seems to work. */
		SDL_Rect new = { 0, 0, (int) (tile_width + 1.5), (int) (tile_height + 1.5) };

		for(int i = 0; i < GRID_SIZE; i++)
		{
			for(int j = 0; j < GRID_SIZE; j++)
			{
				/* Only render the drawable if it intersects with the current camera rect. */
				if(tiles[i][j].x >= camera_x - tile_width &&
				   tiles[i][j].x <= camera_x + DESIGN_WIDTH + tile_width &&
				   tiles[i][j].y >= camera_y - tile_height &&
				   tiles[i][j].y <= camera_y + DESIGN_HEIGHT + tile_width)
				{
					new.x = (int) round(tiles[i][j].x - camera_x);
					new.y = (int) round(tiles[i][j].y - camera_y);
					SDL_RenderCopy(renderer, tiles[i][j].texture, NULL, &new);
				}
			}
		}
		render_drawables(renderer, drawables, count);

		/* Calculate the tile (x,y) in grid that is at the top left of the window. */
		char grid_pos[128];
		sprintf(grid_pos, "%.1f, %.1f", camera_x / tile_width, camera_y / tile_height);
		render_text(renderer, debug_font, grid_pos, white, 150, 4);

		/* Calculate the tile (x,y) in grid that is centered on screen. */
		char centre_string[128];
		sprintf(centre_string,
				"%.1f, %.1f",
		 		centre_tile_x(camera_x, tile_width),
				centre_tile_y(camera_y, tile_height));
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
			frames = -1;
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

/* These should all be pure functions.
*/
static float calculate_tile_width(float scale)
{
	return TILE_WIDTH * scale;
}

static float calculate_tile_height(float scale)
{
	return TILE_WIDTH * scale * SQUISH_FACTOR;
}

static float centre_pixel_x(float left)
{
	return left + (DESIGN_WIDTH >> 1);
}

static float centre_pixel_y(float top)
{
	return top + (DESIGN_HEIGHT >> 1);
}

/*! \fun static float centre_tile_x(float left_pixel, float tile_w)
	\brief calculates the middle position of the grid in units of tiles.

	\param left_pixel the current pixel of the grid that is rendered at x = 0
	\param tile_w the width of a tile.
	\return the grid position at the centre of the screen measured in tiles.
*/
static float centre_tile_x(float left_pixel, float tile_w)
{
	return centre_pixel_x(left_pixel) / tile_w;
}

static float centre_tile_y(float top_pixel, float tile_h)
{
	return centre_pixel_y(top_pixel) / tile_h;
}

static void calculate_tile_positions(Tile t[GRID_SIZE][GRID_SIZE], float tw, float th)
{
	const float w = 0.5f * tw;
	const float h = 0.5f * th;
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			t[i][j].x = w*(float)(j - i);
			t[i][j].y = h*(float)(j + i);
		}
	}
}

/* center_x is in tiles, result is in pixels. */
static float left_pixel_x(float centre_tile_x, float tw)
{
	return centre_tile_x*tw - (DESIGN_WIDTH >> 1);
}

static float top_pixel_y(float centre_tile_y, float th)
{
	return centre_tile_y*th - (DESIGN_HEIGHT >> 1);
}

static void zoom(float zoom)
{
	/* Tiles for centre we want to retain. Using floats to retain accuracy. */
	float ctx = centre_tile_x(camera_x, calculate_tile_width(current_scale));
	float cty = centre_tile_y(camera_y, calculate_tile_height(current_scale));

	/* Calculate new tile dimensions and positions. */
	current_scale *= zoom;
	tile_width  = calculate_tile_width(current_scale);
	tile_height = calculate_tile_height(current_scale);

	calculate_tile_positions(tiles, tile_width, tile_height);

	if(0 == zoom_mode)
	{
		camera_x = left_pixel_x(ctx, tile_width);
		camera_y = top_pixel_y (cty, tile_height);
	}
}
