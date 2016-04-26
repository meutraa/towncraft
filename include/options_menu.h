/*! \file options_menu.h
        \brief Access to the options_menu loop.
*/

#ifndef OPTIONS_MENU_H
#define OPTIONS_MENU_H

#include "SDL.h"
#include "status.h"

/*! Status options_menu_loop(SDL_Renderer* renderer)
    \brief Starts a loop that renders the options menu screen.
    Before calling this, all unneeded memory should be freed. Once this
    starts, it will load the options menu layout
    and start rendering the layout.
    \param renderer used for all rendering inside the main menu screen.
    \return QUIT_PROGRAM, SWITCHTO_MAINMENU or NORMAL enums.
*/
Status options_menu_loop(SDL_Renderer* renderer);

#endif
