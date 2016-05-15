#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
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

const char* layout = "resources/layouts/game_ui.csv";

static const int maskmap[32] = {
    0, 4, 5, 9, 6, 21, 10, 13, 7, 8, 20, 12, 11, 15, 14, [23] = 16, [27] = 17, [29] = 18, [30] = 19
};

/* Free up some space in the stack. */
Drawable* drawables;
SDL_Window* win;
SDL_GLContext* con;
SDL_Renderer* ren;
int key_status[283];

static GLenum render_mode = GL_FILL;

/* The grid of tiles. */
static int heightmap[GRID_SIZE][GRID_SIZE];

/* Tile dimensions must be divisible by exp2(DEFAULT_SCALE). */
static const int TILE_WIDTH  = 64;
static const int TILE_HEIGHT = 32;
static const int TILE_DEPTH  = 8;

static SDL_Point tile_to_pixel(int x, int y)
{
    return (SDL_Point) {
        (x - y) * (TILE_WIDTH >> 1),
        (x + y) * (TILE_HEIGHT >> 1)
    };
}

static Tile tiles[GRID_SIZE][GRID_SIZE];

static GLuint vbo_id;
#define TILES ((GRID_SIZE - 1) * (GRID_SIZE - 1))

#define NUM_VERTEX 12
#define NUM_COLOR 18
static GLfloat scale;
float dx;
float dy;

static void render_grid()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(2, GL_INT, 0, 0);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, TILES * NUM_VERTEX * sizeof(GLint));

    glDrawArrays(GL_TRIANGLES, 0, TILES * 6);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
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

    /* Create the map. */
    fill_heightmap(heightmap, GRID_SIZE - 1, ROUGHNESS);

    forXY(0, GRID_SIZE - 1)
    {
        SDL_Point pixel = tile_to_pixel(x , y);

        int u = hxy, d = h1xy, l = hxy1, r = hx1y;

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
        tp->water = u <= 0 || l <= 0 || r <= 0 || d <= 0;
        tp->tile_id  = !mask ? !u : maskmap[mask];
    }

    glClearColor(0, 0, 0, 1);
    glShadeModel(GL_FLAT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_TEXTURE_2D);

    /* Create vertex array. */
    unsigned long size_vertices = sizeof(GLint) * NUM_VERTEX * TILES;
    unsigned long size_colors   = sizeof(GLubyte) * NUM_COLOR * TILES;
    GLint *v = malloc(size_vertices);
    GLubyte *c = malloc(size_colors);
    int i = 0, j = 0;
    forXY(0, GRID_SIZE - 1)
    {
        SDL_Point p1 = tile_to_pixel(x, y);
        SDL_Point p2 = tile_to_pixel(x + 1, y);
        SDL_Point p3 = tile_to_pixel(x + 1, y + 1);
        SDL_Point p4 = tile_to_pixel(x, y + 1);

        p1.y += TILE_DEPTH * hxy;
        p2.y += TILE_DEPTH * hx1y;
        p3.y += TILE_DEPTH * h1xy;
        p4.y += TILE_DEPTH * hxy1;
        /* where will the tile's color be split? */
        if(p2.y != p4.y)
        {
            add_triangle(p3, p4, p1)
            add_triangle(p1, p2, p3)
        }
        else /* vertical split, or if not, the colors will be the same anyway. */
        {
            add_triangle(p1, p2, p4)
            add_triangle(p4, p2, p3)
        }

        const int id = tiles[x][y].tile_id;
        /* Add color vertices. */
        if(tiles[x][y].water)
        {
            add_triangle_color(100, 173, 199)
            add_triangle_color(100, 173, 199)
        }
        else if(0 == id)
        {
            add_triangle_color(52, 187, 52)
            add_triangle_color(52, 187, 52)
        }
        else if(4 == id)
        {
            add_triangle_color(63, 221, 63)
            add_triangle_color(52, 187, 52)
        }
        else if(5 == id)
        {
            add_triangle_color(52, 187, 52)
            add_triangle_color(63, 211, 63)
        }
        else if(6 == id)
        {
            add_triangle_color(52, 187, 52)
            add_triangle_color(35, 136, 35)
        }
        else if(7 == id)
        {
            add_triangle_color(35, 136, 35)
            add_triangle_color(52, 187, 52)
        }
        else if(8 == id || 9 == id)
        {
            add_triangle_color(64, 224, 64)
            add_triangle_color(64, 224, 64)
        }
        else if(10 == id || 11 == id)
        {
            add_triangle_color(35, 136, 35)
            add_triangle_color(35, 136, 35)
        }
        else if(12 == id)
        {
            add_triangle_color(52, 187, 52)
            add_triangle_color(64, 224, 64)
        }
        else if(13 == id)
        {
            add_triangle_color(64, 224, 64)
            add_triangle_color(52, 187, 52)
        }
        else if(14 == id)
        {
            add_triangle_color(35, 136, 35)
            add_triangle_color(52, 187, 52)
        }
        else if(15 == id)
        {
            add_triangle_color(52, 187, 52)
            add_triangle_color(35, 136, 35)
        }
        else if(16 == id || 17 == id)
        {
            add_triangle_color(64, 224, 64)
            add_triangle_color(64, 224, 64)
        }
        else if(18 == id || 19 == id)
        {
            add_triangle_color(63, 187, 63)
            add_triangle_color(63, 187, 63)
        }
        else if(20 == id)
        {
            add_triangle_color(34, 133, 34)
            add_triangle_color(62, 219, 62)
        }
        else if(21 == id)
        {
            add_triangle_color(62, 219, 62)
            add_triangle_color(34, 133, 34)
        }
    }
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

    GLint *all = malloc(size_vertices + size_colors);
    memmove(all, v, size_vertices);
    free(v);
    memmove(all + (NUM_VERTEX * TILES), c, size_colors);
    free(c);

    glBufferData(GL_ARRAY_BUFFER, size_vertices + size_colors, all, GL_STATIC_DRAW);
    free(all);

    int status = 1;
    render_grid();
    while(status)
    {
        status = event_loop();
        int render = 0;
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
            if(SDL_SCANCODE_RETURN == event.key.keysym.scancode
                && SDL_KEYDOWN == event.key.type && 0 == event.key.repeat)
            {
                render_mode = render_mode == GL_FILL ? GL_LINE : GL_FILL;
                glPolygonMode(GL_FRONT_AND_BACK, render_mode);
                render_grid();
            }
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
