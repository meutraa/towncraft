#include "diamond.h"

#include <stdlib.h>
#include <math.h>

#define HEIGHT 12
#define LOWER_HEIGHT 4
const float ROUGHNESS = 0.075f;

static int add(int x, int y, int height[GRID_SIZE][GRID_SIZE], int* c)
{
    if(x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE)
    {
        (*c)++;
        return height[x][y];
    }
    return 0;
}

static void diamond_square(int height[GRID_SIZE][GRID_SIZE], int size, const float roughness)
{
    const int half = size >> 1;
    if (half < 1) return;

    int x, y, c, total;
    const float scale = roughness * size;

    for(y = half; y < GRID_SIZE - 1; y += size)
    {
        for(x = half; x < GRID_SIZE - 1; x += size)
        {
            c = 0;
            total = add(x - half, y - half, height, &c)
                  + add(x + half, y - half, height, &c)
                  + add(x + half, y + half, height, &c)
                  + add(x - half, y + half, height, &c);
            height[x][y] = (int) round(((float)total / c) + ((rand()/(float)RAND_MAX * scale * 2.0f) - scale));
        }
    }
    for(y = 0; y < GRID_SIZE; y += half)
    {
        for(x = (y + half) % size; x < GRID_SIZE; x += size)
        {
            c = 0;
            total = add(x, y - half, height, &c)
                  + add(x + half, y, height, &c)
                  + add(x, y + half, height, &c)
                  + add(x - half, y, height, &c);
            height[x][y] = (int) round(((float)total / c) + ((rand()/(float)RAND_MAX * scale * 2.0f) - scale));
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
