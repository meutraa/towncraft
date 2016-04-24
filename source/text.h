/*! \file text.h
	\brief text drawing related functions.
*/

#ifndef TEXT_H
#define TEXT_H

#include "SDL.h"

/*! \fn void render_text(SDL_Renderer* renderer, char text[], int size, SDL_Color color, int pos_x, int pos_y)
	\brief Render text to screen with given parameters.
	\param renderer	current SDL_Renderer.
	\param text text to render.
	\param size font size.
	\param color font color.
	\param pos_x x component of text position.
	\param pos_y y component of text position.
*/
void render_text(SDL_Renderer* renderer, char text[], int size, SDL_Color color, int pos_x, int pos_y);

#endif
