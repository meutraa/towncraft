#include <epoxy/gl.h>
#include <epoxy/glx.h>

#include <stdlib.h>
#include <string.h>

#include "diamond.h"
#include "map.h"
#include "color.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define TILES ((GRID_SIZE - 1) * (GRID_SIZE - 1))
#define NUM_TRIANGLES TILES * 2

int WATER_LEVEL = 0;

static const int maskmap[32] = {
    0, 4, 5, 9, 6, 21, 10, 13, 7, 8, 20, 12, 11, 15, 14, [23] = 16, [27] = 17, [29] = 18, [30] = 19
};

/* The grid of tiles. */
static int heightmap[GRID_SIZE][GRID_SIZE];
#define hxy heightmap[x][y]
#define hx1y heightmap[x + 1][y]
#define hxy1 heightmap[x][y + 1]
#define h1xy heightmap[x + 1][y + 1]

static Tile tiles[GRID_SIZE][GRID_SIZE];
static const int TILE_WIDTH  = 64;
static const int TILE_HEIGHT = 32;
static const int TILE_DEPTH  = 8;

static Point tile_to_pixel(int x, int y, int z)
{
    return (Point) {
        (x - y) * (TILE_WIDTH >> 1),
        ((x + y) * (TILE_HEIGHT >> 1)) + z,
    };
}

static GLubyte* app_tri_colors(GLubyte *ar, Color c)
{
    *(ar++) = c.r; *(ar++) = c.g; *(ar++) = c.b; *(ar++) = c.a;
    *(ar++) = c.r; *(ar++) = c.g; *(ar++) = c.b; *(ar++) = c.a;
    *(ar++) = c.r; *(ar++) = c.g; *(ar++) = c.b; *(ar++) = c.a;
    return ar;
}

static GLint* app_tri_vertices(GLint *ar, Point p1, Point p2, Point p3)
{
    *(ar++) = p1.x; *(ar++) = p1.y;
    *(ar++) = p2.x; *(ar++) = p2.y;
    *(ar++) = p3.x; *(ar++) = p3.y;
    return ar;
}

GLuint create_map(GLuint* map_vbo_id)
{
    fill_heightmap(heightmap, GRID_SIZE - 1, ROUGHNESS);
    return recreate_map(map_vbo_id);
}

GLuint recreate_map(GLuint* map_vbo_id)
{
    if(*map_vbo_id)
    {
        glDeleteBuffers(1, map_vbo_id);
    }

    /* Create vertex array. */
    /* *2 because we assume every tile has an extra water tile. */
    unsigned long size_vertices = sizeof(GLint)   * NUM_VERTEX * NUM_TRIANGLES * 2;
    unsigned long size_colors   = sizeof(GLubyte) * NUM_COLOR  * NUM_TRIANGLES * 2;
    GLint   *v = malloc(size_vertices);
    GLubyte *c = malloc(size_colors);
    GLint   *vp = v;
    GLubyte *cp = c;
    GLuint triangle_count = 0;
    for(int y = 0; y < GRID_SIZE - 1; y++)
    for(int x = 0; x < GRID_SIZE - 1; x++)
    {
        Tile *tp = &tiles[x][y];
        tp->b = tile_to_pixel(x,     y,     TILE_DEPTH *  hxy);
        tp->l = tile_to_pixel(x + 1, y,     TILE_DEPTH * hx1y);
        tp->t = tile_to_pixel(x + 1, y + 1, TILE_DEPTH * h1xy);
        tp->r = tile_to_pixel(x,     y + 1, TILE_DEPTH * hxy1);

        int low = MIN(MIN(MIN(hxy, h1xy), hx1y), hxy1);

        int steep = abs(h1xy - hxy) >= 2 || abs(hxy1 - hx1y) >= 2;
        int mask = (hxy1 > low) | ((hxy > low) << 1) | ((hx1y > low) << 2) | ((h1xy > low) << 3) | (steep << 4);

        tiles[x][y].water = h1xy <= WATER_LEVEL || hxy1 <= WATER_LEVEL || hx1y <= WATER_LEVEL || hxy <= WATER_LEVEL;
        tiles[x][y].tile_id = maskmap[mask];

        /* where will the tile's color be split? */
        if(tp->l.y != tp->r.y)
        {
            vp = app_tri_vertices(vp, tp->t, tp->r, tp->b);
            vp = app_tri_vertices(vp, tp->b, tp->l, tp->t);
        }
        else /* vertical split, or if not, the colors will be the same anyway. */
        {
            vp = app_tri_vertices(vp, tp->r, tp->l, tp->t);
            vp = app_tri_vertices(vp, tp->b, tp->l, tp->r);
        }
        triangle_count += 2;

        int sand = hxy <= WATER_LEVEL + 1;

        /* First and second triangle. Top/left then bottom/right. */
        switch(tiles[x][y].tile_id)
        {
            case 0: case 5: case 6: case 12: case 15:
                cp = app_tri_colors(cp, sand ?  sand_base : grass_base);
                break;
            case 4: case 8: case 9: case 13: case 16: case 17: case 21:
                cp = app_tri_colors(cp, sand ? sand_light : grass_light);
                break;
            case 7: case 10: case 11: case 14: case 18: case 19: case 20:
                cp = app_tri_colors(cp, sand ?  sand_dark : grass_dark);
                break;
        }
        switch(tiles[x][y].tile_id)
        {
            case 0: case 4: case 7: case 13: case 14:
                cp = app_tri_colors(cp, sand ?  sand_base : grass_base);
                break;
            case 5: case 8: case 9: case 12: case 16: case 17: case 20:
                cp = app_tri_colors(cp, sand ? sand_light : grass_light);
                break;
            case 6: case 10: case 11: case 15: case 18: case 19: case 21:
                cp = app_tri_colors(cp, sand ?  sand_dark : grass_dark);
                break;
        }
    }
    /* Add the water stuff. */
    int woffset = TILE_DEPTH * (WATER_LEVEL + 1);
    for(int y = 0; y < GRID_SIZE - 1; y++)
    for(int x = 0; x < GRID_SIZE - 1; x++)
    {
        Tile *tp = &tiles[x][y];

        /* Add color vertices. */
        if(tiles[x][y].water)
        {
            Point pb = tile_to_pixel(x,     y,     woffset);
            Point pl = tile_to_pixel(x + 1, y,     woffset);
            Point pt = tile_to_pixel(x + 1, y + 1, woffset);
            Point pr = tile_to_pixel(x,     y + 1, woffset);

            Color water_shade = water;
            water_shade.r += 2*hxy;
            water_shade.g += 2*hxy;
            water_shade.b += 2*hxy;
            water_shade.a = 212;

            /* Only render triangles with land underneath them. */
                 if(12 == tp->tile_id && hx1y >= WATER_LEVEL) vp = app_tri_vertices(vp, pb, pl, pt);
            else if(13 == tp->tile_id && hxy  >  WATER_LEVEL) vp = app_tri_vertices(vp, pl, pt, pr);
            else if(14 == tp->tile_id && hxy1 >= WATER_LEVEL) vp = app_tri_vertices(vp, pb, pr, pt);
            else if(15 == tp->tile_id && h1xy >  WATER_LEVEL) vp = app_tri_vertices(vp, pl, pb, pr);
            else
            {
                vp = app_tri_vertices(vp, pt, pr, pb);
                vp = app_tri_vertices(vp, pb, pl, pt);
                cp = app_tri_colors(cp, water_shade);
                triangle_count++;
            }
            cp = app_tri_colors(cp, water_shade);
            triangle_count++;
        }
    }

    size_vertices = sizeof(GLint)   * NUM_VERTEX * triangle_count;
    size_colors   = sizeof(GLubyte) * NUM_COLOR  * triangle_count;

    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    *map_vbo_id = vbo_id;

    /* Copy vertex and color arrays to an single data buffer. */
    GLint *all = malloc(size_vertices + size_colors);
    memmove(all, v, size_vertices);
    memmove(all + (NUM_VERTEX * triangle_count), c, size_colors);

    glBufferData(GL_ARRAY_BUFFER, size_vertices + size_colors, all, GL_STATIC_DRAW);
    free(v);
    free(c);
    free(all);
    return triangle_count;
}
