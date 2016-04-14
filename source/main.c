#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <stdlib.h>

const char* GAME_NAME = "Towncraft";
int win_width = 1600;
int win_height = 800;
float win_scale = 1.0f;

int main(int argc, char** argv)
{
    int vsync = 1;
    
    /* Ensures any return will call SDL_Quit first. */
    atexit(SDL_Quit);
    
    /* Initialise the video and timer subsystem. */
    int iret = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    if(iret != 0)
    {
        fprintf(stderr, "\nUnable to initialize SDL Subsystem:  %s\n", SDL_GetError());
        return 1;
    }
    
    /* Initialise the (optional) audio subsystem. */
    iret = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if(iret != 0)
    {
        fprintf(stderr, "\nUnable to initialize SDL Audio Subsystem:  %s\n", SDL_GetError());
        // Do not quit if audio fails
    }
    
    /* Create window */
    SDL_Window* window = SDL_CreateWindow(
        GAME_NAME,                 // Window title
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  // Position (x,y)
        win_width, win_height,     // Size (x,y)
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
    
    /* Create bitmap texture. */
    SDL_Surface* cat_surf = SDL_LoadBMP("resources/cat.bmp");
    SDL_Texture* cat_tex = SDL_CreateTextureFromSurface(renderer, cat_surf);
    SDL_FreeSurface(cat_surf);
    
    /* Rectangle to draw the cat. */
    SDL_Rect cat_rect;
    cat_rect.x = 150;
    cat_rect.y = 25;
    cat_rect.w = 640;
    cat_rect.h = 480;
    
    /* Colors for background rainbow. */
    int r = 0, g = 0, b = 0;
    
    /* Load button to texture. */
    SDL_RWops* rwop = SDL_RWFromFile("resources/button.pnm", "rb");
    SDL_Surface* button_surf = IMG_LoadPNM_RW(rwop);
    SDL_Texture* button_tex = SDL_CreateTextureFromSurface(renderer, button_surf);
    printf("loading w+h\n");
    int button_w = button_surf->w;
    int button_h = button_surf->h;
    SDL_FreeSurface(button_surf);
    printf("done\n");
    
    SDL_Rect button_opt_rect;
    button_opt_rect.x = win_width - win_width/10;
    button_opt_rect.y = win_height - win_height/5;
    button_opt_rect.w = button_w;
    button_opt_rect.h = button_h;
    
    SDL_Rect button_exit_rect;
    button_exit_rect.x = win_width - win_width/10;
    button_exit_rect.y = win_height - win_height/10;
    button_exit_rect.w = button_w;
    button_exit_rect.h = button_h;
    
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
                        {
                            return 0;
                            break;
                        }
                        default:
                            break;
                    }
                    // printf("%d\n", event.key.keysym.scancode); // Use to determine scancodes
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    switch(event.button.button)
                    {
                        default:    // For now, fall through and do the same as button left.
                        case SDL_BUTTON_LEFT:
                        {
                            int x = event.button.x;
                            int y = event.button.y;
                            int time = event.button.timestamp;
                            printf("Button %d pressed at %d,%d at %d\n", event.button.button, x, y, time);
                            break;
                        }
                    }
                    break;
                case SDL_WINDOWEVENT:
                    switch(event.window.event)
                    {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            win_width  = event.window.data1;
                            win_height = event.window.data2;
                            button_opt_rect.x = win_width - win_width/10;
                            button_opt_rect.y = win_height - win_height/20;
                            button_exit_rect.x = win_width - win_width/10;
                            button_exit_rect.y = win_height - win_height/10;
                        }
                        default:
                            break;
                    }
                default:
                    break;
            }
        }
        /* Finished dealing with events; render stuff now. */
       
        /* Set the draw color and fill the screen with it. */
        SDL_SetRenderDrawColor(renderer, (r+=1)%255, (g+=2)%255, (b+=3)%255, 255);
        SDL_RenderClear(renderer);
        
        /* Copy the cat to the destination rectangle on the renderer. */
        printf("rendering\n");
        SDL_RenderCopy(renderer, cat_tex, NULL, &cat_rect);
        SDL_RenderCopy(renderer, button_tex, NULL, &button_opt_rect);
        SDL_RenderCopy(renderer, button_tex, NULL, &button_exit_rect);
        
        /* Draw the renderer. */
        SDL_RenderPresent(renderer);
    }
   
    /* Clean up stuff. */
    SDL_DestroyTexture(cat_tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    return 0;
}