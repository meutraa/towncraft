/** @file math.h 
 *  @brief A bunch of math functions.
 */
#include "SDL.h"

/** @fn bounded_by(int x, int y, SDL_Rect r)
 *  @brief Checks if a 2d point is inside a rectangle (SDL_Rect)
 *  @param x an integer value for the horizontal position.
 *  @param y an integer value for the vertical position.
 *  @param r an SDL_Rect for bounds.
 *  @return 0 if point is outside bounds, 1 else.
 */
int bounded_by(int x, int y, SDL_Rect r);