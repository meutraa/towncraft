/*! \file diamond.h
    \brief A file for the Diamond-Square algorithm.
*/

#ifndef DIAMOND_H
#define DIAMOND_H

#define GRID_SIZE (1 << 7) + 1
#define ROUGHNESS 0.05f
#define HEIGHT 32
#define LOWER_HEIGHT 16.0f

/*! \fn void fill_heightmap(float height[GRID_SIZE][GRID_SIZE], int size, const int max, const float roughness)

    \brief Fills the given array with a heightmap.

    \param height a 2D array to fill.
    \param size the value of the highest 1D index.
    \param max the same as size.
    \param roughness determines how spikey the land will be.
*/
void fill_heightmap(float height[GRID_SIZE][GRID_SIZE], int size, const float roughness);

#endif
