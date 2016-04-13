#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    /* Ensures any return will call SDL_Quit first. */
    atexit(SDL_Quit);
    
    /* Initialise the video subsystem. */
    int iret = SDL_Init(SDL_INIT_VIDEO);
    if(iret != 0)
    {
        fprintf(stderr, "\nUnable to initialize SDL Video Subsystem:  %s\n", SDL_GetError());
        return 1;
    }
    
    /* Initialise the timer subsystem. */
    iret = SDL_InitSubSystem(SDL_INIT_TIMER);
    if(iret != 0)
    {
        fprintf(stderr, "\nUnable to initialize SDL Timer Subsystem:  %s\n", SDL_GetError());
        return 1;
    }
    
    /* Initialise the audio subsystem. */
    iret = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if(iret != 0)
    {
        fprintf(stderr, "\nUnable to initialize SDL Audio Subsystem:  %s\n", SDL_GetError());
        // Do not quit if audio fails
    }
    
    /* Create window */
    SDL_Window* window = SDL_CreateWindow(
        "A Window",                 // Window title
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, // Position (x,y)
        640, 480,                   // Size (x,y)
        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE
        // Window flags http://wiki.libsdl.org/SDL_WindowFlags
    );
    
    if(window == NULL)
    {
        fprintf(stderr, "\nCould not create window: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Delay(3000);  // Pause execution for 3000 milliseconds, for example
   
    /* Clean up stuff. */
    SDL_DestroyWindow(window);
    
    return 0;
}