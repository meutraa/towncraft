#include "scalable.h"

struct Scalable create_scalable(float x, float y, int w, int h, int texture_id)
{
    struct Scalable scalable;
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    scalable.texture_id = texture_id;
    scalable.rect = rect;
    scalable.initial_width_percentage = x;
    scalable.initial_height_percentage = y;
    return scalable;
}