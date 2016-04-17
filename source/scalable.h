/** @file scalable.h 
 *  @brief A structure that simplifies scaling SDL_Textures.
 */

#include "SDL.h"

/** @fn create_scalable(float x, float y, int w, int h, int texture_id)
 *  @brief Creates a new scalable struct.
 *  @param x a float ϵ [0-1] that represents relative horizontal starting position to the window.
 *  @param y a float ϵ [0-1] that represents relative vertical starting position to the window.
 *  @note A value of 0 for x will be the very left, and 1 will be the very right.\n
 *        A value of 0 for y will be the very top, and 1 will be the very bottom.
 *  @param w an integer value for the width of the texture.
 *  @param h an integer value for the height of the texture.
 *  @param texture_id an integer id that should map to a defined Texture.
 *  @note It is the developer's responsibility to ensure this id maps to a Texture some how.\n
          For example: Texture texture = textures[scalable.texture_id];
 *  @return a Scalable struct
 */
struct Scalable create_scalable(float x, float y, int w, int h, int texture_id);

/**
 * @brief A structure that contains some values for convenience 
 *
 * This is effectively a wrapper structure for a Texture which does not contain a copy
 * of a Texture but rather a reference to be used as the Developer sees fit.
 */
struct Scalable 
{
    int texture_id;     /**< an integer id to be used as a texture reference. */
    SDL_Rect rect;     /**< an SDL_Rect that defines the current absolute position and size. */
    float initial_width_percentage;     /**< a float value for the initial requested horizontal position. */
    float initial_height_percentage;    /**< a float value for the initial requested vertical position. */
};