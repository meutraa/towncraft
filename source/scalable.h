#ifndef SCALABLE_H
#define SCALABLE_H

/** @file scalable.h 
 *  @brief A structure that simplifies scaling SDL_Textures.
 */

#include "SDL.h"
#include "SDL_image.h"
#include "math.h"

/**
 * @brief A structure that contains some values for convenience 
 *
 * This is effectively a wrapper structure for a Texture which does not contain a copy
 * of a Texture but rather a reference to be used as the Developer sees fit.
 */
typedef struct Scalable 
{
	int texture_id;     /**< an integer id to be used as a texture reference. */
	SDL_Rect rect;      /**< an SDL_Rect that defines the current absolute position and size. */
	float pos_x;    /**< a float value for the initial requested horizontal position. */
	float pos_y;    /**< a float value for the initial requested vertical position. */
	int width;			/**< the width of the texture associated with texture_id. */
	int height;			/**< the height of the texture associated with texture_id. */
} Scalable;

/** @fn Scalable create_scalable(float pos_x, float pos_y, SDL_Texture* textures[], int texture_id)
 *  @brief Creates a new scalable struct.
 *  @param pos_x a float ϵ [0-1] that represents relative horizontal starting position to the window.
 *  @param pos_y a float ϵ [0-1] that represents relative vertical starting position to the window.
 *  @note A value of 0 for x will be the very left, and 1 will be the very right.\n
 *        A value of 0 for y will be the very top, and 1 will be the very bottom.
 *  @param textures an array of SDL_Textures that corresponds to the texture_id that provides dimensions.
 *  @param texture_id an integer id that should map to a defined Texture.
 *  @note It is the developer's responsibility to ensure this id maps to a Texture some how.\n
		  For example: Texture texture = textures[scalable.texture_id];
 *  @return a Scalable struct
 */
Scalable create_scalable(float pos_x, float pos_y, SDL_Texture* textures[], int texture_id);

/** @fn void resize_scalables(SDL_Window* window, Scalable scalables[], int n, float scale)
 *  @brief Takes a Scalable array with the corresponding SDL_Texture array, and resizes the scalables according to scale.
 *  @param window the window used to determine window dimensions and positions of scalables. 
 *  @param scalables a Scalable array for which the members should be scaled.
 *  @param n the number of scalables to be resized.
 *  @param scale a value to which the original size of the texture should be scaled by.
 */
void resize_scalables(SDL_Window* window, Scalable scalables[], int n, float scale);

/** @fn int load_textures(SDL_Renderer* renderer, SDL_Texture* textures[], char* texture_paths[], int n)
 *  @brief Takes a string array of file paths and loads these files as textures to the given texture array.
 *  @param renderer the SDL_Renderer used to render this texture.
 *  @param textures an SDL_Texture array to fill. 
 *  @param texture_paths a string array with relative file paths.
 *  @param n the number of textures to be loaded.
 *  @warning The textures array must have memory allocated and must be of the same length as texture_paths.
 *  @return 0 if no errors, else the number of textures that failed to load. 
 */
int load_textures(SDL_Renderer* renderer, SDL_Texture* textures[], char* texture_paths[], int n);

#endif
