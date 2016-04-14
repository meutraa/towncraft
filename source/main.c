#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <stdlib.h>

/* Function declarations. */
void main_menu_event_loop();

void resize_scalables();

/* Checks whether a co-ordinate is inside an SDL_Rect boundry. */
int bounded_by(int x, int y, SDL_Rect r);

/* Free up memory. */
void quit();

#define GAME_NAME "Towncraft"
#define SCA_COUNT 2     // Number of Scalables
#define TEX_COUNT 1     // Number of Textures

/* Current window dimensions. */
int win_width = 1600;
int win_height = 800;

/* User option to scale ui elements. */
float win_scale = 1.0f;

/* Global for cleanup on quit. */
SDL_Renderer* renderer;
SDL_Window* window;

/* Number of textures we are loading. */
char* texture_paths[TEX_COUNT] = {
    "resources/button.pnm"    // 0
};

/* A structure that stores the initial texture with width and height. */
struct Texture 
{
    SDL_Texture *texture;
    int width;
    int height;
} textures[TEX_COUNT];  // Create an array of these structs

/* Contains the initial position of the scalable in terms of a percentage. */
struct Scalable 
{
    int texture_id;
    SDL_Rect rect;
    float initial_width_percentage;
    float initial_height_percentage;
} scalables[SCA_COUNT]; // Create an array of these structs

/* Create a new Scalable struct. */
struct Scalable CreateScalable(float x, float y, int texture_id)
{
    struct Scalable scalable;
    SDL_Rect rect;
    rect.x = x * win_width;
    rect.y = y * win_height;
    rect.w = textures[texture_id].width;
    rect.h = textures[texture_id].height;
    scalable.texture_id = texture_id;
    scalable.rect = rect;
    scalable.initial_width_percentage = x;
    scalable.initial_height_percentage = y;
    return scalable;
}

int main(int argc, char** argv)
{
    int vsync = 1;
    
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
        // Do not quit if audio fails
    }
    
    /* Create window */
    window = SDL_CreateWindow(
        GAME_NAME,                                       // Window title
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  // Position (x,y)
        win_width, win_height,                           // Size (x,y)
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
        textures[i].texture = SDL_CreateTextureFromSurface(renderer, surface);
        textures[i].width   = surface->w;
        textures[i].height  = surface->h;
        SDL_FreeSurface(surface);
    }
    
    /* Set renderer colour to black and clear window. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    scalables[0] = CreateScalable(0.8, 0.9, 0); // Button for options
    scalables[1] = CreateScalable(0.9, 0.9, 0); // Button for exit
    
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

int bounded_by(int x, int y, SDL_Rect r)
{
    if((x > r.x && x < r.x + r.w) && (y > r.y && y < r.y + r.h))
        return 1;
    return 0;
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