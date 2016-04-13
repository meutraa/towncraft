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
    
    SDL_Surface* cat_surf = SDL_LoadBMP("resources/cat.bmp");
    SDL_Texture* cat_tex = SDL_CreateTextureFromSurface(renderer, cat_surf);
    int width = cat_surf->w;
    int height = cat_surf->h;
    SDL_FreeSurface(cat_surf);
    
    SDL_Rect src_rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = width;
    rect.h = height;
    
    SDL_Rect dest_rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 150;
    rect.h = 25;
    
    printf("%d\n", cat_surf->w);
    printf("%d\n", cat_surf->h);
    
        
    int render_error = SDL_RenderCopy(
        renderer,
        cat_tex,
        NULL,
        &dest_rect
    );
    
    printf("%d\n", render_error);
    
    // Render the rect to the screen
    SDL_RenderPresent(renderer);
    
    /* Start the main loop. */
    SDL_Event event; 
    while(1)
    {
        /* If there are events in the event queue, process them. */
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
                    switch(event.key.keysym.scancode)
                    {
                        case 41: // ESC - Close the program.
                            return 0;
                            break;
                        default:
                            break;
                    }
                    // printf("%d\n", event.key.keysym.scancode); // Use to determine scancodes
                    return 1;
                    break;
                default:
                    break;
            }
        }
        
        /* Finished dealing with events; render stuff now. */
        
        
        /* Wait for next frame */
        SDL_Delay(300);
    }
   
    /* Clean up stuff. */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    return 0;
}