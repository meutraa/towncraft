#include "diamond.h"

#include <stdlib.h>

static float frand()
{
    return (float)rand()/(float)(RAND_MAX);
}

static void square(float height[GRID_SIZE][GRID_SIZE], int x, int y, int size, float offset)
{
    int count = 0;
    float total = 0.0f;
    if(x - size >= 0 && x - size < GRID_SIZE && y - size >= 0 && y - size < GRID_SIZE){
        count++;
        total += height[x - size][y - size];
    }
    if(x + size >= 0 && x + size < GRID_SIZE && y - size >= 0 && y - size < GRID_SIZE){
        count++;
        total += height[x + size][y - size];
    }
    if(x + size >= 0 && x + size < GRID_SIZE && y + size >= 0 && y + size < GRID_SIZE){
        count++;
        total += height[x + size][y + size];
    }
    if(x - size >= 0 && x - size < GRID_SIZE && y + size >= 0 && y + size < GRID_SIZE){
        count++;
        total += height[x - size][y + size];
    }
    total /= (float)count;
    height[x][y] = total + offset;
}

static void diamond(float height[GRID_SIZE][GRID_SIZE], int x, int y, int size, float offset)
{
    int count = 0;
    float total = 0.0f;
    if(x >= 0 && x < GRID_SIZE && y - size >= 0 && y - size < GRID_SIZE){
        count++;
        total += height[x][y - size];
    }
    if(x + size >= 0 && x + size < GRID_SIZE && y >= 0 && y < GRID_SIZE){
        count++;
        total += height[x + size][y];
    }
    if(x >= 0 && x < GRID_SIZE && y + size >= 0 && y + size < GRID_SIZE){
        count++;
        total += height[x][y + size];
    }
    if(x - size >= 0 && x - size < GRID_SIZE && y >= 0 && y < GRID_SIZE){
        count++;
        total += height[x - size][y];
    }
    total /= (float)count;
    height[x][y] = total + offset;
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
