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
