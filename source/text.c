#include "text.h"
#include "SDL_ttf.h"

void render_text(SDL_Renderer* renderer, char text[], int size, SDL_Color color, int pos_x, int pos_y)
{
	TTF_Font* font = TTF_OpenFont("resources/fonts/fleftex_mono_8.ttf", size);
	SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
	TTF_CloseFont(font);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect rect = {pos_x, pos_y, surface->w, surface->h};
	SDL_FreeSurface(surface);
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
}
