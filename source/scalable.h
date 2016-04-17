struct Scalable create_scalable(float x, float y, int w, int h, int texture_id);

/* Contains the initial position of the scalable in terms of a percentage. */
struct Scalable 
{
    int texture_id;
    SDL_Rect rect;
    float initial_width_percentage;
    float initial_height_percentage;
};