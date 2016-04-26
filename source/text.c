#include "text.h"

#include "SDL_ttf.h"

void render_text(SDL_Renderer* renderer, TTF_Font* font, char text[], SDL_Color color, int pos_x, int pos_y)
{
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = { pos_x, pos_y, surface->w, surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
}
