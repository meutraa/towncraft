#include "diamond.h"

#include <stdlib.h>
#include <math.h>

const float ROUGHNESS = 0.075f;

static float add(int x, int y, int height[GRID_SIZE][GRID_SIZE], int* c)
{
    if(x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE)
    {
        (*c)++;
        return height[x][y];
    }
    return 0.0f;
}

static void square(int height[GRID_SIZE][GRID_SIZE], int x, int y, int size, float offset)
{
    int c = 0;
    float total = 0.0f;
    total += add(x - size, y - size, height, &c);
    total += add(x + size, y - size, height, &c);
    total += add(x + size, y + size, height, &c);
    total += add(x - size, y + size, height, &c);
    height[x][y] = (int) round(total / c + offset);
}

static void diamond(int height[GRID_SIZE][GRID_SIZE], int x, int y, int size, float offset)
{
    int c = 0;
    float total = 0.0f;
    total += add(x, y - size, height, &c);
    total += add(x + size, y, height, &c);
    total += add(x, y - size, height, &c);
    total += add(x - size, y, height, &c);
    height[x][y] = (int) round(total / c + offset);
}

void fill_heightmap(int height[GRID_SIZE][GRID_SIZE], int size, const float roughness)
{
    int half = size >> 1;
    if (half < 1) return;

    float scale = roughness * size;

    for(int y = half; y < GRID_SIZE - 1; y += size)
    {
        for(int x = half; x < GRID_SIZE - 1; x += size)
        {
            square(height, x, y, half, (rand()/(float)RAND_MAX * scale * 2.0f) - scale);
        }
    }
    for(int y = 0; y < GRID_SIZE; y += half)
    {
        for(int x = (y + half) % size; x < GRID_SIZE; x += size)
        {
            diamond(height, x, y, half, (rand()/(float)RAND_MAX * scale * 2.0f) - scale);
        }
    }
    fill_heightmap(height, size >> 1, roughness);
}
