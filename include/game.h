/*! \file game.h
    \brief Access to the game loop.
*/

#ifndef GAME_H
#define GAME_H

#include <GLFW/glfw3.h>

/*!
    \brief A structure that contains some values for tiles.
*/
typedef struct Tile {
    int tile_id;            /*!< an id. */
    int terrain_id;
    int water;
    int x;              /*!< x position of tile. */
    int y;              /*!< y position of tile. */
} Tile;

/*! Status game_loop(SDL_Window* window, SDL_Renderer* renderer)
    \brief Starts a loop that renders the game screen.
    \param renderer used for all rendering inside the game.
    \return QUIT_PROGRAM or NORMAL enums.
*/
int game_loop(GLFWwindow* window);

#endif
