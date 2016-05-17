/*! \file game.h
    \brief Access to the game loop.
*/

#ifndef GAME_H
#define GAME_H

#include <GLFW/glfw3.h>

/*! Status game_loop(SDL_Window* window, SDL_Renderer* renderer)
    \brief Starts a loop that renders the game screen.
    \param renderer used for all rendering inside the game.
    \return QUIT_PROGRAM or NORMAL enums.
*/
int game_loop(GLFWwindow* window);

#endif
