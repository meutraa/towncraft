/** @file texture.h 
 *  @brief A structure that adds the dimensions of a SDL_Texture with one.
 */

#include "SDL.h"

/**
 * @brief An SDL_Texture wrapper that contains dimensions. 
 *
 * Because an SDL_Surface contains members width and height and an SDL_Texture
 * requires a function call to get these values, this saves calling a function
 * each time.
 */
struct Texture 
{
    SDL_Texture *texture;   /**< an SDL_Texture. */
    int width;              /**< the width of the SDL_Texture. */
    int height;             /**< the height of the SDL_Texture. */
}; 