/*! \file game.h
	\brief Access to the game loop.
*/

#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "status.h"

/* This next line tricks gitlab-linguist into thinking this is C. */
//typedef struct

/*! Status game_loop(SDL_Renderer* renderer)
	\brief Starts a loop that renders the game screen.
	\param renderer used for all rendering inside the game.
	\return QUIT_PROGRAM or NORMAL enums.
*/
Status game_loop(SDL_Renderer* renderer);

#endif
