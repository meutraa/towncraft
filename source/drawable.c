#include "constants.h"
#include "drawable.h"

#define NEXT_INT(a) atoi(strtok(NULL, a))

Drawable create_text_drawable(SDL_Renderer* renderer, int pos_x, int pos_y, char* text, char* font_path, int font_size, SDL_Color color)
{
	TTF_Font* font = TTF_OpenFont(font_path, font_size);
	SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, color);
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
	
	SDL_Rect rect;
	rect.w = text_surface->w;
	rect.h = text_surface->h;
	rect.x = pos_x;
	rect.y = pos_y;
	
	Drawable drawable;
	drawable.texture = text_texture;
	drawable.widescreen = rect;
	drawable.rect = &drawable.widescreen;
	
	SDL_FreeSurface(text_surface);
	return drawable;
}

int load_drawables_unchecked(SDL_Renderer* renderer, SDL_Texture* (*textures)[], 
							int texture_count, Drawable (*drawables)[], int drawable_count, char* layout_file)
{
	FILE* file = fopen(layout_file, "r");
	
	char* resources[texture_count];
	int current_texture_index = 0;
	
	char lines[5][MAX_LINE_LENGTH + 1];   // +1 for the terminating null-character.
	
	for(int i = 0; i < drawable_count; i++)
	{		
		/* Read the next five lines and trim the new lines. */
		for(int j = 0; j < 5; j++)
		{
			fgets(lines[j], MAX_LINE_LENGTH + 1, file);
		}
		
		/* Name of resource. */
		lines[0][strlen(lines[0]) - 1] = '\0';
		(*drawables)[i].name = (char*) calloc(strlen(lines[0]) + 1, sizeof(char));
		strncpy((*drawables)[i].name, lines[0], strlen(lines[0]));
		
		/* Resource path. */
		if(NULL != strstr(lines[1], "fonts/"))
		{
			TTF_Font* font = TTF_OpenFont(strtok(lines[1], " "), NEXT_INT(" "));
			SDL_Color color = {NEXT_INT(" "), NEXT_INT(" "), NEXT_INT(" "), NEXT_INT(" \n")};
			SDL_Surface* surface = TTF_RenderText_Solid(font, (*drawables)[i].name, color);
			(*textures)[i] = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_FreeSurface(surface);
			TTF_CloseFont(font);
		}
		else if(NULL != strstr(lines[1], "images/"))
		{
			int recycle = 0;
			for(int j = 0; j < texture_count && j < current_texture_index; j++)
			{
				/* If we already have this resource loaded as a texture. */
				if(0 == strncmp(lines[1], resources[j], strlen(lines[1])))
				{
					recycle = 1;
					(*drawables)[i].texture = (*textures)[j];
					break;
				}
			}
			
			/* If this is a new resource path. */
			if(0 == recycle)
			{
				int p = current_texture_index;
				resources[p] = lines[1];
				resources[p][strlen(resources[p]) - 1] = '\0';
				SDL_Surface* surface = IMG_Load(lines[1]);
				(*textures)[p] = SDL_CreateTextureFromSurface(renderer, surface);
				(*drawables)[i].texture = (*textures)[p];
				SDL_FreeSurface(surface);
				current_texture_index++;
			}
		}
		
		/* Widescreen rect. */
		SDL_Rect widescreen, monitor;
		widescreen.x = atoi(strtok(lines[2], " "));
		widescreen.y = atoi(strtok(NULL, " \n"));
		monitor.x = atoi(strtok(lines[3], " "));
		monitor.x = atoi(strtok(NULL, " \n"));
		
		/* Fill the dimensions. */
		int w, h;
		SDL_QueryTexture((*drawables)[i].texture, NULL, NULL, &w, &h);
		widescreen.w = w;
		widescreen.h = h;
		monitor.w = w;
		monitor.h = h;
		(*drawables)[i].widescreen = widescreen;
		(*drawables)[i].monitor = monitor;
		(*drawables)[i].rect = &(*drawables)[i].widescreen;
	}
	return 0;
}
