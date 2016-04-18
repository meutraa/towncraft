#include "scalable.h"

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
	drawable.rect = &rect;
	
	SDL_FreeSurface(text_surface);
	return drawable;
}

int load_drawables(SDL_Renderer* renderer, SDL_Texture*** textures, Drawable** drawables, char* layout_file)
{
	FILE* file = fopen(layout_file, "r");
	if(NULL == file) return -1;
	
	/* This is an array containing unique resource file paths. */
	char* resources[MAX_RESOURCES];
	Drawable drawables_tmp[MAX_DRAWABLES];
	
	char buf[128 + 1];   // +1 for the terminating null-character.
	/* The number of unique resource paths found. */
	int texture_count = 0;
	int drawable_count = 0;
	 
	/* Loop to obtain sizes and values. */
	while(NULL != fgets(buf, 128 + 1, file))
	{
		if(NULL != strstr(buf, "resources/"))
		{
			int found = 0;
			for(int i = 0; i < MAX_RESOURCES && i < texture_count; i++)
			{
				if(NULL != strstr(buf, resources[i])) // buf has a appending '\n'
				{
					found = 1;
					break;
				}
			}
			if(0 == found)
			{
				int len = strlen(buf);
				resources[texture_count] = calloc(len + 1, sizeof(char));
				strncpy(resources[texture_count], buf, len);
				texture_count++;
				resources[texture_count - 1][len - 1] = '\0'; // len - 1 to remove \n
			}
			
			/* Save the resource path. */
			int len = strlen(buf);
			drawables_tmp[drawable_count].resource_path = calloc(len + 1, sizeof(char));
			strncpy(drawables_tmp[drawable_count].resource_path, buf, len);
			drawables_tmp[drawable_count].resource_path[len - 1] = '\0';
			
			/* Read the next line and save the widescreen values. */
			char* c;
			fgets(buf, 128 + 1, file);
			c = strtok(buf, " ");
			SDL_Rect widescreen;
			widescreen.x = atoi(c);
			c = strtok(NULL, " ");
			widescreen.y = atoi(c);
			drawables_tmp[drawable_count].widescreen = widescreen;
			
			/* Read the last line and save the monitor values. */
			fgets(buf, 128 + 1, file);
			c = strtok(buf, " ");
			SDL_Rect monitor;
			monitor.x = atoi(c);
			c = strtok(NULL, " ");
			monitor.y = atoi(c);
			drawables_tmp[drawable_count].monitor = monitor;
						
			drawable_count++;
		}		
	}
	//printf("Total unique resources loaded: %d\n", texture_count);
	if(NULL != file) fclose(file);
	
	*textures = (SDL_Texture**) calloc(texture_count, sizeof(SDL_Texture*));
	*drawables = (Drawable*) calloc(drawable_count, sizeof(Drawable));
	memcpy(*drawables, drawables_tmp, sizeof(Drawable)*drawable_count);
	
	/* Debugging. */
	/*for(int i = 0; i < drawable_count; i++)
	{
		printf("%s\t%d,%d\t%d,%d\n", drawables[i].resource_path, drawables[i].widescreen.x, drawables[i].widescreen.y,
			drawables[i].monitor.x, drawables[i].monitor.y);
	}*/
	
	/* Load textures into array. */
	for(int i = 0; i < texture_count; i++)
	{
		SDL_Surface* surface = IMG_Load(resources[i]);
		if(NULL == surface)
		{
			fprintf(stderr, "\nIMG_Load failed to load \"%s\" to surface\n", resources[i]);
		}
		else
		{
			(*textures)[i] = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_FreeSurface(surface);
		}
	}
	
	for(int i = 0; i < drawable_count; i++)
	{
		for(int j = 0; j < texture_count; i++)
		{
			/* If the resource path of the drawable matches this mapped texture id. */
			if(!strcmp((*drawables)[i].resource_path, resources[j]))
			{
				(*drawables)[i].texture = (*textures)[j];
				
				/* Fill the dimensions. */
				int w, h;
				SDL_QueryTexture((*textures)[j], NULL, NULL, &w, &h);
				(*drawables)[i].widescreen.w = w;
				(*drawables)[i].widescreen.h = h;
				(*drawables)[i].monitor.w = w;
				(*drawables)[i].monitor.h = h;
				(*drawables)[i].rect = &(*drawables)[i].widescreen;
				break;
			}
		}
	}
	return drawable_count;
}
