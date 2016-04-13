#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    int init_return = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    if(init_return != 0)
    {
        fprintf(stderr, "\nUnable to initialize SDL:  %s\n", SDL_GetError());
        return 1;
    }
    atexit(SDL_Quit);

    return 0;
}