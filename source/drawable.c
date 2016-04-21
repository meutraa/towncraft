#include "drawable.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "constants.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

void load_drawables(SDL_Renderer* renderer, Drawable (*drawables)[], char* layout_file)
{
	FILE* file = fopen(layout_file, "r");
		
	int len = MAX_LINE_LENGTH + 1;   // +1 for the terminating null-character.
	int i = 0;
	
	char line[len], name[len], path[len];
	int wx, wy, mx, my;
	int mode, r, g, b, a, font_size;
	SDL_Surface* surface;
	
	while(NULL != fgets(line, len, file))
	{
		if(NULL != strstr(line, "images/")) 
		{
			sscanf(line, "%256[^;];%256[^;];%d;%d;%d;%d", name, path, &wx, &wy, &mx, &my);
			surface = IMG_Load(path);
		}
		else if(NULL != strstr(line, "fonts/"))
		{
			sscanf(line, "%256[^;];%256[^;];%d;%d;%d;%d;%d;%d;%d;%d;%d;%d", 
					name, path, &font_size, &mode, &r, &g, &b, &a, &wx, &wy, &mx, &my);
			TTF_Font* font = TTF_OpenFont(path, font_size);
			SDL_Color color = { r, g, b, a };
			surface = TTF_RenderText_Blended(font, name, color);
			TTF_CloseFont(font);
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
	}
	
clean:
	/* Close file. */
	if(NULL != file) fclose(file);
}
