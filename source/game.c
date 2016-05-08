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
SDL_Texture **buildings;
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

static GLuint vbo_id;
static unsigned long land_count, water_count;
#define vsize (12 * (GRID_SIZE ) * (GRID_SIZE))
static GLfloat scale;
float dx;
float dy;

static void render_grid()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_INT, 0, 0);

    glColor3ub(52, 171, 52);
    glDrawArrays(GL_TRIANGLES, 0, land_count/2);

    glColor3ub(102, 204, 255);
    glDrawArrays(GL_TRIANGLES, land_count / 2, (water_count - land_count) / 2);

    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    SDL_GL_SwapWindow(win);
}

static void update_view()
{
    glLoadIdentity();
    glOrtho(dx, (DESIGN_WIDTH * scale) + dx, dy, (DESIGN_HEIGHT * scale) + dy, -1, 1);
    render_grid();
}

Status game_loop(SDL_Window* window, SDL_Renderer* renderer)
{
    memset(&key_status, 0, 283);
    scale = 1.0f;
    dx = 0.0f, dy = 0.0f;
    win = window;
    ren = renderer;
    con = SDL_GL_CreateContext(win);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, DESIGN_WIDTH, 0, DESIGN_HEIGHT, -1, 1);

    srand((unsigned int)time(NULL));

    /* Initialise globals. */
    generate_map();

    glClearColor(0, 0, 0, 1);

    /* Create vertex array. */
    GLint *v = calloc(vsize, sizeof(GLint));
    unsigned long i = 0;
    forXY(0, GRID_SIZE - 1)
    {
        SDL_Point p1 = tile_to_pixel(x, y);
        SDL_Point p2 = tile_to_pixel(x + 1, y);
        SDL_Point p3 = tile_to_pixel(x + 1, y + 1);
        SDL_Point p4 = tile_to_pixel(x, y + 1);
        int h = TILE_DEPTH * heightmap[x][y];

        if(tiles[x][y].tile_id == 0)
        {
            if(h > 0)
            {
                v[i++] = p1.x; v[i++] = p1.y + h;
                v[i++] = p2.x; v[i++] = p2.y + h;
                v[i++] = p3.x; v[i++] = p3.y + h;
                v[i++] = p3.x; v[i++] = p3.y + h;
                v[i++] = p4.x; v[i++] = p4.y + h;
                v[i++] = p1.x; v[i++] = p1.y + h;
            }
        }
    }
    land_count = i;
    forXY(0, GRID_SIZE - 1)
    {
        SDL_Point p1 = tile_to_pixel(x, y);
        SDL_Point p2 = tile_to_pixel(x + 1, y);
        SDL_Point p3 = tile_to_pixel(x + 1, y + 1);
        SDL_Point p4 = tile_to_pixel(x, y + 1);

            if(heightmap[x][y] <= 0)
            {
                v[i++] = p1.x; v[i++] = p1.y;
                v[i++] = p2.x; v[i++] = p2.y;
                v[i++] = p3.x; v[i++] = p3.y;
                v[i++] = p3.x; v[i++] = p3.y;
                v[i++] = p4.x; v[i++] = p4.y;
                v[i++] = p1.x; v[i++] = p1.y;
            }
    }

    water_count = i;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLint) * i, v, GL_STATIC_DRAW);
    free(v);

    int status = 1;
    render_grid();
    while(status)
    {
        status = event_loop();
        int render = 0;
    #define SCROLL(a, b, c, d, e) if(key_status[a] || (fullscreen && b == c)) \
        { dx += scroll_speed * scale * d; \
          dy += scroll_speed * scale * e; \
          render = 1; }
        SCROLL(SDL_SCANCODE_A, 0,    100,  -1.0f,  0.0f)
        SCROLL(SDL_SCANCODE_D, 1279, 100,  1.0f,  0.0f)
        SCROLL(SDL_SCANCODE_W, 0,    100,  0.0f, 0.5f)
        SCROLL(SDL_SCANCODE_S, 719,  100,  0.0f, -0.5f)
        if(render) update_view();
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
                int mouse_x, mouse_y;
                int zoomout = event.wheel.y < 0, zoomin = !zoomout;
                SDL_GetMouseState(&mouse_x, &mouse_y);
                GLfloat gscale = zoomin ? 0.5 : 2.0f;
                float oldscale = scale;
                scale *= gscale;

                if(zoomin)
                {
                    dx += mouse_x * scale;
                    dy += (DESIGN_HEIGHT - mouse_y) * scale;
                }
                else
                {
                    dx -= mouse_x * oldscale;
                    dy -= (DESIGN_HEIGHT - mouse_y) * oldscale;
                }

                update_view();
            }
        }
    }
    return 1;
}
