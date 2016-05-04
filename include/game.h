/*! \file game.h
    \brief Access to the game loop.
*/

#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "status.h"

/*!
    \brief A structure that contains some values for tiles.
*/
typedef struct Tile {
    SDL_Texture* building;  /*!< a pointer to an Building. */
    SDL_Rect* src;          /*!< a pointer to a Terrain. */
    int tile_id;            /*!< an id. */
    int terrain_id;
    int voffset;
    int water;
    int x;              /*!< x position of tile. */
    int y;              /*!< y position of tile. */
} Tile;

/*!
    \brief Camera infomation.
*/
typedef struct Camera {
    int scale;              /*!< 1 should be lowest, and you should bitshift */
    int x;                  /*!< the top left pixel on the grid */
    int y;                  /*!< the top left pixel on the grid */
} Camera;

/*! Status game_loop(SDL_Renderer* renderer)
    \brief Starts a loop that renders the game screen.
    \param renderer used for all rendering inside the game.
    \return QUIT_PROGRAM or NORMAL enums.
*/
Status game_loop(SDL_Renderer* renderer);

#endif
