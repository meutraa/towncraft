/*! \file tile.h
	\brief A structure that simplifies scaling SDL_Textures.
*/

#ifndef TILE_H
#define TILE_H

#include "SDL.h"

/*!
	\brief A structure that contains some values for tiles.
*/
typedef struct Tile {
    SDL_Texture* tile_texture; /*!< a pointer to an SDL_Texture */
    SDL_Texture* sprite_texture; /*!< a pointer to am SDL_Texture */
    int tile_id;
    float x; /*!< x position of tile. */
    float y; /*!< y position of tile. */
} Tile;

#endif
