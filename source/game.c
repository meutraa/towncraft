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

#define printbuf(x, y) render_text(renderer, debug_font, strbuf, white, x, y, camera.scale);
#define SCANCODE_COUNT 283

/* The grid of tiles. */
static const int GRID_SIZE = 256;
static const int DEFAULT_SCALE = 8;
static const int TILE_WIDTH = 128 * DEFAULT_SCALE;
static const int TILE_HEIGHT = 64 * DEFAULT_SCALE;

/* Function prototypes. */
static void change_scale(Camera* camera, SDL_Renderer* renderer, Drawable drawables[], int count, int bits);
static FPoint pixel_to_tile(int x, int y);
static FPoint tile_to_pixel(float x, float y);

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

    /* Create and fill the positions of the tiles. */
    Tile tiles[GRID_SIZE][GRID_SIZE];
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            FPoint pixel = tile_to_pixel((float)x , (float)y);
            tiles[x][y].x = (int)pixel.x;
            tiles[x][y].y = (int)pixel.y;
        }
    }

    /* Rectangles for rendering loop. */
    SDL_Rect rect_terrain  = { 0, 0, TILE_WIDTH, TILE_HEIGHT };
    SDL_Rect rect_building = { 0, 0, TILE_WIDTH, TILE_HEIGHT << 2 };

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

    /* The camera with scale, and the top left of the grid. */
    Camera camera = { 1, 0, 0 };

    change_scale(&camera, renderer, drawables, count, 3); // * times bigger.

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
                    int zoom = (event.wheel.y > 0) ? -1 : 1;

                    if((1 == zoom && camera.scale != 64) || (-1 == zoom && camera.scale != 1))
                    {
                        change_scale(&camera, renderer, drawables, count, zoom);

                        int offset_x = zoom_mode == 0 ? DESIGN_WIDTH >> 1  : mouse_x;
                        int offset_y = zoom_mode == 0 ? DESIGN_HEIGHT >> 1 : mouse_y;

                        // zooming in
                        if(-1 == zoom)
                        {
                            camera.x += (int)(camera.scale * offset_x);
                            camera.y += (int)(camera.scale * offset_y);
                        }
                        if(1 == zoom)
                        {
                            camera.x -= (int)((camera.scale >> 1) * offset_x);
                            camera.y -= (int)((camera.scale >> 1) * offset_y);
                        }
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

        float speed = camera.scale / DEFAULT_SCALE * scroll_speed * 60.0f / (float)fps;
        int sw = (int) ceil(TILE_WIDTH * speed);
        int sh = (int) ceil(TILE_HEIGHT * speed);
        if (1 == key_status[80] || (0 != fullscreen && 0 == mouse_x)) // left
        {
            camera.x -= sw;
        }
        if (1 == key_status[79] || (0 != fullscreen && 1279 == mouse_x)) // right
        {
            camera.x += sw;
        }
        if (1 == key_status[82] || (0 != fullscreen && 0 == mouse_y)) // up
        {
            camera.y -= sh;
        }
        if (1 == key_status[81] || (0 != fullscreen && 719 == mouse_y)) // down
        {
            camera.y += sh;
        }

        /* Clear the screen for areas that do not have textures mapped to them. */
        /* Comment out for windows 95 mode. */
        SDL_RenderClear(renderer);

        /* Lowest y we need to rend is top right corner. */
        //int y1 = (int)floor(pixel_to_tile(camera.x + DESIGN_WIDTH, camera.y).y);
        //y1 = (y1 < 0) ? 0 : y1;

        /* Highest y is bottom left. */
        //int y2 = (int)ceil(pixel_to_tile(camera.x, camera.y + DESIGN_HEIGHT).y);
        //y2 = (y2 > GRID_SIZE) ? GRID_SIZE : y2;

        /* Lowest x we need is top left corner. */
        //int x1 = (int)floor(pixel_to_tile(camera.x, camera.y).x);
        //x1 = (x1 < 0) ? 0 : x1;

        /* Highest x is at bottom right. */
        //int x2 = (int)ceil(pixel_to_tile(camera.x + DESIGN_WIDTH, camera.y + DESIGN_HEIGHT).x);
        //x2 = (x2 > GRID_SIZE) ? GRID_SIZE : x2;

        for (int y = 0; y < GRID_SIZE; y++)
        {
            for (int x = 0; x < GRID_SIZE; x++)
            {
                rect_terrain.x = tiles[x][y].x - camera.x;
                rect_terrain.y = tiles[x][y].y - camera.y;
                rect_building.x = rect_terrain.x;
                rect_building.y = rect_terrain.y - (3 * TILE_HEIGHT);
                SDL_RenderCopy(renderer, tiles[x][y].tile_texture, NULL, &rect_terrain);
                if (NULL != tiles[x][y].sprite_texture)
                {
                    SDL_RenderCopy(renderer, tiles[x][y].sprite_texture, NULL, &rect_building);
                }
            }
        }

        render_drawables(renderer, drawables, count);

        int centre[] = {
            camera.x + camera.scale*(DESIGN_WIDTH >> 1),
            camera.y + camera.scale*(DESIGN_HEIGHT >> 1)
        };
        int mouse[] = {
            camera.x + camera.scale*mouse_x,
            camera.y + camera.scale*mouse_y
        };
        FPoint mouse_tile  = pixel_to_tile(mouse[0], mouse[1]);
        FPoint corner_tile = pixel_to_tile(camera.x, camera.y);
        FPoint centre_tile = pixel_to_tile(centre[0], centre[1]);

        /* Print the UI debugging infomation. */
        sprintf(strbuf, "%d, %d", camera.x, camera.y);
        printbuf(200, 680);
        sprintf(strbuf, "%.1f, %.1f", corner_tile.x, corner_tile.y);
        printbuf(200, 700);

        sprintf(strbuf, "%d, %d", centre[0], centre[1]);
        printbuf(603, 680);
        sprintf(strbuf, "%.1f, %.1f", centre_tile.x, centre_tile.y);
        printbuf(603, 700);

        sprintf(strbuf, "%d, %d", mouse[0], mouse[1]);
        printbuf(1014, 680);
        sprintf(strbuf, "%.1f, %.1f", mouse_tile.x, mouse_tile.y);
        printbuf(1014, 700);

        sprintf(strbuf, "%d", camera.scale);
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

static void change_scale(Camera* camera, SDL_Renderer* renderer, Drawable drawables[], int count, int bits)
{
    int abits = abs(bits);
    if(bits > 0)
    {
        (*camera).scale <<= abits;
    }
    else if(bits < 0)
    {
        (*camera).scale >>= abits;
    }
    SDL_RenderSetLogicalSize(renderer, (*camera).scale * resolution_width, (*camera).scale * resolution_height);
    for(int i = 0; i < count; i++)
    {
        if(bits > 0)
        {
            drawables[i].rect->x <<= abits;
            drawables[i].rect->y <<= abits;
            drawables[i].rect->w <<= abits;
            drawables[i].rect->h <<= abits;
        }
        else if(bits < 0)
        {
            drawables[i].rect->x >>= abits;
            drawables[i].rect->y >>= abits;
            drawables[i].rect->w >>= abits;
            drawables[i].rect->h >>= abits;
        }
    }
}

/* These should all be pure functions.
 */
static FPoint pixel_to_tile(int x, int y)
{
    FPoint tile = {
        (x / (float)TILE_WIDTH) + (y / (float)TILE_HEIGHT),
        (y / (float)TILE_HEIGHT) - (x / (float)TILE_WIDTH)
    };
    return tile;
}

static FPoint tile_to_pixel(float x, float y)
{
    FPoint pixel = {
        (x - y) * (float)(TILE_WIDTH >> 1),
        (x + y) * (float)(TILE_HEIGHT >> 1)
    };
    return pixel;
}
