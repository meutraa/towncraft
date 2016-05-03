#include "diamond.h"

#include <stdlib.h>

static float frand()
{
    return (float)rand()/(float)(RAND_MAX);
}

static float add(int x, int y, float height[GRID_SIZE][GRID_SIZE], int* c)
{
    if(x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE)
    {
        (*c)++;
        return height[x][y];
    }
    return 0.0f;
}

static void square(float height[GRID_SIZE][GRID_SIZE], int x, int y, int size, float offset)
{
    int c = 0;
    float total = 0.0f;
    total += add(x - size, y - size, height, &c);
    total += add(x + size, y - size, height, &c);
    total += add(x + size, y + size, height, &c);
    total += add(x - size, y + size, height, &c);
    height[x][y] = total / c + offset;
}

static void diamond(float height[GRID_SIZE][GRID_SIZE], int x, int y, int size, float offset)
{
    int c = 0;
    float total = 0.0f;
    total += add(x, y - size, height, &c);
    total += add(x + size, y, height, &c);
    total += add(x, y - size, height, &c);
    total += add(x - size, y, height, &c);
    height[x][y] = total / c + offset;
}

void fill_heightmap(float height[GRID_SIZE][GRID_SIZE], int size, const float roughness)
{
    int half = size >> 1;
    if (half < 1) return;

    float scale = roughness * size;

    for(int y = half; y < GRID_SIZE - 1; y += size)
    {
        for(int x = half; x < GRID_SIZE - 1; x += size)
        {
            square(height, x, y, half, (frand() * scale * 2.0f) - scale);
        }
    }
    for(int y = 0; y <= GRID_SIZE - 1; y += half)
    {
        for(int x = (y + half) % size; x <= GRID_SIZE - 1; x += size)
        {
            diamond(height, x, y, half, (frand() * scale * 2.0f) - scale);
        }
    }
    fill_heightmap(height, size >> 1, roughness);
}
