#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL_opengl.h>

#include "SDL_image.h"
#include "constants.h"
#include "drawable.h"
#include "options.h"
#include "text.h"
#include "diamond.h"
#include "macros.h"
#include "status.h"

static int event_loop();

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
SDL_Texture *terrain, **buildings;
Drawable* drawables;
SDL_Window* win;
SDL_GLContext* con;
SDL_Renderer* ren;
int key_status[283];

static inline int  LENGTH(void** array)  { int l = 0; while(array[l]) l++; return l; }

/* The grid of tiles. */
static Tile tiles[GRID_SIZE][GRID_SIZE];
static int heightmap[GRID_SIZE][GRID_SIZE];

/* Tile dimensions must be divisible by exp2(DEFAULT_SCALE). */
static const int TILE_WIDTH  = 64;
static const int TILE_HEIGHT = 32;
static const int TILE_DEPTH  = 8;

static SDL_Rect src_rects[3][22];

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
        (x + y) * (TILE_HEIGHT >> 1)
    };
}

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
        //tp->building = !tp->water && tp->tile_id == 0 && rand() % 6 == 0 ? buildings[rand() % building_count] : NULL;
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

GLuint vbo_id;
#define vsize (18 * (GRID_SIZE ) * (GRID_SIZE))
GLint *v;

static void render_grid()
{
    glClear(GL_COLOR_BUFFER_BIT);
    // bind VBOs for vertex array and index array
    glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo_id);         // for vertex coordinates
    //glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboId2); // for indices


    // do same as vertex array except pointer
    glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
    glVertexPointer(3, GL_INT, 0, 0);               // last param is offset, not ptr

    glDrawArrays(GL_TRIANGLES, 0, vsize/3);

    glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex array
    glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
    SDL_GL_SwapWindow(win);
}

Status game_loop(SDL_Window* window, SDL_Renderer* renderer)
{
    memset(&key_status, 0, 283);
    win = window;
    ren = renderer;
    con = SDL_GL_CreateContext(win);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-DESIGN_WIDTH/2, DESIGN_WIDTH/2, DESIGN_HEIGHT, -DESIGN_HEIGHT, -50, 50);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    srand((unsigned int)time(NULL));

    /* Initialise globals. */
    generate_map();

    glClearColor(0, 1, 1, 1);

    v = malloc(vsize * sizeof(GLint));
    forXY(0, GRID_SIZE - 1)
    {
        int os = (y * GRID_SIZE + x) * 18;
        SDL_Point p1 = tile_to_pixel(x, y);
        SDL_Point p2 = tile_to_pixel(x + 1, y);
        SDL_Point p3 = tile_to_pixel(x + 1, y + 1);
        SDL_Point p4 = tile_to_pixel(x, y + 1);
        int h = TILE_DEPTH * heightmap[x][y];
        v[os + 0] = p1.x;  v[os + 1] = p1.y;  v[os + 2] = h;
        v[os + 3] = p2.x;  v[os + 4] = p2.y;  v[os + 5] = h;
        v[os + 6] = p3.x;  v[os + 7] = p3.y;  v[os + 8] = h;
        v[os + 9] = p3.x;  v[os + 10] = p3.y; v[os + 11] = h;
        v[os + 12] = p4.x; v[os + 13] = p4.y; v[os + 14] = h;
        v[os + 15] = p1.x; v[os + 16] = p1.y; v[os + 17] = h;
    }

    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLint) * vsize, v, GL_STATIC_DRAW);
    free(v);

    int status = 1;
    render_grid();
    while(status)
    {
        status = event_loop();
        int render = 0;
    #define SCROLL(a, b, c, d, e) if(key_status[a] || (fullscreen && b == c)) \
        { glTranslatef(scroll_speed * d, scroll_speed * e, 0); render = 1; }
        SCROLL(80, 0,    100,  1.0f,  0.0f)
        SCROLL(79, 1279, 100, -1.0f,  0.0f)
        SCROLL(82, 0,    100,  0.0f,  0.5f)
        SCROLL(81, 719,  100,  0.0f, -0.5f)
        if(render) render_grid();
        SDL_Delay(16);
    }

    /* Free any allocated memory. */
    glDeleteBuffers(1, &vbo_id);
    SDL_GL_DeleteContext(con);
    return SWITCHTO_MAINMENU;
}

static int event_loop()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(SDL_KEYUP == event.type || SDL_KEYDOWN == event.type)
        {
            key_status[event.key.keysym.scancode] = event.type == SDL_KEYUP ? 0 : 1;
            if(key_status[41]) return 0;
        }
        else if(SDL_MOUSEWHEEL == event.type)
        {
            /* Zoom in is 1, zoom out is -1 */
            if(event.wheel.y)
            {
                GLfloat scale = (GLfloat) (event.wheel.y < 0 ? 0.8 : 1.25);
                //glScalef(scale, scale, 0);
                const GLfloat r[] = { scale, 0, 0, 0,  0, scale, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
                glMultMatrixf(&r);
                render_grid();
            }
        }
    }
    return 1;
}
