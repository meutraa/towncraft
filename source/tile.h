/*! \file tile.h
	\brief A structure that simplifies scaling SDL_Textures.
*/

#ifndef TILE_H
#define TILE_H

#include "SDL.h"

/*!
	\brief A structure that contains some values for tiles.
*/
typedef struct Tile
{
	SDL_Texture* texture;   /*!< a pointer to an SDL_Texture */
	float x;
	float y;
} Tile;

#endif
