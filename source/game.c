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
#include "diamond.h"
#include "macros.h"

static const int maskmap[32] = {
    0, 4, 5, 9, 6, 21, 10, 13, 7, 8, 20, 12, 11, 15, 14, [23] = 16, [27] = 17, [29] = 18, [30] = 19
};
static const int voffsetmap[32] = {
    [2] = 1, [3] = 1, [6] = 1, [7] = 1, [10] = 1, [11] = 1, [14] = 1, [23] = 1, [29] = -1,
};

static const char* building_images[] = {
    "resources/images/building-mega.tga",
    "resources/images/building-1.tga",
    "resources/images/building-2.tga",
    "resources/images/building-2-1.tga",
    "resources/images/building-2-2.tga",
    "resources/images/building-2-3.tga",
    "resources/images/building-2-4.tga",
    NULL,
};

const char* layout = "resources/layouts/game_ui.csv";
const char* terrain_path = "resources/images/terrains.png";

/* Free up some space in the stack. */
char strbuf[32];
TTF_Font* debug_font;
const SDL_Color white = { 255, 255, 255, 0 };
int fps;

SDL_Texture *terrain, **buildings;
Drawable* drawables;

/* This is just to stop repetative stuff. */
#define printbuf(x, y, format, ...) \
    sprintf(strbuf, format, __VA_ARGS__); \
    render_text(renderer, debug_font, strbuf, white, x, y, cam.scale);

static inline void SHIFT (int* a, int b) { if(b > 0) *a >>= b; else if(b < 0) *a <<= abs(b); }
static inline int  LENGTH(void** array)  { int l = 0; while(array[l]) l++; return l; }

/* The grid of tiles. */
static Tile tiles[GRID_SIZE][GRID_SIZE];
static int heightmap[GRID_SIZE][GRID_SIZE];

/* Tile dimensions must be divisible by exp2(DEFAULT_SCALE). */
static const int TILE_WIDTH  = 64;
static const int TILE_HEIGHT = 48;

static SDL_Rect src_rects[3][22];

/* Rectangles for rendering loop. */
static SDL_Rect rect  = { 0, 0, TILE_WIDTH, TILE_HEIGHT };
static SDL_Rect rect_building = { 0, 0, TILE_WIDTH, 0 };

static SDL_Point pixel_to_tile(int x, int y)
{
    return (SDL_Point) {
        (int) floor(((x / (float)TILE_WIDTH) + (3.0f * y / (float)TILE_HEIGHT / 2.0f)) - 0.5f),
        (int) floor(((3.0f * y / (float)TILE_HEIGHT / 2.0f) - (x / (float)TILE_WIDTH)) + 0.5f)
    };
}

static SDL_Point tile_to_pixel(int x, int y)
{
    return (SDL_Point) {
        (x - y) * (TILE_WIDTH >> 1),
        (x + y) * ((int)floor(TILE_HEIGHT / 3.0f))
    };
}

static SDL_Texture** load_textures(SDL_Renderer* renderer, const char* image_paths[])
{
    int l = LENGTH((void**) image_paths);
    SDL_Texture** textures = malloc((unsigned long) (l + 1) * sizeof(SDL_Texture*));

    for(int i = 0; i < l; i++)
    {
        SDL_Surface* s = IMG_Load(image_paths[i]);
        textures[i] = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
    }
    textures[l] = NULL;
    return textures;
}

/* UNPURE FUNCTIONS */

/* Gives bound safe values for corner heights.
   [0] = top, [1] = right, [2] = left, [3] = bottom */
static void get_corner_heights(int heights[4], int x, int y)
{
    heights[0] = heightmap[x][y];
    heights[1] = heightmap[x == GRID_SIZE - 1 ? x : x + 1][y];
    heights[2] = heightmap[x][y == GRID_SIZE - 1 ? y : y + 1];
    heights[3] = heightmap[x == GRID_SIZE - 1 ? x : x + 1][y == GRID_SIZE - 1 ? y : y + 1];
}

static void generate_map(void)
{
    fill_heightmap(heightmap, GRID_SIZE - 1, ROUGHNESS);

    const int building_count = LENGTH((void**) building_images);

    /* Create and fill the positions of the tiles. */
    /* This loop runs upwards of one million times so take out constants! */
    /* FIRST PASS */
    forXY(0, GRID_SIZE)
    {
        SDL_Point pixel = tile_to_pixel(x , y);

        int heights[4];
        get_corner_heights(heights, x, y);
        int u = heights[0], d = heights[3], l = heights[2], r = heights[1];

        /* Get lowest corner. Loop deliberately unrolled.*/
        int low = u;
        if(d < low) low = d;
        if(l < low) low = l;
        if(r < low) low = r;

        int ST = abs(u - d) >= 2 || abs(l - r) >= 2;
        int mask = (l > low) | ((d > low) << 1) | ((r > low) << 2) | ((u > low) << 3) | (ST << 4);

        Tile* tp = &tiles[x][y];
        tp->x = pixel.x;
        tp->y = pixel.y;
        tp->voffset = (voffsetmap[mask] - heights[3]) * (int)(TILE_HEIGHT / 6.0f);
        tp->water = u <= 0 || l <= 0 || r <= 0 || d <= 0;
        tp->terrain_id = u + d + l + r >= 2 ? 0 : 2;
        tp->tile_id  = !mask ? !u : maskmap[mask];
        tp->building = !tp->water && tp->tile_id == 0 && rand() % 6 == 0 ? buildings[rand() % building_count] : NULL;
    }
    /* SECOND PASS */
    forXY(1, GRID_SIZE - 1)
    {
        /* \TODO if tile's corners are higher than tiles height, set as sand_grass. */
        if(tiles[x - 1][y - 1].water + tiles[x - 1][y + 1].water + tiles[x + 1][y - 1].water + + tiles[x + 1][y + 1].water)
        {
            tiles[x][y].terrain_id = 2;
        }
    }
    forXY(0, GRID_SIZE)
    {
        tiles[x][y].src = &src_rects[tiles[x][y].terrain_id][tiles[x][y].tile_id];
    }
}

static void render_grid(SDL_Renderer* renderer, Camera cam)
{
    SDL_RenderClear(renderer);
    int shift = (int) floor(TILE_HEIGHT / 6.0f);

    /* To see the clipping work, comment out all instances of cam.scale in the next 15 lines or so. */

    /* Min y is tr, max x is bl */
    const int tlx = pixel_to_tile(cam.x, cam.y).x;
    const int try = pixel_to_tile(cam.x + DESIGN_WIDTH*cam.scale, cam.y).y;
    const int bly = pixel_to_tile(cam.x, cam.y + DESIGN_HEIGHT*cam.scale).y;
    const int brx = pixel_to_tile(cam.x + DESIGN_WIDTH*cam.scale, cam.y + DESIGN_HEIGHT*cam.scale).x;

    for(int y = try < 0 ? 0 : try; y < ((bly > GRID_SIZE) ? GRID_SIZE : bly); y++)
    for(int x = tlx < 0 ? 0 : tlx; x < ((brx > GRID_SIZE) ? GRID_SIZE : brx); x++)
    {
        Tile* tp = &tiles[x][y];
        rect.x = tp->x - cam.x;
        rect.y = tp->y - cam.y + tp->voffset;

        /* Only render if it will be visible on the screen. */
        if(rect.x + TILE_WIDTH > 0 && rect.x < DESIGN_WIDTH*cam.scale && rect.y + TILE_HEIGHT > 0 && rect.y < DESIGN_HEIGHT*cam.scale)
        {
            SDL_RenderCopyEx(renderer, terrain, tp->src, &rect, 0.0, NULL, SDL_FLIP_NONE);
            if(tp->water)
            {
                rect.y = tp->y - cam.y - shift;
                SDL_RenderCopyEx(renderer, terrain, &src_rects[0][2], &rect, 0.0, NULL, SDL_FLIP_NONE);
            }
            if (tp->building)
            {
                /*rect_building.x = rtx;
                rect_building.h = tp->building->height;
                rect_building.y = rect.y - rect_building.h + (int) floor(TILE_HEIGHT);
                SDL_RenderCopy(renderer, tp->building->texture, NULL, &rect_building);*/
            }
        }
    }
    /* Copy the game_ui layout drawables to the renderer. */
    render_drawables(renderer, drawables, cam.scale);

    /* Print the UI infomation. */
    printbuf(1080, 4, "%d", cam.scale);
    printbuf(1200, 4, "%d", fps);

    /* Finish and render the frame. */
    SDL_RenderPresent(renderer);
}

Status game_loop(SDL_Renderer* renderer)
{
    Status status = NORMAL;
    int key_status[283] = { 0 };
    SDL_Event event;

    srand((unsigned int)time(NULL));
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

    /* Initialise globals. */
    /* Fill src_rects. */
    for(int i = 0; i < 3; i++) for(int j = 0; j < 22; j++)
    {
        src_rects[i][j] = (SDL_Rect) {j*TILE_WIDTH, i*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT};
    }
    fps = 60;
    debug_font = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", 16);
    drawables  = load_drawables(renderer, layout);
    buildings  = load_textures(renderer, building_images);
    SDL_Surface* s = IMG_Load(terrain_path);
    terrain = SDL_CreateTextureFromSurface(renderer, s);
    SDL_FreeSurface(s);
    generate_map();

    /* Assume 60 for scroll speed to not become infinity. */
    int frames = 0;
    int mouse_x, mouse_y;

    /* Initialise the cam and set to the center of the grid. */
    Camera cam = {
        1, 0, 0
        //(TILE_WIDTH / 2) - (DESIGN_WIDTH / 2),
        //tile_to_pixel(((GRID_SIZE - 1) / 2) - 1, ((GRID_SIZE - 1) / 2) - 1).y - DESIGN_HEIGHT / 2
    };

    unsigned int start_time = SDL_GetTicks();
    int render = 1;
    while (NORMAL == status)
    {
        /* Clear the renderer and get mouse co-ordinates. */
        SDL_GetMouseState(&mouse_x, &mouse_y);
        unsigned int start_frame = SDL_GetTicks();

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
                if (-1 == event.wheel.y || (1 == event.wheel.y && cam.scale > 1))
                {
                    SHIFT(&cam.scale, event.wheel.y);
                    SDL_RenderSetLogicalSize(renderer, cam.scale * resolution_width, cam.scale * resolution_height);
                    int factor = 1 == event.wheel.y ? cam.scale : -(cam.scale >> 1);
                    cam.x += factor * (zoom_mode == 0 ? DESIGN_WIDTH  >> 1 : mouse_x);
                    cam.y += factor * (zoom_mode == 0 ? DESIGN_HEIGHT >> 1 : mouse_y);
                    render = 1;
                }
            }
        }

        /* Quit the program if Escape is pressed. */
        if(key_status[41]) status = SWITCHTO_MAINMENU;

#define SCROLL(a, b, c, d, e) if(key_status[a] || (fullscreen && b == c)) { d += (int)(scroll_speed*cam.scale * e); render = 1; }
        SCROLL(80, 0,    mouse_x, cam.x, -1)
        SCROLL(79, 1279, mouse_x, cam.x,  1)
        SCROLL(82, 0,    mouse_y, cam.y, -0.5)
        SCROLL(81, 719,  mouse_y, cam.y,  0.5)

        /* Calculate the frame rate. */
        unsigned int dt = SDL_GetTicks() - start_frame;
        if(dt < 16) SDL_Delay(16 - dt);
        if(SDL_GetTicks() - start_time >= 1000)
        {
            fps = frames;
            frames = -1;
            start_time = SDL_GetTicks();
            render = 1;

            SDL_Point tile = pixel_to_tile(cam.x + mouse_x*cam.scale, cam.y + mouse_y*cam.scale);
            printf("Mouse tile: %d, %d\n", tile.x, tile.y);
        }
        frames++;
        if(render){ render_grid(renderer, cam); render = 0; }
    }

    /* Free any allocated memory. */
    destroy_drawables(drawables);
    TTF_CloseFont(debug_font);
    destroy_textures(buildings);
    SDL_DestroyTexture(terrain);
    SDL_RenderSetLogicalSize(renderer, DESIGN_WIDTH, DESIGN_HEIGHT);
    return status;
}
