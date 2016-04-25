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

#define SCANCODE_COUNT 283

/* The grid of tiles. */
#define GRID_SIZE 256
#define TILE_WIDTH DESIGN_WIDTH / GRID_SIZE

/* Function prototypes. */
static int inside_screen(float x, float y, float cam_x, float cam_y, float tw, float th);
static float cal_tw(float scale);
static float cal_th(float scale);
static float cal_cpx(float left);
static float cal_cpy(float left);
static float cal_ctx(float camera_x, float tile_width);
static float cal_cty(float camera_y, float tile_width);
static void calculate_tile_positions(Tile tiles[GRID_SIZE][GRID_SIZE], float tile_width, float tile_height);
static float cal_px(float centre_tile_x, float tile_width);
static float cal_py(float centre_tile_y, float tile_width);

/*! If you would like not to kill yourself in this file.
    Cx = Tcx * Tw - (0.5 * Dw)

    Cx is camera_x is the pixel offset from 0,0 on the grid.
    Tcx is the tile [0-256][0-256] at the center of the screen.
    Tw is tile_width is the width of each tile in pixels.
    Dw is DESIGN_WIDTH is the virtual co-ordinate system.
*/

/* Milliseconds per frame .*/
#define MSPF 1000 / 60

#define SQUISH_FACTOR 0.7f

Status game_loop(SDL_Renderer* renderer)
{
	Status status = NORMAL;
	SDL_Event event;

	const SDL_Color white = { 255, 255, 255, 0 };

	char fps_string[16], centre_string[32], grid_pos[32];
	int key_status[SCANCODE_COUNT];
	TTF_Font* debug_font = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", 16);

	int mouse_x, mouse_y;
	int fps = 0, frames = 0;
	unsigned int start_time, dt, total_time = 0;

	float scale = 16.0f;

	/* These are the pixel widths of each tile. */
	float tw = cal_tw(scale), th = cal_th(scale);

	Tile tiles[GRID_SIZE][GRID_SIZE];
	calculate_tile_positions(tiles, tw, th);

	/* These are the grid position in pixels of the top left of the screen. */
	float px = 0.0f, py = 0.0f;

	/* Create two generic color textures. */
	SDL_Surface* blue = IMG_Load("resources/images/rhombus-blue.tga");
	SDL_Surface* green = IMG_Load("resources/images/rhombus-green.tga");
	SDL_Texture* tex1 = SDL_CreateTextureFromSurface(renderer, blue);
	SDL_Texture* tex2 = SDL_CreateTextureFromSurface(renderer, green);
	SDL_FreeSurface(blue);
	SDL_FreeSurface(green);

	/* Enable to see how outside of camera borders is rendered. */
	//SDL_RenderSetLogicalSize(renderer, resolution_width*1.2, resolution_height*1.2);

	srand((unsigned int) time(NULL));

	/* Create the grid. */
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			int k = rand() % 2;
			tiles[i][j].texture = k == 0 ? tex1 : tex2;
		}
	}

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	count = load_drawables(renderer, &drawables, layout);

	while(NORMAL == status)
	{
		start_time = SDL_GetTicks();
		SDL_GetMouseState(&mouse_x, &mouse_y);

		/* If there are events in the event queue, process them. */
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
			else if(SDL_MOUSEWHEEL == event.type)
			{
				scale *= event.wheel.y < 0 ? 0.8f : 1.25f;
				float ctx, cty, tx, ty;

				/* Centre position in tiles (ctX) we want to retain. */
				if(0 == zoom_mode)
				{
					ctx = cal_ctx(px, tw);
					cty = cal_cty(py, th);
				}
				else if(1 == zoom_mode)
				{
					tx = (mouse_x + px) / tw;
					ty = (mouse_y + py) / th;
				}

				/* Calculate new tile dimensions and positions. */
				tw  = cal_tw(scale);
				th = cal_th(scale);

				calculate_tile_positions(tiles, tw, th);

				if(0 == zoom_mode)
				{
					/* Calculate the new top left corner in pixels. */
					px = cal_px(ctx, tw);
					py = cal_py(cty, th);
				}
				else if(1 == zoom_mode)
				{
					px = (tx * tw) - mouse_x;
					py = (ty * th) - mouse_y;
				}
			}
		}

		if(1 == key_status[41])	// ESC - Close the program.
		{
			status = QUIT_PROGRAM;
			break;
		}

		if(1 == key_status[80] || (0 != fullscreen && 0 == mouse_x)) // left
			px -= tw * scroll_speed * 60.0f/fps;
		if(1 == key_status[79] || (0 != fullscreen && 1279 == mouse_x)) // right
			px += tw * scroll_speed * 60.0f/fps;
		if(1 == key_status[82] || (0 != fullscreen && 0 == mouse_y)) // up
			py -= th * scroll_speed * 60.0f/fps;
		if(1 == key_status[81] || (0 != fullscreen && 719 == mouse_y)) // down
			py += th * scroll_speed * 60.0f/fps;

		/* Clear the screen for areas that do not have textures mapped to them. */
		/* Comment out for windows 95 mode. */
		SDL_RenderClear(renderer);

		/* Adding is a quick hack that seems to work. */
		SDL_Rect new = { 0, 0, (int) (tw + 1.5), (int) (th + 1.5) };

		for(int i = 0; i < GRID_SIZE; i++)
		{
			for(int j = 0; j < GRID_SIZE; j++)
			{
				/* Only render the drawable if it intersects with the current camera rect. */
				if(1 == inside_screen(tiles[i][j].x, tiles[i][j].y, px, py, tw, th))
				{
					new.x = (int) round(tiles[i][j].x - px);
					new.y = (int) round(tiles[i][j].y - py);
					SDL_RenderCopy(renderer, tiles[i][j].texture, NULL, &new);
				}
			}
		}
		render_drawables(renderer, drawables, count);

		/* Calculate the tile (x,y) in grid that is at the top left of the window. */
		sprintf(grid_pos, "%.1f, %.1f", px / tw, py / th);
		render_text(renderer, debug_font, grid_pos, white, 150, 4);

		/* Calculate the tile (x,y) in grid that is centered on screen. */
		sprintf(centre_string, "%.1f, %.1f", cal_ctx(px, th), cal_cty(py, th));
		render_text(renderer, debug_font, centre_string, white, 450, 4);

		/* Render FPS count. */
		sprintf(fps_string, "%d", fps);
		render_text(renderer, debug_font, fps_string, white, 1200, 4);

		/* Draw the renderer. */
		SDL_RenderPresent(renderer);

		dt = SDL_GetTicks() - start_time;
		total_time += dt;
		if(total_time >= 1000)
		{
			fps = frames;
			sprintf(fps_string, "%d", fps);
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

/* These should all be pure functions.
*/
static int inside_screen(float x, float y, float cam_x, float cam_y, float tw, float th)
{
	return x >= cam_x - tw && x <= cam_x + DESIGN_WIDTH &&
	y >= cam_y - th && y <= cam_y + DESIGN_HEIGHT;
}

static float cal_tw(float scale)
{
	return TILE_WIDTH * scale;
}

static float cal_th(float scale)
{
	return TILE_WIDTH * scale * SQUISH_FACTOR;
}

static float cal_cpx(float px)
{
	return px + (DESIGN_WIDTH >> 1);
}

static float cal_cpy(float py)
{
	return py + (DESIGN_HEIGHT >> 1);
}

/*! \fun static float centre_tile_x(float left_pixel, float tile_w)
	\brief calculates the middle position of the grid in units of tiles.

	\param left_pixel the current pixel of the grid that is rendered at x = 0
	\param tile_w the width of a tile.
	\return the grid position at the centre of the screen measured in tiles.
*/
static float cal_ctx(float px, float tw)
{
	return cal_cpx(px) / tw;
}

static float cal_cty(float py, float th)
{
	return cal_cpy(py) / th;
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

static float cal_px(float ctx, float tw)
{
	return ctx*tw - (DESIGN_WIDTH >> 1);
}

static float cal_py(float cty, float th)
{
	return cty*th - (DESIGN_HEIGHT >> 1);
}
