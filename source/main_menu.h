/** @file main_menu.h 
 *  @brief function return codes to be used program-wide.
 */

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "status.h"
#include "SDL.h"

/** @def DESIGN_WIDTH
 *  @brief The horizontal pixel position that all functions will treat as 100% of any other resolution.
 *  This means that if the user has a horizontal resolution of 1920, any SDL_Rects set to render at 1280, will 
 *  really render at 1920.
 */
#define DESIGN_WIDTH 1280

/** @def DESIGN_HEIGHT
 *  @brief The vertical pixel position that all functions will treat as 100% of any other resolution.
 *  This means that if the user has a vertical resolution of 2160, any SDL_Rects set to render at 720, will 
 *  really render at 2160.
 */
#define DESIGN_HEIGHT 720

/** Return main_menu_loop(SDL_Renderer* renderer)
 *  @brief Starts a loop that renders the main menu screen.
 *  Before calling this, all unneeded memory should be freed. Once this starts, it will load the main menu layout
 *  and start rendering the layout.
 *  @param renderer used for all rendering inside the main menu screen.
 *  @return QUIT_PROGRAM or NORMAL enums.
 */
Return main_menu_loop(SDL_Renderer* renderer);

#endif
