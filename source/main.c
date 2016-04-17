#include "main.h"
#include "file.h"
#include "math.h"
#include "scalable.h"
#include "texture.h"

#include <stdio.h>
#include <stdlib.h>

/* Current window dimensions. */
int win_width;
int win_height;

/* User option to scale ui elements. */
float win_scale = 1.0f;

/* Global for cleanup on quit. */
SDL_Renderer* renderer;
SDL_Window* window;

/* Number of textures we are loading. */
char* texture_paths[TEX_COUNT] = {
    "resources/button.pnm"    // 0
};

/* Create our texture arrays. */
struct Texture textures[TEX_COUNT];
struct Scalable scalables[SCA_COUNT];

int main(int argc, char** argv)
{
    int vsync = 1;
    
    /* fullscreen = 0, use fallback/window res,
     * fullscreen = 1, use fullscreen at fallback/window res,
     * fullscreen = 2, user fullscreen at current desktop res
     */
    int fullscreen = 0;
    win_width = FALLBACK_RES_WIDTH;
    win_height = FALLBACK_RES_HEIGHT;
    
    /* Ensures any return will call SDL_Quit first. */
    atexit(quit);
    
    /* Initialise the video and timer subsystem. */
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER))
    {
        fprintf(stderr, "\nUnable to initialize SDL Subsystem:  %s\n", SDL_GetError());
        return 1;
    }
    
    /* Initialise the (optional) audio subsystem. */
    if(SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        fprintf(stderr, "\nUnable to initialize SDL Audio Subsystem:  %s\n", SDL_GetError());
    }
    
    /* Create window */
    window = SDL_CreateWindow(
        GAME_NAME,                                       // Window title
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  // Position (x,y)
        win_width, win_height,                           // Size (x,y)
        SDL_WINDOW_RESIZABLE|
        ((fullscreen == 1) ? SDL_WINDOW_FULLSCREEN : 0)|        // If custom_resolution is set use size x,y fullscreen
        ((fullscreen == 2) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) // If fullscreen is set use desktop resolution
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
    renderer = SDL_CreateRenderer(window, -1, flags);
    
    if(NULL == renderer)
    {
        fprintf(stderr, "\nCould not create renderer: %s\n", SDL_GetError());
        return 1;
    }
    
    /* Load all textures to arrays. */
    for(int i = 0; i < TEX_COUNT; i++)
    {
        SDL_Surface* surface = IMG_Load(texture_paths[i]);
        if(NULL == surface)
        {
            fprintf(stderr, "\nIMG_Load failed to load \"%s\" to surface\n", texture_paths[i]);
        }
        else
        {
            textures[i].texture = SDL_CreateTextureFromSurface(renderer, surface);
            textures[i].width   = surface->w;
            textures[i].height  = surface->h;
            SDL_FreeSurface(surface);
        }
    }
    
    /* Set renderer colour to black and clear window. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    scalables[0] = create_scalable(0.8, 0.9, textures[0].width, textures[0].height, 0); // Button for options
    scalables[1] = create_scalable(0.9, 0.9, textures[0].width, textures[0].height, 0); // Button for exit
    
    /* Set the background color to black. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    
    /* Start the main loop. */
    while(1)
    {
        /* If there are events in the event queue, process them. */
        main_menu_event_loop();
               
        /* Fill the screen with the background color. */
        SDL_RenderClear(renderer);
        
        /* Copy all the scalables to the window. */
        for(int i = 0; i < SCA_COUNT; i++)
        {
            SDL_RenderCopy(renderer, textures[scalables[i].texture_id].texture, NULL, &scalables[i].rect);
        }
        
        /* Draw the renderer. */
        SDL_RenderPresent(renderer);
    }
    
    return 0;
}

void main_menu_event_loop()
{
    SDL_Event event; 
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_KEYDOWN:
            switch(event.key.keysym.scancode)
            {
                case 41: // ESC - Close the program.
                    exit(0);
                    break;
                case 80:    // LEFT - shrink ui scale
                    win_scale -= 0.05f;
                    resize_scalables();
                    break;
                case 79:    // RIGHT - grow ui scale
                    win_scale += 0.05f;
                    resize_scalables();
                    break;
                default:
                    printf("Key %d pressed\n", event.key.keysym.scancode);
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
                    
                    /* If the click is within the exit button boundry. */
                    if(bounded_by(x, y, scalables[1].rect)) exit(0);
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
                    for(int i = 0; i < SCA_COUNT; i++)
                    {
                        scalables[i].rect.x = scalables[i].initial_width_percentage  * win_width;
                        scalables[i].rect.y = scalables[i].initial_height_percentage * win_height;
                    }
                }
                default:
                    break;
            }
        default:
            break;
        }
    }
}

void resize_scalables()
{
    for(int i = 0; i < SCA_COUNT; i++)
    {
        scalables[i].rect.w = textures[scalables[i].texture_id].width  * win_scale;
        scalables[i].rect.h = textures[scalables[i].texture_id].height * win_scale;
    }
}

void quit()
{
    for(int i = 0; i < TEX_COUNT; i++)
    {
        SDL_DestroyTexture(textures[i].texture);
    }
    if(NULL != renderer) SDL_DestroyRenderer(renderer);
    if(NULL != window) SDL_DestroyWindow(window);
    printf("shutdown complete\n");
    SDL_Quit();
}