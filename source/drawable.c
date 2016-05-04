#include "drawable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL_image.h"
#include "SDL_ttf.h"
#include "constants.h"
#include "file.h"

/* font placed at (x,y), then (x - 15, y - 5), width(int n) = 24*7 + 30 ,
 * height(32) = 45 */

#define TEXT_FORMAT \
    "%d%*[ \t]text%*[^\"]\"%256[^\"]\"%*[ \t]%256[^:]:%d:%d%*[^(](%hhu,%hhu,%hhu,%hhu)%*[^(](%d,%d)%*[^(](%d,%d)"
#define IMG_FORMAT \
    "%d%*[ \t]image%*[^\"]\"%256[^\"]\"%*[ \t]%256[^ \t]%*[^(](%d,%d)%*[^(](%d,%d)"
#define COLOR_FORMAT \
    "%d%*[ \t]color%*[^\"]\"%256[^\"]\"%*[^(](%d,%d)%*[^(](%hhu,%hhu,%hhu,%hhu)%*[^(](%d,%d)%*[^(](%d,%d)"

static int TEXT_COUNT = 13, IMG_COUNT = 7, COLOR_COUNT = 12;

void destroy_drawables(Drawable* drawables)
{
    for (int i = 0; (drawables + i)->texture; i++)
    {
        SDL_DestroyTexture((drawables + i)->texture);
    }
    free(drawables);
}

void destroy_textures(SDL_Texture** textures)
{
    for(int i = 0; textures[i]; SDL_DestroyTexture(textures[i++]));
    free(textures);
}

void render_drawables(SDL_Renderer* renderer, Drawable* drawables, int scale)
{
    for(int i = 0; (drawables + i)->texture; i++)
    {
        Drawable* d = (drawables + i);
        if(d->visible)
        {
            SDL_Rect scaled_rect = {d->rect->x * scale, d->rect->y * scale, d->rect->w * scale, d->rect->h * scale};
            SDL_RenderCopy(renderer, d->texture, NULL, &scaled_rect);
        }
    }
}

static Drawable* load_drawables_in(SDL_Renderer* renderer, const char* layout_file, int* count)
{
    /* If we do not have the number of drawables, get it. */
    int length = 0;
    if(!count) load_drawables_in(NULL, layout_file, &length);
    FILE* file = fopen(layout_file, "r");
    if (!file)
    {
        fprintf(stderr, "%s: file not readable\n\n", layout_file);
        return NULL;
    }

    char line[256], name[128], path[128];
    int wx, wy, mx, my, width, height, visible;
    int mode, font_size;
    unsigned char r, g, b, a;
    SDL_Surface* surface;
    Drawable* drawables = malloc(sizeof(Drawable) * (unsigned long)(length + 1));
    (drawables + length)->texture = NULL;     // Null terminate the memory block.

    for (int i = 0; fgets(line, 256, file);)
    {
        if (IMG_COUNT == sscanf(line, IMG_FORMAT, &visible, name, path, &wx, &wy, &mx, &my))
        {
            if (-1 == file_exists(path))
            {
                fprintf(stderr, "%s:%d:%s\n^^ file not readable ^^\n\n", layout_file, i, line);
                continue;
            }
            if (!count) surface = IMG_Load(path);
        }
        else if (TEXT_COUNT == sscanf(line, TEXT_FORMAT, &visible, name, path, &font_size, &mode, &r, &g, &b, &a, &wx, &wy, &mx, &my))
        {
            if (-1 == file_exists(path))
            {
                fprintf(stderr, "%s:%d:%s\n^^ file not readable ^^\n\n", layout_file, i, line);
                continue;
            }
            if (!count)
            {
                TTF_Font* font = TTF_OpenFont(path, font_size);
                SDL_Color color = { r, g, b, a };
                surface = (0 == mode) ? TTF_RenderText_Solid(font, name, color) : TTF_RenderText_Blended(font, name, color);
                TTF_CloseFont(font);
            }
        }
        else if (COLOR_COUNT == sscanf(line, COLOR_FORMAT, &visible, name, &width, &height, &r, &g, &b, &a, &wx, &wy, &mx, &my))
        {
            if (!count)
            {
                surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
                SDL_SetSurfaceColorMod(surface, r, g, b);
                SDL_SetSurfaceAlphaMod(surface, a);
                SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
            }
        }
        else
        {
            if (strlen(line) >= 2 && 0 != strncmp(line, "/*", 2))
            {
                fprintf(stderr, "%s:%d:%s^^ unrecognised layout string ^^\n\n", layout_file, i, line);
            }
            continue;
        }

        if(count) (*count)++;
        else
        {
            /* Assign new values. */
            unsigned long l = strlen(name);
            strncpy((drawables + i)->name, name, l);
            (drawables + i)->name[l] = '\0';
            (drawables + i)->visible = (1 == visible) ? 1 : 0;
            (drawables + i)->widescreen = (SDL_Rect){  wx, wy, surface->w, surface->h };
            (drawables + i)->monitor = (SDL_Rect){ mx, my, surface->w, surface->h };

            /* Assign pointers new pointees. */
            (drawables + i)->texture = SDL_CreateTextureFromSurface(renderer, surface);
            (drawables + i)->rect = &drawables[i].widescreen;

            SDL_FreeSurface(surface);
        }
        i++;
    }

    fclose(file);
    return drawables;
}

Drawable* load_drawables(SDL_Renderer* renderer, const char* layout_file)
{
    return load_drawables_in(renderer, layout_file, NULL);
}
