#ifndef COLOR_H
#define COLOR_H

typedef struct Color {
    GLubyte r, g, b, a;
} Color;

#define DARK 0.56f
#define LIGHT 1.23f

#define GRASS_R 51
#define GRASS_G 185
#define GRASS_B 51
static const Color grass_base  = { GRASS_R, GRASS_G, GRASS_B, 0 };
static const Color grass_light = { (int)(GRASS_R * LIGHT), (int)(GRASS_G * LIGHT), (int)(GRASS_B * LIGHT), 0 };
static const Color grass_dark  = { (int)(GRASS_R * DARK), (int)(GRASS_G * DARK), (int)(GRASS_B * DARK), 0 };

#define SAND_R 204
#define SAND_G 171
#define SAND_B 101
static const Color sand_base  = { SAND_R, SAND_G, SAND_B, 0 };
static const Color sand_light = {(int)(SAND_R * LIGHT),(int)(SAND_G * LIGHT),(int)(SAND_B * LIGHT), 0 };
static const Color sand_dark  = { (int)(SAND_R * DARK), (int)(SAND_G * DARK), (int)(SAND_B * DARK), 0 };

static const Color water      = { 100, 173, 199, 192 };

#endif
