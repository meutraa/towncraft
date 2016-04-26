#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "SDL_image.h"
#include "constants.h"
#include "drawable.h"
#include "options.h"
#include "status.h"
#include "text.h"
#include "tile.h"

static const char* sprite_images[] = {
    "resources/images/building-mega.tga",
    "resources/images/building-1.tga",
    "resources/images/building-2.tga",
    "resources/images/building-2-1.tga",
    "resources/images/building-2-2.tga",
    "resources/images/building-2-3.tga",
    "resources/images/building-2-4.tga",
};

static const char* tile_images[] = {
    "resources/images/terrain-water-test.tga",
    "resources/images/terrain-grass-test.tga",
};

static const int SPRITE_LENGTH = 7;
static const int TILE_LENGTH = 2;

static const SDL_Color white = { 255, 255, 255, 0 };

#define printbuf(x, y) render_text(renderer, debug_font, strbuf, white, x, y);
#define SCANCODE_COUNT 283

/* The grid of tiles. */
static const int GRID_SIZE = 256;
static const float TILE_WIDTH = 128.0f;
static const float TILE_HEIGHT = 64.0f;

/* Function prototypes. */
static float pixel_to_tile_x(float px, float py, float tw, float th);
static float pixel_to_tile_y(float px, float py, float tw, float th);
static float tile_to_pixel_x(float tx, float ty, float tw);
static float tile_to_pixel_y(float tx, float ty, float th);
static void calculate_tile_positions(Tile t[GRID_SIZE][GRID_SIZE], float tw, float th);

Status game_loop(SDL_Renderer* renderer)
{
    Status status = NORMAL;
    SDL_Event event;

    char strbuf[32];
    int key_status[SCANCODE_COUNT] = { 0 };
    TTF_Font* debug_font = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", 16);

    int mouse_x, mouse_y;

    /* Assume 60 for scroll speed to not become infinity. */
    int fps = 60, frames = 0;
    unsigned int start_time;

    /* These are the pixel widths of each tile. */
    float scale = 1.0f, tw = TILE_WIDTH, th = TILE_HEIGHT;

    Tile tiles[GRID_SIZE][GRID_SIZE];
    calculate_tile_positions(tiles, tw, th);

    /* These are the grid position in pixels of the top left of the screen. */
    float px = 0.0f, py = 0.0f;

    SDL_Texture* sprite_textures[SPRITE_LENGTH];
    SDL_Texture* tile_textures[TILE_LENGTH];

    for (int i = 0; i < SPRITE_LENGTH; i++)
    {
        SDL_Surface* s = IMG_Load(sprite_images[i]);
        sprite_textures[i] = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
    }

    for (int i = 0; i < TILE_LENGTH; i++)
    {
        SDL_Surface* s = IMG_Load(tile_images[i]);
        tile_textures[i] = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
    }

    /* Enable to see how outside of camera borders is rendered. */
    //SDL_RenderSetLogicalSize(renderer, resolution_width*2, resolution_height*2);

    srand((unsigned int)time(NULL));

    /* Create the grid. */
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            /* If connected to land, and roll was water, reroll. */
            int l = rand() % (TILE_LENGTH + 10);
            l = (l < 9) ? 1 : 0;
            if (i > 0 && j > 0)
            {
                if (l != tiles[i - 1][j - 1].tile_id)
                {
                    l = rand() % TILE_LENGTH;
                }
            }
            tiles[i][j].tile_texture = tile_textures[l];
            tiles[i][j].tile_id = l;
            /* If water, do not place a building. */
            if (0 == l)
            {
                tiles[i][j].sprite_texture = NULL;
            }
            else
            {
                int k = rand() % (SPRITE_LENGTH << 2);
                if (0 == k)
                {
                    k = rand() % 4;
                }
                tiles[i][j].sprite_texture = k < SPRITE_LENGTH ? sprite_textures[k] : NULL;
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

    /* Load in the UI. */
    const char* layout = "resources/layouts/game_ui.csv";
    int count = count_drawables(layout);
    Drawable drawables[count];
    load_drawables(renderer, drawables, layout, 0);

    while (NORMAL == status)
    {
        SDL_GetMouseState(&mouse_x, &mouse_y);

        /* If there are events in the event queue, process them. */
        while (1 == SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                {
                    key_status[event.key.keysym.scancode] = 1;
                    // printf("Key %d pressed\n", event.key.keysym.scancode);
                    break;
                }
                case SDL_KEYUP:
                {
                    key_status[event.key.keysym.scancode] = 0;
                    break;
                }
                case SDL_MOUSEWHEEL:
                {
                    float new_scale = scale * ((event.wheel.y < 0) ? 0.5f : 2.0f);
                    if (new_scale >= 0.125f && new_scale <= 8.0f)
                    {
                        scale = new_scale;
                        float offset_x = (0 == zoom_mode) ? DESIGN_WIDTH >> 1 : (float)mouse_x;
                        float offset_y = (0 == zoom_mode) ? DESIGN_HEIGHT >> 1 : (float)mouse_y;

                        // Position in tiles we want to retain.
                        float tx = pixel_to_tile_x(px + offset_x, py + offset_y, tw, th);
                        float ty = pixel_to_tile_y(px + offset_x, py + offset_y, tw, th);

                        // Calculate new tile dimensions and positions.
                        tw = scale * TILE_WIDTH;
                        th = scale * TILE_HEIGHT;

                        calculate_tile_positions(tiles, tw, th);

                        // Calculate the new top left corner in pixels.
                        px = tile_to_pixel_x(tx, ty, tw) - offset_x;
                        py = tile_to_pixel_y(tx, ty, th) - offset_y;
                    }
                    break;
                }
            }
        }

        if (1 == key_status[41]) // ESC - Close the program.
        {
            status = QUIT_PROGRAM;
            break;
        }

        float sw = tw * scroll_speed * 60.0f / (float)fps;
        float sh = th * scroll_speed * 60.0f / (float)fps;
        if (1 == key_status[80] || (0 != fullscreen && 0 == mouse_x)) // left
        {
            px -= sw;
        }
        if (1 == key_status[79] || (0 != fullscreen && 1279 == mouse_x)) // right
        {
            px += sw;
        }
        if (1 == key_status[82] || (0 != fullscreen && 0 == mouse_y)) // up
        {
            py -= sh;
        }
        if (1 == key_status[81] || (0 != fullscreen && 719 == mouse_y)) // down
        {
            py += sh;
        }

        /* Clear the screen for areas that do not have textures mapped to them. */
        /* Comment out for windows 95 mode. */
        SDL_RenderClear(renderer);

        SDL_Rect new = { 0, 0, (int)tw, (int)th };
        SDL_Rect newb = { 0, 0, (int)tw, (int)(th * 4.0f) };

        /* Lowest y we need to rend it top right corner. */
        int y1 = (int)floor(pixel_to_tile_y(px + DESIGN_WIDTH, py, tw, th));
        y1 = (y1 < 0) ? 0 : y1;

        /* Highest y is bottom left. */
        int y2 = (int)ceil(pixel_to_tile_y(px, py + DESIGN_HEIGHT, tw, th));
        y2 = (y2 > GRID_SIZE) ? GRID_SIZE : y2;

        /* Lowest x we need is top left corner. */
        int x1 = (int)floor(pixel_to_tile_x(px, py, tw, th));
        x1 = (x1 < 0) ? 0 : x1;

        /* Highest x is at bottom right. */
        int x2 = (int)ceil(pixel_to_tile_x(px + DESIGN_WIDTH, py + DESIGN_HEIGHT, tw, th));
        x2 = (x2 > GRID_SIZE) ? GRID_SIZE : x2;

        for (int y = y1; y < y2; y++)
        {
            for (int x = x1; x < x2; x++)
            {
                float newy = tiles[x][y].y - py;
                new.x = (int)floor(tiles[x][y].x - px);
                new.y = (int)floor(newy);
                newb.x = new.x;
                newb.y = (int)floor(newy - 3.0f * th);
                SDL_RenderCopy(renderer, tiles[x][y].tile_texture, NULL, &new);
                if (NULL != tiles[x][y].sprite_texture)
                {
                    SDL_RenderCopy(renderer, tiles[x][y].sprite_texture, NULL, &newb);
                }
            }
        }

        render_drawables(renderer, drawables, count);

        /* Print the UI debugging infomation. */
        sprintf(strbuf, "%.1f, %.1f", px, py);
        printbuf(200, 680);
        sprintf(strbuf, "%.1f, %.1f", pixel_to_tile_x(px, py, tw, th), pixel_to_tile_y(px, py, tw, th));
        printbuf(200, 700);

        sprintf(strbuf, "%.1f, %.1f", px + (DESIGN_WIDTH >> 1), py + (DESIGN_HEIGHT >> 1));
        printbuf(583, 680);
        sprintf(strbuf, "%.1f, %.1f",
            pixel_to_tile_x(px + (DESIGN_WIDTH >> 1), py + (DESIGN_HEIGHT >> 1), tw, th),
            pixel_to_tile_y(px + (DESIGN_WIDTH >> 1), py + (DESIGN_HEIGHT >> 1), tw, th));
        printbuf(583, 700);

        sprintf(strbuf, "%.1f, %.1f", px + (float)mouse_x, py + (float)mouse_y);
        printbuf(974, 680);
        sprintf(strbuf, "%.1f, %.1f",
            pixel_to_tile_x(px + (float)mouse_x, py + (float)mouse_y, tw, th),
            pixel_to_tile_y(px + (float)mouse_x, py + (float)mouse_y, tw, th));
        printbuf(974, 700);

        sprintf(strbuf, "%.1f", tw);
        printbuf(1210, 680);
        sprintf(strbuf, "%.1f", th);
        printbuf(1210, 700);

        sprintf(strbuf, "%.1f", scale);
        printbuf(1080, 4);
        sprintf(strbuf, "%d", fps);
        printbuf(1200, 4);

        /* Draw the renderer. */
        SDL_RenderPresent(renderer);

        if (SDL_GetTicks() - start_time >= 1000)
        {
            fps = frames;
            frames = -1;
            start_time = SDL_GetTicks();
        }
        frames++;
    }

    /* Clean up and return to the main function. */
    destroy_drawables(drawables, count);
    for (int i = 0; i < SPRITE_LENGTH; i++)
    {
        SDL_DestroyTexture(sprite_textures[i]);
    }

    for (int i = 0; i < TILE_LENGTH; i++)
    {
        SDL_DestroyTexture(tile_textures[i]);
    }

    TTF_CloseFont(debug_font);

    return status;
}

/* These should all be pure functions.
 */
static float pixel_to_tile_x(float px, float py, float tw, float th)
{
    return (px / tw) + (py / th);
}

static float pixel_to_tile_y(float px, float py, float tw, float th)
{
    return (py / th) - (px / tw);
}

static float tile_to_pixel_x(float tx, float ty, float tw)
{
    float w = 0.5f * tw;
    return ((tx - ty) * w); // - w;
}

static float tile_to_pixel_y(float tx, float ty, float th)
{
    return (tx + ty) * 0.5f * th;
}

static void calculate_tile_positions(Tile t[GRID_SIZE][GRID_SIZE], float tw, float th)
{
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            t[x][y].x = tile_to_pixel_x((float)x, (float)y, tw);
            t[x][y].y = tile_to_pixel_y((float)x, (float)y, th);
        }
    }
}
