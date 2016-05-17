#include <epoxy/gl.h>
#include <epoxy/glx.h>

#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>

#include "options.h"
#include "color.h"
#include "constants.h"
#include "diamond.h"

typedef struct Point {
    GLint x, y;
} Point;

static const int maskmap[32] = {
    0, 4, 5, 9, 6, 21, 10, 13, 7, 8, 20, 12, 11, 15, 14, [23] = 16, [27] = 17, [29] = 18, [30] = 19
};

static GLenum render_modes[] = { GL_FILL, GL_FILL, GL_LINE, GL_POINT };
static int render_mode = 0;

/* The grid of tiles. */
static int heightmap[GRID_SIZE][GRID_SIZE];
#define hxy heightmap[x][y]
#define hx1y heightmap[x + 1][y]
#define hxy1 heightmap[x][y + 1]
#define h1xy heightmap[x + 1][y + 1]

static const int TILE_WIDTH  = 64;
static const int TILE_HEIGHT = 32;
static const int TILE_DEPTH  = 8;

static Point tile_to_pixel(int x, int y)
{
    return (Point) {
        (x - y) * (TILE_WIDTH >> 1),
        (x + y) * (TILE_HEIGHT >> 1)
    };
}

static Tile tiles[GRID_SIZE][GRID_SIZE];

static GLuint vbo_id;
#define TILES ((GRID_SIZE - 1) * (GRID_SIZE - 1))
#define NUM_TRIANGLES TILES * 2
static unsigned long tri_count;

static GLfloat scale, dx, dy;

#define NUM_COLOR 12
static GLubyte* app_tri_colors(GLubyte *ar, Color c)
{
    *(ar++) = c.r; *(ar++) = c.g; *(ar++) = c.b; *(ar++) = c.a;
    *(ar++) = c.r; *(ar++) = c.g; *(ar++) = c.b; *(ar++) = c.a;
    *(ar++) = c.r; *(ar++) = c.g; *(ar++) = c.b; *(ar++) = c.a;
    return ar;
}

#define NUM_VERTEX 6
static GLint* app_tri_vertices(GLint *ar, Point p1, Point p2, Point p3)
{
    *(ar++) = p1.x; *(ar++) = p1.y;
    *(ar++) = p2.x; *(ar++) = p2.y;
    *(ar++) = p3.x; *(ar++) = p3.y;
    return ar;
}

static void render_grid(GLFWwindow* window)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(2, GL_INT, 0, 0);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, (void*) (tri_count * NUM_VERTEX * sizeof(GLint)));

    glDrawArrays(GL_TRIANGLES, 0, tri_count * 3);

    glDisableClientState(GL_COLOR_ARRAY);

    if(render_mode % 4 == 1)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor4ub(0, 0, 0, 16);
        glDrawArrays(GL_TRIANGLES, 0, tri_count * 3);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glfwSwapBuffers(window);
}

static void update_view(GLFWwindow* window)
{
    glLoadIdentity();
    glOrtho(dx, (DESIGN_WIDTH * scale) + dx, dy, (DESIGN_HEIGHT * scale) + dy, -1, 1);
    render_grid(window);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(GLFW_KEY_ENTER == key && action == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, render_modes[++render_mode % 4]);
        render_grid(window);
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    double mouse_x, mouse_y;
    int zoomin = yoffset > 0;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    float oldscale = scale;
    scale *= zoomin ? 0.5 : 2.0f;
    dx += mouse_x * (zoomin ? scale : -oldscale);
    dy += (DESIGN_HEIGHT - mouse_y) * (zoomin ? scale : -oldscale);
    update_view(window);
}

int game_loop(GLFWwindow* window)
{
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    scale = 1.0f;
    dx = 0.0f, dy = 0.0f;

    srand((unsigned int)time(NULL));

    /* Create the map. */
    fill_heightmap(heightmap, GRID_SIZE - 1, ROUGHNESS);

    for(int y = 0; y < GRID_SIZE - 1; y++)
    for(int x = 0; x < GRID_SIZE - 1; x++)
    {
        Point pixel = tile_to_pixel(x , y);

        /* Get lowest corner. Loop deliberately unrolled.*/
        int low = h1xy;
        if(hxy < low) low = hxy;
        if(hxy1 < low) low = hxy1;
        if(hx1y < low) low = hx1y;

        int steep = abs(h1xy - hxy) >= 2 || abs(hxy1 - hx1y) >= 2;
        int mask = (hxy1 > low) | ((hxy > low) << 1) | ((hx1y > low) << 2) | ((h1xy > low) << 3) | (steep << 4);

        tiles[x][y].x = pixel.x;
        tiles[x][y].y = pixel.y;
        tiles[x][y].water = h1xy <= 0 || hxy1 <= 0 || hx1y <= 0 || hxy <= 0;
        tiles[x][y].tile_id = maskmap[mask];
    }

    /* Create vertex array. */
    /* *2 because we assume every tile has an extra water tile. */
    unsigned long size_vertices = sizeof(GLint) * NUM_VERTEX * NUM_TRIANGLES * 2;
    unsigned long size_colors   = sizeof(GLubyte) * NUM_COLOR * NUM_TRIANGLES * 2;
    GLint   *v = malloc(size_vertices);
    GLubyte *c = malloc(size_colors);
    GLint   *vp = v;
    GLubyte *cp = c;
    tri_count = 0;
    for(int y = 0; y < GRID_SIZE - 1; y++)
    for(int x = 0; x < GRID_SIZE - 1; x++)
    {
        Point p1 = tile_to_pixel(x, y),
            p2 = tile_to_pixel(x + 1, y),
            p3 = tile_to_pixel(x + 1, y + 1),
            p4 = tile_to_pixel(x, y + 1);

        p1.y += TILE_DEPTH * hxy;
        p2.y += TILE_DEPTH * hx1y;
        p3.y += TILE_DEPTH * h1xy;
        p4.y += TILE_DEPTH * hxy1;

        /* where will the tile's color be split? */
        if(p2.y != p4.y)
        {
            vp = app_tri_vertices(vp, p3, p4, p1);
            vp = app_tri_vertices(vp, p1, p2, p3);
        }
        else /* vertical split, or if not, the colors will be the same anyway. */
        {
            vp = app_tri_vertices(vp, p4, p2, p3);
            vp = app_tri_vertices(vp, p1, p2, p4);
        }
        tri_count += 2;

        int sand = hxy <= 1;

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
    for(int y = 0; y < GRID_SIZE - 1; y++)
    for(int x = 0; x < GRID_SIZE - 1; x++)
    {
        Point p1 = tile_to_pixel(x, y),
            p2 = tile_to_pixel(x + 1, y),
            p3 = tile_to_pixel(x + 1, y + 1),
            p4 = tile_to_pixel(x, y + 1);

        /* Add color vertices. */
        if(tiles[x][y].water)
        {
            p1.y += TILE_DEPTH;
            p2.y += TILE_DEPTH;
            p3.y += TILE_DEPTH;
            p4.y += TILE_DEPTH;
                 if(12 == tiles[x][y].tile_id && hx1y >= 0) vp = app_tri_vertices(vp, p1, p2, p3);
            else if(13 == tiles[x][y].tile_id && hxy  >  0) vp = app_tri_vertices(vp, p2, p3, p4);
            else if(14 == tiles[x][y].tile_id && hxy1 >= 0) vp = app_tri_vertices(vp, p1, p4, p3);
            else if(15 == tiles[x][y].tile_id && h1xy >  0) vp = app_tri_vertices(vp, p2, p1, p4);
            else
            {
                vp = app_tri_vertices(vp, p3, p4, p1);
                vp = app_tri_vertices(vp, p1, p2, p3);
                cp = app_tri_colors(cp, water);
                tri_count++;
            }
            cp = app_tri_colors(cp, water);
            tri_count++;
        }
    }

    size_vertices = sizeof(GLint) * NUM_VERTEX * tri_count;
    size_colors   = sizeof(GLubyte) * NUM_COLOR * tri_count;

    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

    GLint *all = malloc(size_vertices + size_colors);
    memmove(all, v, size_vertices);
    free(v);
    memmove(all + (NUM_VERTEX * tri_count), c, size_colors);
    free(c);

    glBufferData(GL_ARRAY_BUFFER, size_vertices + size_colors, all, GL_STATIC_DRAW);
    free(all);

    glMatrixMode(GL_PROJECTION);
    update_view(window);
    while(GLFW_PRESS != glfwGetKey(window, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float speed = scroll_speed * scale;
        int r = 0;
        if(GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)) { dx -= speed; r = 1; }
        if(GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)) { dx += speed; r = 1; }
        if(GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)) { dy += speed / 2; r = 1; }
        if(GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)) { dy -= speed / 2; r = 1; }
        if(r) update_view(window);
    }

    /* Free any allocated memory. */
    glDeleteBuffers(1, &vbo_id);
    return 0;
}
