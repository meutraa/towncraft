#include "drawable.h"
#include "file.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "constants.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

void render_drawables(SDL_Renderer renderer, Drawable drawables, int count)
{
	for(int i = 0; i < count; i++)
	{
		SDL_RenderCopy(renderer, drawables[i].texture, NULL, drawables[i].rect);
	}
}

int load_drawables(SDL_Renderer* renderer, Drawable (*drawables)[], char* layout_file)
{
	FILE* file = fopen(layout_file, "r");
	if(NULL == file) return 0;

	int len = MAX_LINE_LENGTH + 1;   // +1 for the terminating null-character.
	int i = 0, loaded = 0, status;

	char line[len], name[len], path[len];
	int wx, wy, mx, my;
	int mode, r, g, b, a, font_size;
	SDL_Surface* surface;

	while(NULL != fgets(line, len, file))
	{
		if(NULL != strstr(line, "images/"))
		{
			status = sscanf(line, "%256[^;];%256[^;];%d;%d;%d;%d", name, path, &wx, &wy, &mx, &my);
			if(6 != status || -1 == file_exists(path))
			{
				loaded = 0; break;
			}

			surface = IMG_Load(path);
		}
		else if(NULL != strstr(line, "fonts/"))
		{
			status = sscanf(line, "%256[^;];%256[^;];%d;%d;%d;%d;%d;%d;%d;%d;%d;%d",
					name, path, &font_size, &mode, &r, &g, &b, &a, &wx, &wy, &mx, &my);
			if(12 != status || -1 == file_exists(path))
			{
				loaded = 0; break;
			}

			TTF_Font* font = TTF_OpenFont(path, font_size);
			SDL_Color color = { r, g, b, a };
			surface = (0 == mode) ? TTF_RenderText_Solid(font, name, color) :
								  TTF_RenderText_Blended(font, name, color);
			TTF_CloseFont(font);
		}
		else if(NULL != strstr(line, ";color;"))
		{
			int width, height;
			status = sscanf(line, "%256[^;];%256[^;];%d;%d;%d;%d;%d;%d;%d;%d;%d;%d",
			name, path, &width, &height, &r, &g, &b, &a, &wx, &wy, &mx, &my);
			if(12 != status)
			{
				loaded = 0; break;
			}
			surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
			SDL_SetSurfaceColorMod(surface, r, g, b);
			SDL_SetSurfaceAlphaMod(surface, a);
			SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
		}
		else
		{
			loaded = 0;
			break;
		}

		/* Save the name. */
		(*drawables)[i].name = (char*) calloc(strlen(name) + 1, sizeof(char));
		strncpy((*drawables)[i].name, name, strlen(name));

		/* Save the texture. */
		(*drawables)[i].texture = SDL_CreateTextureFromSurface(renderer, surface);

		/* Fill the dimensions. */
		(*drawables)[i].widescreen = (SDL_Rect) { wx, wy, surface->w, surface->h };
		(*drawables)[i].monitor = (SDL_Rect) { mx, my, surface->w, surface->h };
		(*drawables)[i].rect = &(*drawables)[i].widescreen;

		SDL_FreeSurface(surface);
		i++;
		loaded++;
	}

	/* free up all resources if errors. */
	if(0 != i && 0 == loaded)
	{
		fprintf(stderr, "Line %d of file %s is invalid.\n", i + 1, layout_file);
		while(--i >= 0)
		{
			free((*drawables)[i].name);
			SDL_DestroyTexture((*drawables)[i].texture);
		}
	}

	/* Close file. */
	if(NULL != file) fclose(file);
	return loaded;
}
