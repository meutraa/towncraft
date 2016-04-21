/*! \file math.h
	\brief A bunch of math functions.
*/

#ifndef MATH_H
#define MATH_H

#include "SDL.h"

/* To be detected as C. Can be removed once a keyword like extern/struct/typdef/enum is used. */
#ifndef C_DETECT
#define C_DETECT
typedef struct Empty { int nothing; } Empty;
#endif

/*! \fn int bounded_by(int x, int y, SDL_Rect* r)
	\note This function is made redundant in SDL 2.0.4 with SDL_PointInRect()
	\brief Checks if a 2d point is inside a rectangle (SDL_Rect)
	\param x an integer value for the horizontal position.
	\param y an integer value for the vertical position.
	\param r an SDL_Rect for bounds.
	\return 0 if point is outside bounds, 1 else.
*/
int bounded_by(int x, int y, SDL_Rect* r);

#endif
