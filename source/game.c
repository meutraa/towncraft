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

#define printbuf(x, y) render_text(renderer, debug_font, strbuf, white, x, y);
#define SCANCODE_COUNT 283

/* The grid of tiles. */
#define GRID_SIZE 256

/* Function prototypes. */
static int inside_screen(float x, float y, float cam_x, float cam_y, float tw, float th);
static float cal_tw(float scale);
static float cal_th(float scale);
static float cal_tx(float grid_offset_pixels, float tw, float screen_offset_pixels);
static float cal_ty(float grid_offset_pixels, float th, float screen_offset_pixels);
static void calculate_tile_positions(Tile t[GRID_SIZE][GRID_SIZE], float tw, float th);
static float cal_px(float grid_offset_tiles, float tw, float screen_offset_tiles);
static float cal_py(float grid_offset_tiles, float th, float screen_offset_tiles);

/* Milliseconds per frame .*/
#define MSPF 1000 / 60

Status game_loop(SDL_Renderer* renderer)
{
	Status status = NORMAL;
	SDL_Event event;

	const SDL_Color white = { 255, 255, 255, 0 };

	char strbuf[32];
	int key_status[SCANCODE_COUNT] = {0};
	TTF_Font* debug_font = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", 16);

	int mouse_x, mouse_y;

	/* Assume 60 for scroll speed to not become infinity. */
	int fps = 60, frames = 0;
	unsigned int start_time, dt, total_time = 0;

	float scale = 16.0f;

	/* These are the pixel widths of each tile. */
	float tw = cal_tw(scale), th = cal_th(scale);

	Tile tiles[GRID_SIZE][GRID_SIZE];
	calculate_tile_positions(tiles, tw, th);

	/* These are the grid position in pixels of the top left of the screen. */
	float px = 0.0f, py = 0.0f;

	/* Create two generic color textures. */
	const char* sprite_images[] = {
		"resources/images/building-mega.tga",
		"resources/images/building-1.tga",
		"resources/images/building-2.tga",
		"resources/images/building-2-1.tga",
		"resources/images/building-2-2.tga",
		"resources/images/building-2-3.tga",
		"resources/images/building-2-4.tga",
	};

	const char* tile_images[] = {
		"resources/images/terrain-water-test.tga",
		"resources/images/terrain-grass-test.tga",
	};
	int sprite_length = 7;
	int tile_length = 2;

	SDL_Texture* sprite_textures[sprite_length];
	SDL_Texture* tile_textures[tile_length];

	for(int i = 0; i < sprite_length; i++)
	{
		SDL_Surface* s = IMG_Load(sprite_images[i]);
		sprite_textures[i] = SDL_CreateTextureFromSurface(renderer, s);
		SDL_FreeSurface(s);
	}
	for(int i = 0; i < tile_length; i++)
	{
		SDL_Surface* s = IMG_Load(tile_images[i]);
		tile_textures[i] = SDL_CreateTextureFromSurface(renderer, s);
		SDL_FreeSurface(s);
	}

	/* Enable to see how outside of camera borders is rendered. */
	//SDL_RenderSetLogicalSize(renderer, resolution_width*2, resolution_height*2);

	srand((unsigned int) time(NULL));

	/* Create the grid. */
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			/* If connected to land, and roll was water, reroll. */
			int l = rand() % (tile_length + 10);
			l = (l < 9) ? 1 : 0;
			if(j > 0)
			{
				if(l != tiles[i][j - 1].tile_id) l = rand() % tile_length;
				if(l != tiles[i][j - 1].tile_id) l = rand() % tile_length;
			}
			if(i > 0)
			{
				if(l != tiles[i - 1][j].tile_id) l = rand() % tile_length;
				if(l != tiles[i - 1][j].tile_id) l = rand() % tile_length;
			}
			if(i > 0 && j > 0)
			{
				if(l != tiles[i - 1][j - 1].tile_id) l = rand() % tile_length;
			}
			tiles[i][j].tile_texture = tile_textures[l];
			tiles[i][j].tile_id = l;
			/* If water, do not place a building. */
			if(0 == l)
			{
				tiles[i][j].sprite_texture = NULL;
			}
			else
			{
				int k = rand() % (sprite_length << 2);
				if(0 == k) k = rand() % 4;
				tiles[i][j].sprite_texture = k < sprite_length ? sprite_textures[k] : NULL;
			}
		}
	}

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

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
				float new_scale = scale * ((event.wheel.y < 0) ? 0.5f : 2.0f);
				if(new_scale >= 2.0f && new_scale <= 64.0f)
				{
					scale = new_scale;
					float offset_x = (0 == zoom_mode) ? DESIGN_WIDTH >> 1 : (float)mouse_x;
					float offset_y = (0 == zoom_mode) ? DESIGN_HEIGHT >> 1 : (float)mouse_y;

					/* Position in tiles we want to retain. */
					float tx = cal_tx(px, tw, offset_x);
					float ty = cal_ty(py, th, offset_y);

					/* Calculate new tile dimensions and positions. */
					tw = cal_tw(scale);
					th = cal_th(scale);

					calculate_tile_positions(tiles, tw, th);

					/* Calculate the new top left corner in pixels. */
					px = cal_px(tx, tw, offset_x);
					py = cal_py(ty, th, offset_y);
				}
			}
		}

		if(1 == key_status[41])	// ESC - Close the program.
		{
			status = QUIT_PROGRAM;
			break;
		}

		if(1 == key_status[80] || (0 != fullscreen && 0 == mouse_x)) // left
			px -= tw * scroll_speed * 60.0f/(float)fps;
		if(1 == key_status[79] || (0 != fullscreen && 1279 == mouse_x)) // right
			px += tw * scroll_speed * 60.0f/(float)fps;
		if(1 == key_status[82] || (0 != fullscreen && 0 == mouse_y)) // up
			py -= th * 0.25 * scroll_speed * 60.0f/(float)fps;
		if(1 == key_status[81] || (0 != fullscreen && 719 == mouse_y)) // down
			py += th * 0.25 * scroll_speed * 60.0f/(float)fps;

		/* Clear the screen for areas that do not have textures mapped to them. */
		/* Comment out for windows 95 mode. */
		SDL_RenderClear(renderer);

		/* Adding is a quick hack that seems to work. */
		SDL_Rect new = { 0, 0, (int)tw, (int)th };

		for(int i = 0; i < GRID_SIZE; i++)
		{
			for(int j = 0; j < GRID_SIZE; j++)
			{
				/* Only render the drawable if it intersects with the current camera rect. */
				if(1 == inside_screen(tiles[i][j].x, tiles[i][j].y, px, py, tw, th))
				{
					new.x = (int) floor(tiles[i][j].x - px);
					new.y = (int) floor(tiles[i][j].y - py);
					SDL_RenderCopy(renderer, tiles[i][j].tile_texture, NULL, &new);
					if(NULL != tiles[i][j].sprite_texture) SDL_RenderCopy(renderer, tiles[i][j].sprite_texture, NULL, &new);
				}
			}
		}
		render_drawables(renderer, drawables, count);

		/* Print the UI debugging infomation. */
		sprintf(strbuf, "%.1f, %.1f", px, py);
		printbuf(200, 680);
		sprintf(strbuf, "%.1f, %.1f", cal_tx(px, tw, 0), cal_ty(py, th, 0));
		printbuf(200, 700);

		sprintf(strbuf, "%.1f, %.1f", px + (DESIGN_WIDTH >> 1), py + (DESIGN_HEIGHT >> 1));
		printbuf(583, 680);
		sprintf(strbuf, "%.1f, %.1f", cal_tx(px, tw, DESIGN_WIDTH >> 1), cal_ty(py, th, DESIGN_HEIGHT >> 1));
		printbuf(583, 700);

		sprintf(strbuf, "%.1f, %.1f", px + (float)mouse_x, py + (float)mouse_y);
		printbuf(974, 680);
		sprintf(strbuf, "%.1f, %.1f", cal_tx(px, tw, (float)mouse_x), cal_tx(py, th, (float)mouse_y));
		printbuf(974, 700);

		sprintf(strbuf, "%.1f", tw);
		printbuf(1210, 680);
		sprintf(strbuf, "%.1f", th);
		printbuf(1210, 700);

		sprintf(strbuf, "%.1f", scale);
		printbuf(1080, 4);
		sprintf(strbuf, "%d", fps);
		printbuf(1200, 4);

		//SDL_RenderCopy(renderer, castle_tex, NULL, &castle_rect);

		/* Draw the renderer. */
		SDL_RenderPresent(renderer);

		dt = SDL_GetTicks() - start_time;
		total_time += dt;
		if(total_time >= 1000)
		{
			fps = frames;
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
	for(int i = 0; i < sprite_length; i++)
	{
		SDL_DestroyTexture(sprite_textures[i]);
	}
	for(int i = 0; i < tile_length; i++)
	{
		SDL_DestroyTexture(tile_textures[i]);
	}
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
	return scale / 16.0f * 128.0f;
}

static float cal_th(float scale)
{
	return scale / 16.0f * 256.0f;
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
			t[i][j].y = h*0.25f*(float)(j + i);
		}
	}
}

static float cal_tx(float grid_offset_pixels, float tw, float screen_offset_pixels)
{
	return (screen_offset_pixels + grid_offset_pixels) / tw;
}

static float cal_ty(float grid_offset_pixels, float th, float screen_offset_pixels)
{
	return (screen_offset_pixels + grid_offset_pixels) / th;
}

static float cal_px(float grid_offset_tiles, float tw, float screen_offset_tiles)
{
	return grid_offset_tiles*tw - screen_offset_tiles;
}

static float cal_py(float grid_offset_tiles, float th, float screen_offset_tiles)
{
	return grid_offset_tiles*th - screen_offset_tiles;
}
