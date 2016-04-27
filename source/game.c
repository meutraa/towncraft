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

static const char* building_images[] = {
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

static const int BUILDING_COUNT = sizeof(building_images)/sizeof(*building_images);
static const int TERRAIN_COUNT  = sizeof(tile_images)/sizeof(*tile_images);

static const SDL_Color white = { 255, 255, 255, 0 };

#define screen_width() (int)(DESIGN_WIDTH*camera.scale)
#define screen_height() (int)(DESIGN_HEIGHT*camera.scale)
#define printbuf(x, y) render_text(renderer, debug_font, strbuf, white, x, y, camera.scale);
#define SCANCODE_COUNT 283

/* The grid of tiles. */
static const int GRID_SIZE = 256;
static const int DEFAULT_SCALE = 8;
static const int TILE_WIDTH = 128 * DEFAULT_SCALE;
static const int TILE_HEIGHT = 64 * DEFAULT_SCALE;

/* Function prototypes. */
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

static Point pixel_to_tile(int x, int y)
{
    return (Point) {
        (int) floor(((x / (float)TILE_WIDTH) + (y / (float)TILE_HEIGHT)) - 0.5f),
        (int) floor(((y / (float)TILE_HEIGHT) - (x / (float)TILE_WIDTH)) + 0.5f)
    };
}

static Point tile_to_pixel(int x, int y)
{
    return (Point) {
        (x - y) * (TILE_WIDTH >> 1),
        (x + y) * (TILE_HEIGHT >> 1)
    };
}

Status game_loop(SDL_Renderer* renderer)
{
    Status status = NORMAL;
    SDL_Event event;

    Building buildings[BUILDING_COUNT];
    Terrain terrains[TERRAIN_COUNT];

    char strbuf[32];
    int key_status[SCANCODE_COUNT] = { 0 };
    TTF_Font* debug_font = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", 16);

    int mouse_x, mouse_y;

    /* Assume 60 for scroll speed to not become infinity. */
    int fps = 60, frames = 0;
    unsigned int start_time;

    /* Fill our structure arrays. */
    for (int i = 0; i < BUILDING_COUNT; i++)
    {
        SDL_Surface* s = IMG_Load(building_images[i]);
        buildings[i].texture = SDL_CreateTextureFromSurface(renderer, s);
        buildings[i].height  = s->h * DEFAULT_SCALE;
        SDL_FreeSurface(s);
    }

    for (int i = 0; i < TERRAIN_COUNT; i++)
    {
        SDL_Surface* s = IMG_Load(tile_images[i]);
        terrains[i].texture = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
    }

    /* Create and fill the positions of the tiles. */
    Tile tiles[GRID_SIZE][GRID_SIZE];
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            Point pixel = tile_to_pixel(x , y);
            tiles[x][y].x = pixel.x;
            tiles[x][y].y = pixel.y;

            /* If connected to land, and roll was water, reroll. */
            int l = rand() % (TERRAIN_COUNT + 10);
            l = (l < 9) ? 1 : 0;
            if (y > 0 && x > 0)
            {
                if (l != tiles[y - 1][x - 1].tile_id)
                {
                    l = rand() % TERRAIN_COUNT;
                }
            }
            tiles[y][x].terrain = &terrains[l];
            tiles[y][x].tile_id = l;

            /* If land, maybe place a building. */
            if (1 == l)
            {
                int k = rand() % (BUILDING_COUNT << 2);
                if (0 == k)
                {
                    k = rand() % 4;
                }
                tiles[y][x].building = k < BUILDING_COUNT ? &buildings[k] : NULL;
            }
        }
    }

    /* Rectangles for rendering loop. */
    SDL_Rect rect_terrain  = { 0, 0, TILE_WIDTH, TILE_HEIGHT };
    SDL_Rect rect_building = { 0, 0, TILE_WIDTH, 0 };

    srand((unsigned int)time(NULL));

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

    /* Load in the UI. */
    const char* layout = "resources/layouts/game_ui.csv";
    int count = count_drawables(layout);
    Drawable drawables[count];
    load_drawables(renderer, drawables, layout, 0);

    Camera camera = { 1, 0, 0 };

    change_scale(&camera, renderer, drawables, count, 3); // * times bigger.

    /* Centre the top of the grid on the x axis. */
    camera.x = -(screen_width() >> 1) + (TILE_WIDTH >> 1);

    while (NORMAL == status)
    {
        SDL_GetMouseState(&mouse_x, &mouse_y);

        /* If there are events in the event queue, process them. */
        while (1 == SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_KEYUP:
                case SDL_KEYDOWN:
                {
                    key_status[event.key.keysym.scancode] = event.type == SDL_KEYUP ? 0 : 1;
                    break;
                }
                case SDL_MOUSEWHEEL:
                {
                    int zoom = (event.wheel.y > 0) ? -1 : 1;

                    if((1 == zoom && camera.scale != 64) || (-1 == zoom && camera.scale != 1))
                    {
                        change_scale(&camera, renderer, drawables, count, zoom);

                        int offset_x = zoom_mode == 0 ? DESIGN_WIDTH  >> 1 : mouse_x;
                        int offset_y = zoom_mode == 0 ? DESIGN_HEIGHT >> 1 : mouse_y;

                        int factor = -1 == zoom ? camera.scale : -(camera.scale >> 1);

                        camera.x += factor * offset_x;
                        camera.y += factor * offset_y;
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

        float speed = camera.scale / DEFAULT_SCALE * scroll_speed * 60.0f / fps;
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
        int y1 = pixel_to_tile(camera.x + screen_width(), camera.y).y;
        y1 = (y1 < 0) ? 0 : y1;

        /* Highest y is bottom left. */
        int y2 = pixel_to_tile(camera.x, camera.y + screen_height()).y + 1;
        y2 = (y2 > GRID_SIZE) ? GRID_SIZE : y2;

        /* Lowest x we need is top left corner. */
        int x1 = pixel_to_tile(camera.x, camera.y).x;
        x1 = (x1 < 0) ? 0 : x1;

        /* Highest x is at bottom right. */
        int x2 = pixel_to_tile(camera.x + screen_width(), camera.y + screen_height()).x + 1;
        x2 = (x2 > GRID_SIZE) ? GRID_SIZE : x2;

        for (int y = y1; y < y2; y++)
        {
            for (int x = x1; x < x2; x++)
            {
                rect_terrain.x = tiles[x][y].x - camera.x;
                rect_terrain.y = tiles[x][y].y - camera.y;
                SDL_RenderCopy(renderer, tiles[x][y].terrain->texture, NULL, &rect_terrain);
                if (NULL != tiles[x][y].building)
                {
                    rect_building.x = rect_terrain.x;
                    rect_building.h = tiles[x][y].building->height;
                    rect_building.y = rect_terrain.y - rect_building.h + TILE_HEIGHT;
                    SDL_RenderCopy(renderer, tiles[x][y].building->texture, NULL, &rect_building);
                }
            }
        }

        render_drawables(renderer, drawables, count);

        int centre[] = {
            camera.x + (screen_width() >> 1),
            camera.y + (screen_height() >> 1)
        };
        int mouse[] = {
            camera.x + camera.scale*mouse_x,
            camera.y + camera.scale*mouse_y
        };
        Point mouse_tile  = pixel_to_tile(mouse[0], mouse[1]);
        Point corner_tile = pixel_to_tile(camera.x, camera.y);
        Point centre_tile = pixel_to_tile(centre[0], centre[1]);

        /* Print the UI debugging infomation. */
        sprintf(strbuf, "%d, %d", camera.x, camera.y);
        printbuf(200, 680);
        sprintf(strbuf, "%d, %d", corner_tile.x, corner_tile.y);
        printbuf(200, 700);

        sprintf(strbuf, "%d, %d", centre[0], centre[1]);
        printbuf(603, 680);
        sprintf(strbuf, "%d, %d", centre_tile.x, centre_tile.y);
        printbuf(603, 700);

        sprintf(strbuf, "%d, %d", mouse[0], mouse[1]);
        printbuf(1014, 680);
        sprintf(strbuf, "%d, %d", mouse_tile.x, mouse_tile.y);
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
    for (int i = 0; i < BUILDING_COUNT; i++)
    {
        SDL_DestroyTexture(buildings[i].texture);
    }

    for (int i = 0; i < TERRAIN_COUNT; i++)
    {
        SDL_DestroyTexture(terrains[i].texture);
    }

    TTF_CloseFont(debug_font);

    return status;
}
