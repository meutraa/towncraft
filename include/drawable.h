/*! \file drawable.h
	\brief A structure that simplifies scaling SDL_Textures.
*/

#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "SDL.h"

/*!
	\brief A structure that contains some values for convenience

	This is effectively a wrapper structure for a Texture which does not contain a copy
	of a Texture but rather a pointer. It also contains SDL_Rects with positions that vary
	for multiple aspect ratios, and a pointer to one of these which is currently in use.
*/
typedef struct Drawable
{
	SDL_Texture* texture;   /*!< a pointer to an SDL_Texture */
	SDL_Rect* rect;      	/*!< an SDL_Rect pointer that defines the current position and size. */
	char* name;				/*!< a unique name. */
	int	visible;			/*!< 1 == rendered, anything else == not rendered. */
	SDL_Rect widescreen;	/*!< an SDL_Rect containing positions for a 16:9 aspect ratio. */
	SDL_Rect monitor;		/*!< an SDL_Rect containing positions for a 16:10 aspect ratio. */
} Drawable;

/*! \fn int bounded_by(int x, int y, SDL_Rect* r)
	\note This function is made redundant in SDL 2.0.4 with SDL_PointInRect()
	\brief Checks if a 2d point is inside a rectangle (SDL_Rect)
	\param x an integer value for the horizontal position.
	\param y an integer value for the vertical position.
	\param r an SDL_Rect for bounds.
	\return 0 if point is outside bounds, 1 else.
*/
int bounded_by(int x, int y, SDL_Rect* r);

/*! \fn void destroy_drawables(Drawable** drawables, int count)
	\brief Frees all memory contained in the Drawable.

	\param drawables a pointer to an array of drawables.
	\param count the number of drawables to free.
*/
void destroy_drawables(Drawable** drawables, int count);

/*! \fn show_drawables(Drawable** drawables, int count, int show)
	\brief Sets the visible property of all the drawables to 0 or 1

	\param drawables a pointer to an array of drawables.
	\param count the number of drawables.
	\param show 1 for visible, anything else will become 0.
*/
void show_drawables(Drawable** drawables, int count, int show);

/*! \fn void render_drawables(SDL_Renderer* renderer, Drawable* drawables, int count)
	\brief Copys an array of drawables to the renderer provided.

	\param renderer the SDL_Renderer used to render textures.
	\param drawables a pointer to an array of drawables.
	\param count the number of drawables to copy.
*/
void render_drawables(SDL_Renderer* renderer, Drawable* drawables, int count);

/*! \fn void load_drawables(SDL_Renderer* renderer, Drawable** drawables, const char* layout_file)
	\brief Takes a layout file and fills the drawables array.

	\param renderer the SDL_Renderer used to render textures.
	\param drawables a pointer to an array of drawables to fill.
	\param layout_file a relative path to the resource file as a string.
	\return the number of loaded drawables, 0 if error.
*/
int load_drawables(SDL_Renderer* renderer, Drawable** drawables, const char* layout_file);

#endif
