#ifndef MAP_H
#define MAP_H

#define NUM_VERTEX 6
#define NUM_COLOR 12

typedef struct Point {
    GLint x, y;
} Point;

typedef struct Tile {
    int tile_id;
    int water;
    Point t, b, l, r;
} Tile;

/* Returns count of triangles. */
GLuint create_map(GLuint* map_vbo_id);

#endif
