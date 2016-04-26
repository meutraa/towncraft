/*! \file main_menu.h
    \brief Access to the main_menu loop.
*/

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "SDL.h"
#include "status.h"

/*! Status main_menu_loop(SDL_Renderer* renderer)
    \brief Starts a loop that renders the main menu screen.
    Before calling this, all unneeded memory should be freed. Once this starts, it will load the main menu layout
    and start rendering the layout.
    \param renderer used for all rendering inside the main menu screen.
    \return QUIT_PROGRAM or NORMAL enums.
*/
Status main_menu_loop(SDL_Renderer* renderer);

#endif
