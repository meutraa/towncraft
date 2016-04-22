#include "drawable.h"
#include "file.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "constants.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#define MAX_DRAWABLES 32

/* font placed at (x,y), then (x - 15, y - 5), width(int n) = 24*7 + 30 , height(32) = 45 */

#define TEXT_FORMAT  "%d%*[ \t]text%*[^\"]\"%256[^\"]\"%*[ \t]%256[^:]:%d:%d%*[^(](%d,%d,%d,%d)%*[^(](%d,%d)%*[^(](%d,%d)"
#define IMG_FORMAT   "%d%*[ \t]image%*[^\"]\"%256[^\"]\"%*[ \t]%256[^ \t]%*[^(](%d,%d)%*[^(](%d,%d)"
#define COLOR_FORMAT "%d%*[ \t]color%*[^\"]\"%256[^\"]\"%*[^(](%d,%d)%*[^(](%d,%d,%d,%d)%*[^(](%d,%d)%*[^(](%d,%d)"

static int count_params(char* format)
{
	int i = 0, count = 0;
	while(format[i + 1] != '\0')
	{
		if(format[i] == '%' && format[i + 1] != '*') count++;
		i++;
	}
	return count;
}

static int TEXT_COUNT, IMG_COUNT, COLOR_COUNT;

void destroy_drawables(Drawable** drawables, int count)
{
	for(int i = 0; i < count; i++)
	{
		free((*drawables)[i].name);
		SDL_DestroyTexture((*drawables)[i].texture);
	}
	free((*drawables));
}

void show_drawables(Drawable** drawables, int count, int show)
{
	for(int i = 0; i < count; i++)
	{
		(*drawables)[i].visible = (1 == show) ? 1 : 0;
	}
}

void render_drawables(SDL_Renderer* renderer, Drawable* drawables, int count)
{
	for(int i = 0; i < count; i++)
	{
		if(1 == drawables[i].visible)
		{
			SDL_RenderCopy(renderer, drawables[i].texture, NULL, drawables[i].rect);
		}
	}
}

int load_drawables(SDL_Renderer* renderer, Drawable** drawables, char* layout_file)
{
	/* Count the correct number of parameters in the format strings. */
	if(0 == TEXT_COUNT)
	{
		TEXT_COUNT  = count_params(TEXT_FORMAT);
		IMG_COUNT   = count_params(IMG_FORMAT);
		COLOR_COUNT = count_params(COLOR_FORMAT);
	}
	FILE* file = fopen(layout_file, "r");
	if(NULL == file)
	{
		fprintf(stderr, "%s: file not readable\n\n", layout_file);
		return 0;
	}

	*drawables = malloc(MAX_DRAWABLES*sizeof(Drawable));
	if(NULL == drawables)
	{
		fprintf(stderr, "%s: could not assign memory for drawables.\n\n", layout_file);
		fclose(file);
		return 0;
	}

	int len = MAX_LINE_LENGTH + 1;   // +1 for the terminating null-character.
	int i = 0;

	char line[len], name[len], path[len];
	int wx, wy, mx, my, width, height, visible;
	int mode, r, g, b, a, font_size;
	SDL_Surface* surface;

	while(NULL != fgets(line, len, file) && i < MAX_DRAWABLES)
	{
		if(IMG_COUNT == sscanf(line, IMG_FORMAT,
		&visible, name, path, &wx, &wy, &mx, &my))
		{
			if(-1 == file_exists(path))
			{
				fprintf(stderr, "%s:%d:%s\n^^ file not readable ^^\n\n", layout_file, i, line);
				continue;
			}

			surface = IMG_Load(path);
		}
		else if(TEXT_COUNT == sscanf(line, TEXT_FORMAT,
		&visible, name, path, &font_size, &mode, &r, &g, &b, &a, &wx, &wy, &mx, &my))
		{
			if(-1 == file_exists(path))
			{
				fprintf(stderr, "%s:%d:%s\n^^ file not readable ^^\n\n", layout_file, i, line);
				continue;
			}

			TTF_Font* font = TTF_OpenFont(path, font_size);
			SDL_Color color = { r, g, b, a };
			surface = (0 == mode) ? TTF_RenderText_Solid(font, name, color):
								  TTF_RenderText_Blended(font, name, color);
			TTF_CloseFont(font);
		}
		else if(COLOR_COUNT == sscanf(line, COLOR_FORMAT,
		&visible, name, &width, &height, &r, &g, &b, &a, &wx, &wy, &mx, &my))
		{
			surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
			SDL_SetSurfaceColorMod(surface, r, g, b);
			SDL_SetSurfaceAlphaMod(surface, a);
			SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
		}
		else
		{
			fprintf(stderr, "%s:%d:%s^^ unrecognised layout string ^^\n\n", layout_file, i, line);
			continue;
		}

		/* Save the name. */
		(*drawables)[i].name = (char*) calloc(strlen(name) + 1, sizeof(char));
		strncpy((*drawables)[i].name, name, strlen(name));

		/* Save the visibility. */
		(*drawables)[i].visible = (1 == visible) ? 1 : 0;

		/* Save the texture. */
		(*drawables)[i].texture = SDL_CreateTextureFromSurface(renderer, surface);

		/* Fill the dimensions. */
		(*drawables)[i].widescreen = (SDL_Rect) { wx, wy, surface->w, surface->h };
		(*drawables)[i].monitor = (SDL_Rect) { mx, my, surface->w, surface->h };
		(*drawables)[i].rect = &(*drawables)[i].widescreen;

		SDL_FreeSurface(surface);
		i++;
	}

	if(MAX_DRAWABLES == i)
	{
		fprintf(stderr, "%s: MAX_DRAWABLES(%d) limit exceed.\n", layout_file, MAX_DRAWABLES);
	}

	/* Shrink the array to the size we used. */
	Drawable** new = malloc(i*sizeof(Drawable));
	if(NULL == new)
	{
		fprintf(stderr, "%s: could not reassign memory for drawables.\n\n", layout_file);
		destroy_drawables(drawables, i);
		i = 0;
	}
	else
	{
		memmove(new, drawables, i*sizeof(Drawable));
	}
	drawables = new;

	/* Close file. */
	fclose(file);
	return i;
}
