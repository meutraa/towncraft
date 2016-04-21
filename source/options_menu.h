/*! \file options_menu.h
	\brief function return codes to be used program-wide.
*/

#ifndef OPTIONS_MENU_H
#define OPTIONS_MENU_H

#include "status.h"
#include "SDL.h"

/* To be detected as C. Can be removed once a keyword like extern/struct/typdef/enum is used. */
#ifndef C_DETECT
#define C_DETECT
typedef struct Empty { int nothing; } Empty;
#endif

/*! Status options_menu_loop(SDL_Renderer* renderer)
	\brief Starts a loop that renders the options menu screen.
	Before calling this, all unneeded memory should be freed. Once this starts, it will load the options menu layout
	and start rendering the layout.
	\param renderer used for all rendering inside the main menu screen.
	\return QUIT_PROGRAM, SWITCHTO_MAINMENU or NORMAL enums.
*/
Status options_menu_loop(SDL_Renderer* renderer);

#endif
