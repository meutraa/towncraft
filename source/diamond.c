#include "diamond.h"

#include <stdlib.h>

static const int HEIGHT = 18;
static const int LOWER_HEIGHT = 9;
const float ROUGHNESS = 0.1f;
#define ROUND(x) ((x) >= 0 ? (int)((x) + 0.5f) : (int)((x) - 0.5f))

static void diamond_square(int height[GRID_SIZE][GRID_SIZE], int size, const float roughness)
{
    int half = size >> 1, ymh, yph, xmh, xph;
    int yl, yh, xl, xh;
    if (half < 1) return;

    int x, y, c, total;
    const float A = (roughness * size) / (2.0f * RAND_MAX);
    const float B = RAND_MAX >> 1;

    for(y = half; y < GRID_SIZE - 1; y += size)
    {
        ymh = y - half , yph = y + half;
        yl = ymh >= 0, yh = yph < GRID_SIZE;
        for(x = half; x < GRID_SIZE - 1; x += size)
        {
            total = 0, c = 0, xmh = x - half, xph = x + half;
            xl = xmh >= 0, xh = xph < GRID_SIZE;
            if(xl && yl) { c++; total += height[xmh][ymh]; }
            if(xh && yl) { c++; total += height[xph][ymh]; }
            if(xh && yh) { c++; total += height[xph][yph]; }
            if(xl && yh) { c++; total += height[xmh][yph]; }

            height[x][y] = ROUND(((float)total / c) + (A * (rand() - B)));
        }
    }
    for(y = 0; y < GRID_SIZE; y += half)
    {
        ymh = y - half, yph = y + half;
        for(x = (y + half) % size; x < GRID_SIZE; x += size)
        {
            total = 0, c = 0, xmh = x - half, xph = x + half;
            if(ymh >= 0)        { c++; total += height[x][ymh]; }
            if(xph < GRID_SIZE) { c++; total += height[xph][y]; }
            if(yph < GRID_SIZE) { c++; total += height[x][yph]; }
            if(xmh >= 0)        { c++; total += height[xmh][y]; }
            height[x][y] = ROUND(((float)total / c) + (A * (rand() - B)));
        }
    }
    diamond_square(height, size >> 1, roughness);
}

void fill_heightmap(int height[GRID_SIZE][GRID_SIZE], int size, const float roughness)
{
    height[0]   [size] = (rand() % HEIGHT) - LOWER_HEIGHT;
    height[size][0]    = (rand() % HEIGHT) - LOWER_HEIGHT;
    height[0]   [0]    = (rand() % HEIGHT) - LOWER_HEIGHT;
    height[size][size] = (rand() % HEIGHT) - LOWER_HEIGHT;
    diamond_square(height, size, roughness);
}
