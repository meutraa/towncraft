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
typedef struct Drawable {
    SDL_Texture* texture;       /*!< a pointer to an SDL_Texture */
    SDL_Rect* rect;             /*!< an SDL_Rect pointer that defines the current position and size. */
    char name[128];             /*!< a unique name. */
    unsigned int visible : 1;   /*!< 1 == rendered, 0 == not rendered. */
    SDL_Rect widescreen;        /*!< an SDL_Rect containing positions for a 16:9 aspect ratio. */
    SDL_Rect monitor;           /*!< an SDL_Rect containing positions for a 16:10 aspect ratio. */
} Drawable;

void destroy_textures(SDL_Texture** textures);

/*! \fn void destroy_drawables(Drawable drawables[])
    \brief Frees all memory contained in the Drawable.

    \param drawables a pointer to an array of drawables.
*/
void destroy_drawables(Drawable* drawables);

/*! \fn void render_drawables(SDL_Renderer* renderer, Drawable* drawables)
    \brief Copys an array of drawables to the renderer provided.

    \param renderer the SDL_Renderer used to render textures.
    \param drawables a pointer to an array of drawables.
*/
void render_drawables(SDL_Renderer* renderer, Drawable* drawables);

/*! \fn void load_drawables(SDL_Renderer* renderer, Drawable drawables[], const char* layout_file)
    \brief Takes a layout file and fills the drawables array.

    \param renderer the SDL_Renderer used to render textures.
    \param layout_file a relative path to the resource file as a string.
    \return the number of loaded drawables.
*/
Drawable* load_drawables(SDL_Renderer* renderer, const char* layout_file);

#endif
