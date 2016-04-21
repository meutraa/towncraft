/*! \file main_menu.h
	\brief function return codes to be used program-wide.
*/

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "status.h"
#include "SDL.h"

/* This next line tricks gitlab-linguist into thinking this is C. */
//typedef struct

/*! Status main_menu_loop(SDL_Renderer* renderer)
	\brief Starts a loop that renders the main menu screen.
	Before calling this, all unneeded memory should be freed. Once this starts, it will load the main menu layout
	and start rendering the layout.
	\param renderer used for all rendering inside the main menu screen.
	\return QUIT_PROGRAM or NORMAL enums.
*/
Status main_menu_loop(SDL_Renderer* renderer);

#endif
