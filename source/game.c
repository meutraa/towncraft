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

/* The virtual widths of the current visible screen. */
#define screen_width() DESIGN_WIDTH*camera.scale
#define screen_height() DESIGN_HEIGHT*camera.scale

/* This is just to stop repetative stuff. */
#define printbuf(x, y, format, ...) \
    sprintf(strbuf, format, __VA_ARGS__); \
    render_text(renderer, debug_font, strbuf, white, x, y, camera.scale);

#define SHIFT(a, b) if(b > 0){(a) >>= (b);} else if(b < 0){(a) <<= abs(b);}
#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))
#define LENGTH(a) (sizeof(a)/sizeof(*a))

/* The number of keys SDL2 defines. */
#define SCANCODE_COUNT 283

/* The grid of tiles. */
static const int GRID_SIZE = 256;

/* Tile dimensions must be divisible by exp2(DEFAULT_SCALE). */
static const int DEFAULT_SCALE = 3;
static const int TILE_WIDTH = 128 << DEFAULT_SCALE;
static const int TILE_HEIGHT = 64 << DEFAULT_SCALE;

static const int BUILDING_COUNT = LENGTH(building_images);
static const int TERRAIN_COUNT  = LENGTH(tile_images);

/*!
    \brief Scales all the UI elements, and updates the scale value in the camera.

    \var drawables an array of Drawables that need to be scaled.
    \var count the size of the Drawable array.
    \var bits for +ve bits: camera.scale >> bits, for -ve bits: camera.scale << bits
*/
static void change_scale(Camera* camera, SDL_Renderer* renderer, Drawable drawables[], int count, int bits)
{
    SHIFT(((*camera).scale), bits);
    SDL_RenderSetLogicalSize(renderer, (*camera).scale * resolution_width, (*camera).scale * resolution_height);
    for(int i = 0; i < count; i++)
    {
        SHIFT(drawables[i].rect->x, bits);
        SHIFT(drawables[i].rect->y, bits);
        SHIFT(drawables[i].rect->w, bits);
        SHIFT(drawables[i].rect->h, bits);
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

    /* Rectangles for rendering loop. */
    SDL_Rect rect_terrain  = { 0, 0, TILE_WIDTH, TILE_HEIGHT };
    SDL_Rect rect_building = { 0, 0, TILE_WIDTH, 0 };

    int key_status[SCANCODE_COUNT] = { 0 };
    srand((unsigned int)time(NULL));
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

    /* Things required for rendering of text on the UI. */
    char strbuf[32];
    const SDL_Color white = { 255, 255, 255, 0 };
    TTF_Font* debug_font  = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", 16);

    /* Assume 60 for scroll speed to not become infinity. */
    int fps = 60, frames = 0;
    int mouse_x, mouse_y;

    /* Fill our structure arrays. */
    for (int i = 0; i < BUILDING_COUNT; i++)
    {
        SDL_Surface* s = IMG_Load(building_images[i]);
        buildings[i].texture = SDL_CreateTextureFromSurface(renderer, s);
        buildings[i].height  = s->h << DEFAULT_SCALE;
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
            int l = (rand() % (TERRAIN_COUNT + 10) < 9) ? 1 : 0;
            if (l != tiles[MAX(y - 1, 0)][MAX(x - 1, 0)].tile_id)
            {
                l = rand() % TERRAIN_COUNT;
            }
            tiles[y][x].terrain = &terrains[l];
            tiles[y][x].tile_id = l;

            /* If land, maybe place a building. */
            int k = BUILDING_COUNT + 1;
            if (1 == l)
            {
                k = rand() % (BUILDING_COUNT << 2);
                if (0 == k)
                {
                    k = rand() % 4;
                }
            }
            tiles[y][x].building = k < BUILDING_COUNT ? &buildings[k] : NULL;
        }
    }

    /* Load in the game UI. */
    const char* layout = "resources/layouts/game_ui.csv";
    int count = count_drawables(layout);
    Drawable drawables[count];
    load_drawables(renderer, drawables, layout, 0);

    /* Initialise the camera and update everything for the DEFAULT_SCALE. */
    Camera camera = { 1, 0, 0 };
    change_scale(&camera, renderer, drawables, count, -(DEFAULT_SCALE));

    /* Centre the top of the grid on the x axis. */
    camera.x = -(screen_width() >> 1) + (TILE_WIDTH >> 1);

    unsigned int start_time = SDL_GetTicks();
    while (NORMAL == status)
    {
        /* Clear the renderer and get mouse co-ordinates. */
        SDL_RenderClear(renderer);
        SDL_GetMouseState(&mouse_x, &mouse_y);

        /* Process any events in the queue. */
        while (SDL_PollEvent(&event))
        {
            if (SDL_KEYUP == event.type || SDL_KEYDOWN == event.type)
            {
                key_status[event.key.keysym.scancode] = event.type == SDL_KEYUP ? 0 : 1;
            }
            else if (SDL_MOUSEWHEEL == event.type)
            {
                /* Zoom in is 1, zoom out is -1 */
                if ((-1 == event.wheel.y && camera.scale < 64) || (1 == event.wheel.y && camera.scale > 1))
                {
                    change_scale(&camera, renderer, drawables, count, event.wheel.y);
                    int factor = 1 == event.wheel.y ? camera.scale : -(camera.scale >> 1);
                    camera.x += factor * (zoom_mode == 0 ? DESIGN_WIDTH  >> 1 : mouse_x);
                    camera.y += factor * (zoom_mode == 0 ? DESIGN_HEIGHT >> 1 : mouse_y);
                }
            }
        }

        /* Quit the program if Escape is pressed. */
        if (key_status[41])
        {
            status = QUIT_PROGRAM;
            break;
        }

        /* Update the camera co-ordinates if scroll conditions are met. */
        int speed = (int) ceil(camera.scale * scroll_speed * 60.0f / fps);
        if (key_status[80] || (fullscreen && 0 == mouse_x)) // left
        {
            camera.x -= speed;
        }
        if (key_status[79] || (fullscreen && 1279 == mouse_x)) // right
        {
            camera.x += speed;
        }
        if (key_status[82] || (fullscreen && 0 == mouse_y)) // up
        {
            camera.y -= speed >> 1;
        }
        if (key_status[81] || (fullscreen && 719 == mouse_y)) // down
        {
            camera.y += speed >> 1;
        }

        /* Calculate the smallest rectangle sub-grid that will cover our screen. */
        int y1 = pixel_to_tile(camera.x + screen_width(), camera.y).y;
        int y2 = pixel_to_tile(camera.x, camera.y + screen_height()).y + 1;
        int x1 = pixel_to_tile(camera.x, camera.y).x;
        int x2 = pixel_to_tile(camera.x + screen_width(), camera.y + screen_height()).x + 1;

        /* Save two (for-the-next-loop constants). */
        int sw = screen_width();
        int sh = screen_height();

        /* Make sure all these are within bounds of our grid.
           Loop through this sub-grid and render each texture. */
        for (int y = MAX(0, y1); y < MIN(GRID_SIZE, y2); y++)
        {
            for (int x = MAX(0, x1); x < MIN(GRID_SIZE, x2); x++)
            {
                int rtx = tiles[x][y].x - camera.x;
                int rty = tiles[x][y].y - camera.y;

                /* Only render if it will be visible on the screen. */
                if(rtx + TILE_WIDTH  > 0 && rtx < sw && rty + TILE_HEIGHT > 0 && rty < sh)
                {
                    rect_terrain.x = rtx;
                    rect_terrain.y = rty;
                    SDL_RenderCopy(renderer, tiles[x][y].terrain->texture, NULL, &rect_terrain);
                    if (tiles[x][y].building)
                    {
                        rect_building.x = rtx;
                        rect_building.h = tiles[x][y].building->height;
                        rect_building.y = rect_terrain.y - rect_building.h + TILE_HEIGHT;
                        SDL_RenderCopy(renderer, tiles[x][y].building->texture, NULL, &rect_building);
                    }
                }
            }
        }

        /* Copy the game_ui layout drawables to the renderer. */
        render_drawables(renderer, drawables, count);

        /* Get the data we need for the debugging UI. */
        int centre[] = { camera.x + (screen_width() >> 1), camera.y + (screen_height() >> 1) };
        int  mouse[] = { camera.x + camera.scale*mouse_x,  camera.y + camera.scale*mouse_y   };
        Point mouse_tile  = pixel_to_tile(mouse[0], mouse[1]);
        Point corner_tile = pixel_to_tile(camera.x, camera.y);
        Point centre_tile = pixel_to_tile(centre[0], centre[1]);

        /* Print the UI debugging infomation. */
        printbuf(200, 680, "%d, %d", camera.x, camera.y);
        printbuf(200, 700, "%d, %d", corner_tile.x, corner_tile.y);
        printbuf(603, 680, "%d, %d", centre[0], centre[1]);
        printbuf(603, 700, "%d, %d", centre_tile.x, centre_tile.y);
        printbuf(1014, 680, "%d, %d", mouse[0], mouse[1]);
        printbuf(1014, 700, "%d, %d", mouse_tile.x, mouse_tile.y);
        printbuf(1080, 4, "%d", camera.scale);
        printbuf(1200, 4, "%d", fps);

        /* Finish and render the frame. */
        SDL_RenderPresent(renderer);

        /* Calculate the frame rate. */
        if (SDL_GetTicks() - start_time >= 1000)
        {
            fps = frames;
            frames = -1;
            start_time = SDL_GetTicks();
        }
        frames++;
    }

    /* Free any allocated memory. */
    destroy_drawables(drawables, count);
    TTF_CloseFont(debug_font);
    for (int i = 0; i < BUILDING_COUNT; SDL_DestroyTexture(buildings[i++].texture));
    for (int i = 0; i < TERRAIN_COUNT; SDL_DestroyTexture(terrains[i++].texture));

    return status;
}
