/*! \file diamond.h
    \brief A file for the Diamond-Square algorithm.
*/

#ifndef DIAMOND_H
#define DIAMOND_H

#define GRID_SIZE ((1 << 6) + 1)
extern const float ROUGHNESS;

/*! \fn void fill_heightmap(float height[GRID_SIZE][GRID_SIZE], int size, const int max, const float roughness)

    \brief Fills the given array with a heightmap.

    \param height a 2D array to fill.
    \param size the value of the highest 1D index.
    \param max the same as size.
    \param roughness determines how spikey the land will be.
*/
void fill_heightmap(int height[GRID_SIZE][GRID_SIZE], int size, const float roughness);

#endif
