/*! \file game.h
    \brief Access to the game loop.
*/

#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "status.h"

typedef struct Point {
    int x;                /*!< the top left pixel on the grid */
    int y;                /*!< the top left pixel on the grid */
} Point;

typedef struct FPoint {
    float x;              /*!< the top left pixel on the grid */
    float y;              /*!< the top left pixel on the grid */
} FPoint;

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
