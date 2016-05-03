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

#define building_count 7
#define  terrain_count 22

static const char* building_images[building_count + 1] = {
    "resources/images/building-mega.tga",
    "resources/images/building-1.tga",
    "resources/images/building-2.tga",
    "resources/images/building-2-1.tga",
    "resources/images/building-2-2.tga",
    "resources/images/building-2-3.tga",
    "resources/images/building-2-4.tga",
    NULL,
};


#define GRASS_PATH "resources/images/grass/"
static const char* grass_images[terrain_count + 1] = {
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
static const char* sand_images[terrain_count + 1] = {
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
static const char* sg_images[terrain_count + 1] = {
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
const SDL_Color white = { 255, 255, 255, 0 };
Building buildings[building_count];
Terrain grass[terrain_count];
Terrain sand[terrain_count];
Terrain sand_grass[terrain_count];

/* This is just to stop repetative stuff. */
#define printbuf(x, y, format, ...) \
    sprintf(strbuf, format, __VA_ARGS__); \
    render_text(renderer, debug_font, strbuf, white, x, y, camera.scale);

static inline void SHIFT(int* a, int b) { if(b > 0) *a >>= b; else if(b < 0) *a <<= abs(b); }
static inline int  MIN(int a, int b)    { return (a < b) ? a : b;      }
static inline int  MAX(int a, int b)    { return (a > b) ? a : b;      }

/* The number of keys SDL2 defines. */
#define KEYCOUNT 283

/* The grid of tiles. */
Tile tiles[GRID_SIZE][GRID_SIZE];
float floatmap[GRID_SIZE][GRID_SIZE];
int heightmap[GRID_SIZE][GRID_SIZE];

/* Tile dimensions must be divisible by exp2(DEFAULT_SCALE). */
static const int DEFAULT_SCALE = 3;
static const int TILE_WIDTH    = 128 << DEFAULT_SCALE;
static const int TILE_HEIGHT   = 96  << DEFAULT_SCALE;

/*!
    \brief Scales all the UI elements, and updates the scale value in the camera.

    \var drawables an array of Drawables that need to be scaled.
    \var count the size of the Drawable array.
    \var bits for +ve bits: camera.scale >> bits, for -ve bits: camera.scale << bits
*/
static void change_scale(Camera* camera, SDL_Renderer* renderer, Drawable* drawables, int bits)
{
    SHIFT(&((*camera).scale), bits);
    SDL_RenderSetLogicalSize(renderer, (*camera).scale * resolution_width, (*camera).scale * resolution_height);
    for(int i = 0; NULL != (*(drawables + i)).texture; i++)
    {
        SHIFT(&((*(drawables + i)).rect->x), bits);
        SHIFT(&((*(drawables + i)).rect->y), bits);
        SHIFT(&((*(drawables + i)).rect->w), bits);
        SHIFT(&((*(drawables + i)).rect->h), bits);
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

Status game_loop(SDL_Renderer* renderer)
{
    Status status = NORMAL;
    SDL_Event event;

    /* Rectangles for rendering loop. */
    SDL_Rect rect_terrain  = { 0, 0, TILE_WIDTH, TILE_HEIGHT };
    SDL_Rect rect_building = { 0, 0, TILE_WIDTH, 0 };

    int key_status[KEYCOUNT] = { 0 };
    srand((unsigned int)time(NULL));
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

    /* Things required for rendering of text on the UI. */
    char strbuf[32];
    TTF_Font* debug_font  = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", 16);

    /* Assume 60 for scroll speed to not become infinity. */
    int fps = 60, frames = 0;
    int mouse_x, mouse_y;
    Tile *tp;

    /* Fill our structure arrays. */
    for (int i = 0; i < building_count; i++)
    {
        SDL_Surface* s = IMG_Load(building_images[i]);
        buildings[i].texture = SDL_CreateTextureFromSurface(renderer, s);
        buildings[i].height  = s->h << DEFAULT_SCALE;
        SDL_FreeSurface(s);
    }

    for (int i = 0; i < terrain_count; i++)
    {
        SDL_Surface* s = IMG_Load(grass_images[i]);
        grass[i].texture = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);

        s = IMG_Load(sand_images[i]);
        sand[i].texture = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);

        s = IMG_Load(sg_images[i]);
        sand_grass[i].texture = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
    }

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
            tp = &tiles[x][y];
            int hxy = heightmap[x][y];
            int hx1y = heightmap[x + 1][y];
            int hxy1 = heightmap[x][y + 1];
            int hx1y1 = heightmap[x + 1][y + 1];

            if(x == GRID_SIZE - 1 && y != GRID_SIZE - 1)
            {
                hx1y = hxy;
                hx1y1 = hxy1;
            }
            else if(x != GRID_SIZE - 1 && y == GRID_SIZE - 1)
            {
                hxy1 = hxy;
                hx1y1 = hx1y;
            }

            int u = 0, d = 0, l = 0, r = 0, h = hxy;
            int ST = 0, t = 0;

            if(hxy1 > h) l = 1;
            else if(hxy1 < h) l = -1;

            if(hxy1 < hx1y - 1)     { t = 19; ST = 1; }
            else if(hx1y < hxy1 - 1){ t = 17; ST = 1; }
            else if(hx1y1 < h - 1)  { t = 18; ST = 1; tp->voffset = -1; }
            else if(hxy < hx1y1 - 1){ t = 16; ST = 1; tp->voffset = 1;  }

            if(hx1y > h) r = 1;
            else if(hx1y < h) r = -1;

            if(hx1y1 > h) d = 1;
            else if(hx1y1 < h) d = -1;

            tp->voffset = 0;

            // get the magnitude of the lowest corner.
            int low = (u < 0 || d < 0 || l < 0 || r < 0) ? -1 : 0;
            int CW = l > low ?  1 : 0;
            int CS = d > low ?  2 : 0;
            int CE = r > low ?  4 : 0;
            int CN = u > low ?  8 : 0;
            ST     = ST      ? 16 : 0;
            int mask = CW | CS | CE | CN | ST;
            if(0 == mask)
            {
                if(h == 0)
                {
                    t = 1;
                }
                else
                {
                    t = 0;
                }
            }
            else if(1 == mask) { t = 4;   }
            else if(2 == mask) { t = 5;  tp->voffset = 1; }
            else if(3 == mask) { t = 9;  tp->voffset = 1; }
            else if(4 == mask) { t = 6;   }
            else if(5 == mask) { t = 21;  }
            else if(6 == mask) { t = 10; tp->voffset = 1; }
            else if(7 == mask) { t = 13; tp->voffset = 1; }
            else if(8 == mask) { t = 7;   }
            else if(9 == mask) { t = 8;   }
            else if(10 == mask){ t = 20; tp->voffset = 1; }
            else if(11 == mask){ t = 12; tp->voffset = 1; }
            else if(12 == mask){ t = 11;  }
            else if(13 == mask){ t = 15;  }
            else if(14 == mask){ t = 14; tp->voffset = 1; }

            int ran = rand();
            int b = ran % building_count;
            int low_count = 0;
            if(hxy == 0) low_count++;
            if(hx1y == 0) low_count++;
            if(hxy1 == 0) low_count++;
            if(hx1y1 == 0) low_count++;

            tp->x = pixel.x;
            tp->y = pixel.y;
            tp->water = (hxy <= 0 || hx1y1 <= 0 || hxy1 <= 0 || hx1y <= 0);
            if(hxy < 0)
            {
                tp->terrain = &sand[t];
            }
            else if(low_count >= 2)
            {
                tp->terrain = tp->water ? &sand[t] : &grass[t];
            }
            else
            {
                tp->terrain = tp->water ? &sand_grass[t] : &grass[t];
            }
            tp->tile_id  = t;
            tp->building = !tp->water && tp->tile_id == 0 && ran % 6 == 0 ? &buildings[b] : NULL;
        }
    }
    /* SECOND PASS */
    for (int y = 1; y < GRID_SIZE - 1; y++)
    {
        for (int x = 1; x < GRID_SIZE - 1; x++)
        {
            int wc = tiles[x - 1][y - 1].water
             + tiles[x - 1][y + 1].water
             + tiles[x + 1][y - 1].water
             + tiles[x + 1][y + 1].water;

            if(wc > 0) tiles[x][y].terrain = &sand[tiles[x][y].tile_id];
        }
    }

    /* Load in the game UI. */
    const char* layout = "resources/layouts/game_ui.csv";
    Drawable* drawables = load_drawables(renderer, layout);

    /* Initialise the camera and update everything for the DEFAULT_SCALE. */
    Camera camera = { 1, 0, 0 };
    change_scale(&camera, renderer, drawables, -(DEFAULT_SCALE));

    /* Centre the top of the grid on the x axis. */
    camera.x = (TILE_WIDTH >> 1) - (DESIGN_WIDTH << (DEFAULT_SCALE - 1));

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
                    change_scale(&camera, renderer, drawables, event.wheel.y);
                    int factor = 1 == event.wheel.y ? camera.scale : -(camera.scale >> 1);
                    camera.x += factor * (zoom_mode == 0 ? DESIGN_WIDTH  >> 1 : mouse_x);
                    camera.y += factor * (zoom_mode == 0 ? DESIGN_HEIGHT >> 1 : mouse_y);
                }
            }
        }

        /* Screen widths can not change for the rest of the loop. */
        const int sw = DESIGN_WIDTH*camera.scale;
        const int sh = DESIGN_HEIGHT*camera.scale;

        /* Quit the program if Escape is pressed. */
        if (key_status[41])
        {
            status = SWITCHTO_MAINMENU;
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
        const int y1 = pixel_to_tile(camera.x + sw, camera.y     ).y;
        const int y2 = pixel_to_tile(camera.x,      camera.y + sh).y + 1;
        const int x1 = pixel_to_tile(camera.x,      camera.y     ).x;
        const int x2 = pixel_to_tile(camera.x + sw, camera.y + sh).x + 1;

        /* Make sure all these are within bounds of our grid.
           Loop through this sub-grid and render each texture. */
        /*for (int y = MAX(0, y1); y < MIN(GRID_SIZE, y2); y++)
        {
            for (int x = MAX(0, x1); x < MIN(GRID_SIZE, x2); x++)
            {*/
        for (int y = 0; y < GRID_SIZE; y++)
        {
            for (int x = 0; x < GRID_SIZE; x++)
            {
                tp = &tiles[x][y];
                int shift = (int)floor(TILE_HEIGHT / 6.0f);
                int rtx = tp->x - camera.x;
                int rty = tp->y - camera.y;
                int rtw = rty - shift;

                /* Enable for 3D mode. */
                if(x != GRID_SIZE - 1 && y != GRID_SIZE - 1)
                {
                    rty -= heightmap[x + 1][y + 1] * shift;
                }
                else if(x != GRID_SIZE - 1 && y == GRID_SIZE - 1)
                {
                    rty -= heightmap[x + 1][y] * shift;
                }
                else if(x == GRID_SIZE - 1 && y != GRID_SIZE - 1)
                {
                    rty -= heightmap[x][y + 1] * shift;
                }
                else
                {
                    rty -= heightmap[x][y] * shift;
                }
                rty += tp->voffset * shift;

                /* Only render if it will be visible on the screen. */
                if(rtx + TILE_WIDTH > 0 && rtx < sw && rty + TILE_HEIGHT > 0 && rty < sh)
                {
                    rect_terrain.x = rtx;
                    rect_terrain.y = rty;

                    SDL_RenderCopy(renderer, tp->terrain->texture, NULL, &rect_terrain);
                    if(tp->water)
                    {
                        rect_terrain.y = rtw;
                        SDL_RenderCopy(renderer, grass[2].texture, NULL, &rect_terrain);
                    }
                    if (tp->building)
                    {
                        /*rect_building.x = rtx;
                        rect_building.h = tp->building->height;
                        rect_building.y = rect_terrain.y - rect_building.h + (int) floor(TILE_HEIGHT);
                        SDL_RenderCopy(renderer, tp->building->texture, NULL, &rect_building);*/
                    }
                }
            }
        }

        /* Copy the game_ui layout drawables to the renderer. */
        render_drawables(renderer, drawables);

        /* Get the data we need for the debugging UI. */
        int centre[2] = {camera.x + (sw >> 1), camera.y + (sh >> 1) };
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
    destroy_drawables(drawables);
    TTF_CloseFont(debug_font);
    for (int i = 0; i < building_count; SDL_DestroyTexture(buildings[i++].texture));
    for (int i = 0; i <  terrain_count; SDL_DestroyTexture( grass[i++].texture));
    for (int i = 0; i <  terrain_count; SDL_DestroyTexture( sand[i++].texture));
    for (int i = 0; i <  terrain_count; SDL_DestroyTexture( sand_grass[i++].texture));

    SDL_RenderSetLogicalSize(renderer, DESIGN_WIDTH, DESIGN_HEIGHT);
    return status;
}
