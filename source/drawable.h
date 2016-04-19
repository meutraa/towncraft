/** @file drawable.h 
 *  @brief A structure that simplifies scaling SDL_Textures.
 */
 
#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "string.h"
#include "stdlib.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "math.h"

/** @def MAX_RESOURCES
 *  @brief the maximum number of Textures that the load_drawables function will allocate memory in one call.
 */
#define MAX_RESOURCES 128

/** @def MAX_DRAWABLES
 *  @brief the maximum number of Drawables the load_drawables function will allocate to memory in one call.
 */
#define MAX_DRAWABLES 256

/**
 * @brief A structure that contains some values for convenience 
 *
 * This is effectively a wrapper structure for a Texture which does not contain a copy
 * of a Texture but rather a pointer. It also contains SDL_Rects with positions that vary
 * for multiple aspect ratios, and a pointer to one of these which is currently in use.
 */
typedef struct Drawable 
{
	SDL_Texture* texture;   /**< a pointer to an SDL_Texture */
	SDL_Rect* rect;      	/**< an SDL_Rect pointer that defines the current position and size. */
	char* name;				/**< a unique name. */
	char* resource_path;	/**< the resource file path. */
	SDL_Rect widescreen;	/**< an SDL_Rect containing positions for a 16:9 aspect ratio. */
	SDL_Rect monitor;		/**< an SDL_Rect containing positions for a 16:10 aspect ratio. */
} Drawable;

/** @fn Pair load_drawables(SDL_Renderer* renderer, SDL_Texture*** textures, Drawable** drawables, char* layout_path)
 *  @brief Takes a layout file and fills the textures and drawables arrays.
 *
 *  This function will read a valid layout file and allocate memory for Drawables with the least number of SDL_Textures
 *  required.\n It creates drawables from a layout file with the following format:
 *
 *   path/to/resource\n
 *   600 400\n
 *   800 420\n
 *
 *	@note The first set of numbers is the X and Y co-ordinates of the top left corner of the Drawable on the Window.
 *	@note All sizes are scaled to a resolution of 1280x720, so 1280 is always the very right, and 720 is always the very bottom.
 *
 *  @param renderer the SDL_Renderer used to render textures.
 *  @param textures a pointer to an array of SDL_Texture pointers to fill. 
 *  @param drawables a pointer to an array of drawables to fill.
 *  @note It is not neccesary to allocate the memory for textures and drawables before calling this function.
 *  @param layout_path a relative path to the resource file as a string.
 *  @return A pair {a = texture_count, b = resource_count}
 *	@note the texture pointer of the returned Drawables may still be NULL and the SDL_Rects may not have initialised values if the
 *  	function failed to parse any of the layout file. 
 */
Pair load_drawables(SDL_Renderer* renderer, SDL_Texture*** textures, Drawable** drawables, char* layout_path);

Drawable create_text_drawable(SDL_Renderer* renderer, int pos_x, int pos_y, char* text, char* font_path, int font_size, SDL_Color color);
 
#endif
