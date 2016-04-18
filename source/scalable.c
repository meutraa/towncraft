#include "scalable.h"

Scalable create_scalable(float pos_x, float pos_y, SDL_Texture* textures[], int texture_id)
{
	int w, h;
	SDL_QueryTexture(textures[texture_id], NULL, NULL, &w, &h);
	SDL_Rect rect;
	
	Scalable scalable;
	scalable.texture_id = texture_id;
	scalable.rect = rect;
	scalable.pos_x = pos_x;
	scalable.pos_y = pos_y;
	return scalable;
}

void resize_scalables(SDL_Window* window, SDL_Texture* textures[], Scalable scalables[], int n, float scale)
{
	int win_w, win_h;
	SDL_GetWindowSize(window, &win_w, &win_h);
	
	for(int i = 0; i < n; i++)
	{
		int w, h;
		SDL_QueryTexture(textures[scalables[i].texture_id], NULL, NULL, &w, &h);
		scalables[i].rect.x = scalables[i].pos_x * scale * win_w;
		scalables[i].rect.y = scalables[i].pos_y * scale * win_h;
		scalables[i].rect.w = w * scale;
		scalables[i].rect.h = h * scale;
	}
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
