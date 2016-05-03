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

#define GRASS_PATH "resources/images/grass/"
static const char* grass_images[] = {
    GRASS_PATH "0001.png", GRASS_PATH "0002.png", GRASS_PATH "0003.png",
    GRASS_PATH "0004.png", GRASS_PATH "0005.png", GRASS_PATH "0006.png",
    GRASS_PATH "0007.png", GRASS_PATH "0008.png", GRASS_PATH "0009.png",
    GRASS_PATH "0010.png", GRASS_PATH "0011.png", GRASS_PATH "0012.png",
    GRASS_PATH "0013.png", GRASS_PATH "0014.png", GRASS_PATH "0015.png",
    GRASS_PATH "0016.png", GRASS_PATH "0017.png", GRASS_PATH "0018.png",
    GRASS_PATH "0019.png", GRASS_PATH "0020.png", GRASS_PATH "0021.png",
    GRASS_PATH "0022.png", NULL,
};

#define SAND_PATH "resources/images/sand/"
static const char* sand_images[] = {
    SAND_PATH "0001.png", SAND_PATH "0002.png", SAND_PATH "0003.png",
    SAND_PATH "0004.png", SAND_PATH "0005.png", SAND_PATH "0006.png",
    SAND_PATH "0007.png", SAND_PATH "0008.png", SAND_PATH "0009.png",
    SAND_PATH "0010.png", SAND_PATH "0011.png", SAND_PATH "0012.png",
    SAND_PATH "0013.png", SAND_PATH "0014.png", SAND_PATH "0015.png",
    SAND_PATH "0016.png", SAND_PATH "0017.png", SAND_PATH "0018.png",
    SAND_PATH "0019.png", SAND_PATH "0020.png", SAND_PATH "0021.png",
    SAND_PATH "0022.png", NULL,
};

#define SG_PATH "resources/images/sand-grass/"
static const char* sg_images[] = {
    SG_PATH "0001.png", SG_PATH "0002.png", SG_PATH "0003.png",
    SG_PATH "0004.png", SG_PATH "0005.png", SG_PATH "0006.png",
    SG_PATH "0007.png", SG_PATH "0008.png", SG_PATH "0009.png",
    SG_PATH "0010.png", SG_PATH "0011.png", SG_PATH "0012.png",
    SG_PATH "0013.png", SG_PATH "0014.png", SG_PATH "0015.png",
    SG_PATH "0016.png", SG_PATH "0017.png", SG_PATH "0018.png",
    SG_PATH "0019.png", SG_PATH "0020.png", SG_PATH "0021.png",
    SG_PATH "0022.png", NULL,
};

/* Free up some space in the stack. */
char strbuf[32];
TTF_Font* debug_font;
const SDL_Color white = { 255, 255, 255, 0 };
int fps = 60;

SDL_Texture **buildings, **grass, **sand, **sand_grass;
Drawable* drawables;

/* This is just to stop repetative stuff. */
#define printbuf(x, y, format, ...) \
    sprintf(strbuf, format, __VA_ARGS__); \
    render_text(renderer, debug_font, strbuf, white, x, y, camera.scale);

static inline void SHIFT (int* a, int b) { if(b > 0) *a >>= b; else if(b < 0) *a <<= abs(b); }
static inline int  LENGTH(void** array)  { int l = 0; while(NULL != array[l]) l++; return l; }

/* The number of keys SDL2 defines. */
#define KEYCOUNT 283

/* The grid of tiles. */
static Tile tiles[GRID_SIZE][GRID_SIZE];
static float floatmap[GRID_SIZE][GRID_SIZE];
static int heightmap[GRID_SIZE][GRID_SIZE];

/* Tile dimensions must be divisible by exp2(DEFAULT_SCALE). */
static const int DEFAULT_SCALE = 3;
static const int TILE_WIDTH    = 128 << DEFAULT_SCALE;
static const int TILE_HEIGHT   = 96  << DEFAULT_SCALE;

/* Rectangles for rendering loop. */
static SDL_Rect rect  = { 0, 0, TILE_WIDTH, TILE_HEIGHT };
static SDL_Rect rect_building = { 0, 0, TILE_WIDTH, 0 };

static void change_scale(Camera* camera, SDL_Renderer* renderer, Drawable* d, int bits)
{
    SHIFT(&((*camera).scale), bits);
    SDL_RenderSetLogicalSize(renderer, (*camera).scale * resolution_width, (*camera).scale * resolution_height);
    for(int i = 0; NULL != (*(d + i)).texture; i++)
    {
        SHIFT(&((*(d + i)).rect->x), bits);
        SHIFT(&((*(d + i)).rect->y), bits);
        SHIFT(&((*(d + i)).rect->w), bits);
        SHIFT(&((*(d + i)).rect->h), bits);
    }
}

static SDL_Point pixel_to_tile(int x, int y)
{
    return (SDL_Point) {
        (int) floor(((x / (float)TILE_WIDTH) + (y / (float)TILE_HEIGHT)) - 0.5f),
        (int) floor(((y / (float)TILE_HEIGHT) - (x / (float)TILE_WIDTH)) + 0.5f)
    };
}

static SDL_Point tile_to_pixel(int x, int y)
{
    return (SDL_Point) {
        (x - y) * (TILE_WIDTH >> 1),
        (x + y) * ((int)floor((TILE_HEIGHT * 2.0f / 3.0f)) >> 1)
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
    heights[0] = top    corner height
    heights[1] = right  corner height
    heights[2] = left   corner height
    heights[3] = bottom corner height
*/
static void get_corner_heights(int heights[4], int x, int y)
{
    heights[0] = heightmap[x][y];
    heights[1] = heightmap[x == GRID_SIZE - 1 ? x : x + 1][y];
    heights[2] = heightmap[x][y == GRID_SIZE - 1 ? y : y + 1];
    heights[3] = heightmap[x == GRID_SIZE - 1 ? x : x + 1][y == GRID_SIZE - 1 ? y : y + 1];
}

static void generate_map(void)
{
    /* Set corner heights. */
    floatmap[0][GRID_SIZE - 1]             = (rand() % HEIGHT) - LOWER_HEIGHT;
    floatmap[GRID_SIZE - 1][0]             = (rand() % HEIGHT) - LOWER_HEIGHT;
    floatmap[0][0]                         = (rand() % HEIGHT) - LOWER_HEIGHT;
    floatmap[GRID_SIZE - 1][GRID_SIZE - 1] = (rand() % HEIGHT) - LOWER_HEIGHT;
    fill_heightmap(floatmap, GRID_SIZE - 1, ROUGHNESS);
    for(int y = 0; y < GRID_SIZE; y++)
        for(int x = 0; x < GRID_SIZE; x++)
            heightmap[x][y] = (int) floor(floatmap[x][y]);

    /* Create and fill the positions of the tiles. */
    /* FIRST PASS */
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            SDL_Point pixel = tile_to_pixel(x , y);
            Tile* tp = &tiles[x][y];

            int heights[4];
            get_corner_heights(heights, x, y);
            int u = heights[0], d = heights[3], l = heights[2], r = heights[1];

            int t = 0;
            tp->voffset = 0;

            /* get lowest corner. */
            int low = 10000;
            for(int i = 0; i < 4; i++) if(heights[i] < low) low = heights[i];

            /* Is slope steep? */
            int ST = abs(u - d) >= 2 || abs(l - r) >= 2;

            int mask = (l > low) | ((d > low) << 1) | ((r > low) << 2) | ((u > low) << 3) | (ST << 4);
            if(0 == mask)        t = u == 0;
            else if(1 == mask)   t = 4;
            else if(2 == mask) { t = 5;  tp->voffset = 1; }
            else if(3 == mask) { t = 9;  tp->voffset = 1; }
            else if(4 == mask)   t = 6;
            else if(5 == mask)   t = 21;
            else if(6 == mask) { t = 10; tp->voffset = 1; }
            else if(7 == mask) { t = 13; tp->voffset = 1; }
            else if(8 == mask)   t = 7;
            else if(9 == mask)   t = 8;
            else if(10 == mask){ t = 20; tp->voffset = 1; }
            else if(11 == mask){ t = 12; tp->voffset = 1; }
            else if(12 == mask)  t = 11;
            else if(13 == mask)  t = 15;
            else if(14 == mask){ t = 14; tp->voffset = 1; }
            else if(23 == mask){ t = 16; tp->voffset = 1; }
            else if(27 == mask)  t = 17;
            else if(29 == mask){ t = 18; tp->voffset = -1; }
            else if(30 == mask)  t = 19;

            int b = rand() % LENGTH((void**) building_images);

            tp->x = pixel.x;
            tp->y = pixel.y;
            tp->water = u <= 0 || l <= 0 || r <= 0 || d <= 0;
            tp->terrain = u + d + l + r >= 2 ? grass[t] : sand[t];
            tp->tile_id  = t;
            tp->building = !tp->water && tp->tile_id == 0 && rand() % 6 == 0 ? buildings[b] : NULL;
        }
    }
    /* SECOND PASS */
    for (int y = 1; y < GRID_SIZE - 1; y++)
    {
        for (int x = 1; x < GRID_SIZE - 1; x++)
        {
            /* \TODO if tile's corners is heigher than tiles height, set as sand_grass. */
            if(tiles[x - 1][y - 1].water + tiles[x - 1][y + 1].water + tiles[x + 1][y - 1].water + + tiles[x + 1][y + 1].water)
            {
                tiles[x][y].terrain = sand[tiles[x][y].tile_id];
            }
        }
    }
}

static void render_grid(SDL_Renderer* renderer, Camera camera)
{
    SDL_RenderClear(renderer);
    const int sw = DESIGN_WIDTH*camera.scale;
    const int sh = DESIGN_HEIGHT*camera.scale;
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            Tile* tp = &tiles[x][y];
            int shift = (int) floor(TILE_HEIGHT / 6.0f);
            rect.x = tp->x - camera.x;
            rect.y = tp->y - camera.y;
            int rtw = rect.y - shift;

            int heights[4];
            get_corner_heights(heights, x, y);

            /* The shift for tile height. */
            rect.y -= heights[3] * shift;

            /* The shift for tile types. */
            rect.y += tp->voffset * shift;

            /* Only render if it will be visible on the screen. */
            if(rect.x + TILE_WIDTH > 0 && rect.x < sw && rect.y + TILE_HEIGHT > 0 && rect.y < sh)
            {
                SDL_RenderCopy(renderer, tp->terrain, NULL, &rect);
                if(tp->water)
                {
                    rect.y = rtw;
                    SDL_RenderCopy(renderer, grass[2], NULL, &rect);
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
    }
    /* Copy the game_ui layout drawables to the renderer. */
    render_drawables(renderer, drawables);

    /* Get the data we need for the debugging UI. */
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    int centre[2] = {camera.x + (DESIGN_WIDTH*camera.scale >> 1), camera.y + (DESIGN_HEIGHT*camera.scale >> 1) };
    int mouse[2]  = {camera.x + camera.scale*mouse_x, camera.y + camera.scale*mouse_y };
    SDL_Point mouse_tile = pixel_to_tile(mouse[0],  mouse[1]);
    SDL_Point cnr_tile   = pixel_to_tile(camera.x,  camera.y);
    SDL_Point ctr_tile   = pixel_to_tile(centre[0], centre[1]);

    /* Print the UI debugging infomation. */
    printbuf(200, 680, "%d, %d",  camera.x,     camera.y);
    printbuf(200, 700, "%d, %d",  cnr_tile.x,   cnr_tile.y);
    printbuf(603, 680, "%d, %d",  centre[0],    centre[1]);
    printbuf(603, 700, "%d, %d",  ctr_tile.x,   ctr_tile.y);
    printbuf(1014, 680, "%d, %d", mouse[0],     mouse[1]);
    printbuf(1014, 700, "%d, %d", mouse_tile.x, mouse_tile.y);
    printbuf(1080, 4, "%d", camera.scale);
    printbuf(1200, 4, "%d", fps);

    /* Finish and render the frame. */
    SDL_RenderPresent(renderer);
}

Status game_loop(SDL_Renderer* renderer)
{
    Status status = NORMAL;
    SDL_Event event;

    int key_status[KEYCOUNT] = { 0 };
    srand((unsigned int)time(NULL));
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

    /* Initialise globals. */
    debug_font = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", 16);
    drawables  = load_drawables(renderer, layout);
    buildings  = load_textures(renderer, building_images);
    grass      = load_textures(renderer, grass_images);
    sand       = load_textures(renderer, sand_images);
    sand_grass = load_textures(renderer, sg_images);
    generate_map();

    /* Assume 60 for scroll speed to not become infinity. */
    int frames = 0;
    int mouse_x, mouse_y;

    /* Initialise the camera and update everything for the DEFAULT_SCALE. */
    Camera camera = { 1, 0, 0 };
    change_scale(&camera, renderer, drawables, -(DEFAULT_SCALE));
    camera.x = (TILE_WIDTH >> 1) - (DESIGN_WIDTH << (DEFAULT_SCALE - 1));
    camera.y = tile_to_pixel(((GRID_SIZE - 1) >> 1) - 1, ((GRID_SIZE - 1) >> 1) - 1).y + (TILE_HEIGHT >> 1);

    unsigned int start_time = SDL_GetTicks();
    render_grid(renderer, camera);
    while (NORMAL == status)
    {
        /* Clear the renderer and get mouse co-ordinates. */
        SDL_GetMouseState(&mouse_x, &mouse_y);
        unsigned int start_frame = SDL_GetTicks();
        int render = 0;

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
                if (-1 == event.wheel.y || (1 == event.wheel.y && camera.scale > 1))
                {
                    change_scale(&camera, renderer, drawables, event.wheel.y);
                    int factor = 1 == event.wheel.y ? camera.scale : -(camera.scale >> 1);
                    camera.x += factor * (zoom_mode == 0 ? DESIGN_WIDTH  >> 1 : mouse_x);
                    camera.y += factor * (zoom_mode == 0 ? DESIGN_HEIGHT >> 1 : mouse_y);
                    render = 1;
                }
            }
        }

        /* Quit the program if Escape is pressed. */
        if (key_status[41])
        {
            status = SWITCHTO_MAINMENU;
            break;
        }

        /* Update the camera co-ordinates if scroll conditions are met. */
        int speed = (int) (camera.scale * scroll_speed);
        if (key_status[80] || (fullscreen && 0 == mouse_x)) // left
        {
            camera.x -= speed; render = 1;
        }
        if (key_status[79] || (fullscreen && 1279 == mouse_x)) // right
        {
            camera.x += speed; render = 1;
        }
        if (key_status[82] || (fullscreen && 0 == mouse_y)) // up
        {
            camera.y -= speed >> 1; render = 1;
        }
        if (key_status[81] || (fullscreen && 719 == mouse_y)) // down
        {
            camera.y += speed >> 1; render = 1;
        }

        /* Calculate the frame rate. */
        unsigned int dt = SDL_GetTicks() - start_frame;
        if(dt < 16) SDL_Delay(16 - dt);
        if (SDL_GetTicks() - start_time >= 1000)
        {
            fps = frames;
            frames = -1;
            start_time = SDL_GetTicks();
            render = 1;
        }
        frames++;
        if(render) render_grid(renderer, camera);
    }

    /* Free any allocated memory. */
    destroy_drawables(drawables);
    TTF_CloseFont(debug_font);
    for (int i = 0; buildings[i]  != NULL; SDL_DestroyTexture(buildings[i++]));
    for (int i = 0; grass[i]      != NULL; SDL_DestroyTexture(grass[i++]));
    for (int i = 0; sand[i]       != NULL; SDL_DestroyTexture(sand[i++]));
    for (int i = 0; sand_grass[i] != NULL; SDL_DestroyTexture(sand_grass[i++]));
    free(buildings);
    free(grass);
    free(sand);
    free(sand_grass);

    SDL_RenderSetLogicalSize(renderer, DESIGN_WIDTH, DESIGN_HEIGHT);
    return status;
}
