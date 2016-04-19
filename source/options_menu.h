/** @file options_menu.h 
 *  @brief function return codes to be used program-wide.
 */

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "status.h"
#include "SDL.h"

/** Return options_menu_loop(SDL_Renderer* renderer)
 *  @brief Starts a loop that renders the options menu screen.
 *  Before calling this, all unneeded memory should be freed. Once this starts, it will load the options menu layout
 *  and start rendering the layout.
 *  @param renderer used for all rendering inside the main menu screen.
 *  @return QUIT_PROGRAM, SWITCHTO_MAINMENU or NORMAL enums.
 */
Return options_menu_loop(SDL_Renderer* renderer);

#endif
