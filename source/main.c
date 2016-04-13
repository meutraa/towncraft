#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>

const char* GAME_NAME = "Towncraft";

int main(int argc, char** argv)
{
    int vsync = 1;
    int initial_width = 640;
    int initial_height = 480;
    
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
        GAME_NAME,                 // Window title
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, // Position (x,y)
        initial_width, initial_height,                   // Size (x,y)
        SDL_WINDOW_RESIZABLE
        // Window flags http://wiki.libsdl.org/SDL_WindowFlags
    );
    
    if(NULL == window)
    {
        fprintf(stderr, "\nCould not create window: %s\n", SDL_GetError());
        return 1;
    }
    
    /* Get the renderer associated with the SDL_Window. 
        http://wiki.libsdl.org/SDL_RendererFlags    */
    int flags = (vsync ? SDL_RENDERER_PRESENTVSYNC : 0)|SDL_RENDERER_ACCELERATED;
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, flags);
    
    if(NULL == renderer)
    {
        fprintf(stderr, "\nCould not create renderer: %s\n", SDL_GetError());
        return 1;
    }
    
    /* Set renderer colour to black and clear window. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
        
    /* Create a rectangle structure. */
    SDL_Rect rect;
    rect.x = 25;
    rect.y = 25;
    rect.w = 100;
    rect.h = 100;
    
    /* Set the renderer color to red. */
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    
    /* Draw the rectangle to the renderer attached to window. */
    SDL_RenderDrawRect(renderer, &rect);
    
    // Render the rect to the screen
    SDL_RenderPresent(renderer);
    
    SDL_Delay(5000);  // Pause execution for 3000 milliseconds, for example
   
    /* Clean up stuff. */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    return 0;
}