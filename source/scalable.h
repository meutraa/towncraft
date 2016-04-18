#ifndef SCALABLE_H
#define SCALABLE_H

/** @file scalable.h 
 *  @brief A structure that simplifies scaling SDL_Textures.
 */

#include "string.h"
#include "stdlib.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "math.h"

#define DESIGN_WIDTH 1280
#define DESIGN_HEIGHT 720

#define MAX_RESOURCES 128
#define MAX_LINE_LENGTH 128
#define MAX_DRAWABLES 256

/**
 * @brief A structure that contains some values for convenience 
 *
 * This is effectively a wrapper structure for a Texture which does not contain a copy
 * of a Texture but rather a reference to be used as the Developer sees fit.
 */
typedef struct Drawable 
{
	SDL_Texture* texture;   /**< an integer id to be used as a texture reference. */
	SDL_Rect* rect;      	/**< an SDL_Rect that defines the current absolute position and size. */
	char* resource_path;
	SDL_Rect widescreen;
	SDL_Rect monitor;
} Drawable;

/** @fn int load_textures(SDL_Renderer* renderer, SDL_Texture* textures[], char* texture_paths[], int n)
 *  @brief Takes a string array of file paths and loads these files as textures to the given texture array.
 *  @param renderer the SDL_Renderer used to render this texture.
 *  @param textures an SDL_Texture array to fill. 
 *  @param texture_paths a string array with relative file paths.
 *  @param n the number of textures to be loaded.
 *  @warning The textures array must have memory allocated and must be of the same length as texture_paths.
 *  @return 0 if no errors, else the number of textures that failed to load. 
 */
int load_drawables(SDL_Renderer* renderer, SDL_Texture*** textures, Drawable** drawables, char* layout_path);

Drawable create_text_drawable(SDL_Renderer* renderer, int pos_x, int pos_y, char* text, char* font_path, int font_size, SDL_Color color);
 
#endif
