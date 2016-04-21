#include "SDL.h"

int bounded_by(int x, int y, SDL_Rect* r)
{
	if((x > r->x && x < r->x + r->w) && (y > r->y && y < r->y + r->h))
		return 1;
	return 0;
}
