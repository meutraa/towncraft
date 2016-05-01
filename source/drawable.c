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

int bounded_by(int x, int y, SDL_Rect* r)
{
    if ((x > r->x && x < r->x + r->w) && (y > r->y && y < r->y + r->h))
    {
        return 1;
    }
    return 0;
}

void destroy_drawables(Drawable* drawables)
{
    for (int i = 0; NULL != (drawables + i)->texture; i++)
    {
        SDL_DestroyTexture((drawables + i)->texture);
    }
    free(drawables);
}

void render_drawables(SDL_Renderer* renderer, Drawable* drawables)
{
    for (int i = 0; NULL != (drawables + i)->texture; i++)
    {
        if ((drawables + i)->visible)
        {
            SDL_RenderCopy(renderer, (drawables + i)->texture, NULL, (drawables + i)->rect);
        }
    }
}

static Drawable* load_drawables_in(SDL_Renderer* renderer, const char* layout_file, int* count)
{
    /* If we do not have the number of drawables, get it. */
    int length = 0;
    if(NULL == count)
    {
        load_drawables_in(NULL, layout_file, &length);
    }

    FILE* file = fopen(layout_file, "r");
    if (NULL == file)
    {
        fprintf(stderr, "%s: file not readable\n\n", layout_file);
        return NULL;
    }

    int len = MAX_LINE_LENGTH + 1; // +1 for the terminating null-character.
    int i = 0;

    char line[len], name[len], path[len];
    int wx, wy, mx, my, width, height, visible;
    int mode, font_size;
    unsigned char r, g, b, a;
    SDL_Surface* surface;
    Drawable* drawables;

    drawables = malloc(sizeof(Drawable) * (length + 1));
    (drawables + length)->texture = NULL;     // Null terminate the memory block.

    while (fgets(line, len, file))
    {
        if (IMG_COUNT == sscanf(line, IMG_FORMAT, &visible, name, path, &wx, &wy, &mx, &my))
        {
            if (-1 == file_exists(path))
            {
                fprintf(stderr, "%s:%d:%s\n^^ file not readable ^^\n\n", layout_file, i, line);
                continue;
            }
            if (!count)
            {
                surface = IMG_Load(path);
            }
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

        if (!count)
        {
            /* Assign new values. */
            strncpy((drawables + i)->name, name, strlen(name));
            (drawables + i)->visible = (1 == visible) ? 1 : 0;
            (drawables + i)->widescreen = (SDL_Rect){  wx, wy, surface->w, surface->h };
            (drawables + i)->monitor = (SDL_Rect){ mx, my, surface->w, surface->h };

            /* Assign pointers new pointees. */
            (drawables + i)->texture = SDL_CreateTextureFromSurface(renderer, surface);
            (drawables + i)->rect = &drawables[i].widescreen;

            SDL_FreeSurface(surface);
        }
        i++;
        if(count)
        {
            (*count)++;
        }
    }

    fclose(file);
    return drawables;
}

Drawable* load_drawables(SDL_Renderer* renderer, const char* layout_file)
{
    return load_drawables_in(renderer, layout_file, NULL);
}
