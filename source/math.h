#ifndef MATH_H
#define MATH_H

/** @file math.h 
 *  @brief A bunch of math functions.
 */
#include "SDL.h"

/** @def LENGTH(x)
 *  @brief macro to get the length of an array.
 *  @param x the array to be measured.
 *  @warning Do not use this macro with an array pointer, as this will return the size of the pointer.
 *			 This means it is impossible to find the length of an array that is a function parameter.
 *	@return the length of an array as an unsigned long integer
 */
#define LENGTH(x)  (sizeof(x)/sizeof((x)[0]))

/** @def MIN(a, b)
 *  @brief get the lower value of a pair.
 *  @param a the first value.
 *  @param b the second value.
 *	@return the lower value.
 */
#define MIN(a, b) (((a)<(b))?(a):(b))

/** @def MAX(a, b)
 *  @brief get the higher value of a pair.
 *  @param a the first value.
 *  @param b the second value.
 *	@return the higher value.
 */
#define MAX(a, b) (((a)>(b))?(a):(b))

/** @fn int bounded_by(int x, int y, SDL_Rect r)
 *  @brief Checks if a 2d point is inside a rectangle (SDL_Rect)
 *  @param x an integer value for the horizontal position.
 *  @param y an integer value for the vertical position.
 *  @param r an SDL_Rect for bounds.
 *  @return 0 if point is outside bounds, 1 else.
 */
int bounded_by(int x, int y, SDL_Rect* r);

#endif
