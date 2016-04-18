#include "scalable.h"

Scalable create_scalable(int pos_x, int pos_y, SDL_Texture* textures[], int texture_id)
{
	int width, height;
	SDL_QueryTexture(textures[texture_id], NULL, NULL, &width, &height);
	
	SDL_Rect rect;
	rect.w = width;
	rect.h = height;
	rect.x = pos_x;
	rect.y = pos_y;
	
	Scalable scalable;
	scalable.texture_id = texture_id;
	scalable.rect = rect;
	return scalable;
}

Drawable create_text_drawable(SDL_Renderer* renderer, int pos_x, int pos_y, char* text, char* font_path, int font_size, SDL_Color* color)
{
	TTF_Font* font = TTF_OpenFont(font_path, font_size);
	SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, color);
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, surface);
	
	SDL_Rect rect;
	rect.w = text_surface->w;
	rect.h = text_surface->h;
	rect.x = pos_x;
	rect.y = pos_y;
	
	Drawable drawable
	drawable.texture = text_texture;
	drawable.rect = rect;
	
	SDL_FreeSurface(text_surface);
	return drawable;
}

int load_textures(SDL_Renderer* renderer, SDL_Texture* textures[], char* texture_paths[], int n)
{
	int errors = 0;
	for(int i = 0; i < n; i++)
	{
		SDL_Surface* surface = IMG_Load(texture_paths[i]);
		if(NULL == surface)
		{
			errors++;
			fprintf(stderr, "\nIMG_Load failed to load \"%s\" to surface\n", texture_paths[i]);
		}
		else
		{
			textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_FreeSurface(surface);
		}
	}
	return errors;
}
