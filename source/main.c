#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <stdlib.h>

#define SIZE(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

const char* GAME_NAME = "Towncraft";
int win_width = 1600;
int win_height = 800;
float win_scale = 1.0f;
SDL_Renderer* renderer;

#define TEX_COUNT  1

char* texture_paths[TEX_COUNT] = {
    "resources/button.pnm"    // 0
};

struct Texture 
{
    SDL_Texture *texture;
    int width;
    int height;
};

struct Texture textures[TEX_COUNT];

struct Scalable 
{
    int texture_id;
    SDL_Rect rect;
};

struct Scalable CreateScalable(int x, int y, int texture_id)
{
    struct Scalable scalable;
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = textures[texture_id].width;
    rect.h = textures[texture_id].height;
    scalable.texture_id = texture_id;
    scalable.rect = rect;
    return scalable;
}

int inside(int x, int y, SDL_Rect r)
{
    if((x > r.x && x < r.x + r.w) && (y > r.y && y < r.y + r.h))
        return 1;
    return 0;
}

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
    renderer = SDL_CreateRenderer(window, -1, flags);
    
    if(NULL == renderer)
    {
        fprintf(stderr, "\nCould not create renderer: %s\n", SDL_GetError());
        return 1;
    }
    
    /* Load all textures to arrays. */
    int i;
    for(i = 0; i < TEX_COUNT; i++)
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

    /* Colors for background rainbow. */
    int r = 0, g = 0, b = 0;
     
    /*SDL_Rect button_opt_rect;
    button_opt_rect.x = win_width - win_width/10;
    button_opt_rect.y = win_height - win_height/5;
    button_opt_rect.w = button_w;
    button_opt_rect.h = button_h;
    
    SDL_Rect button_exit_rect;
    button_exit_rect.x = win_width - win_width/10;
    button_exit_rect.y = win_height - win_height/10;
    button_exit_rect.w = button_w;
    button_exit_rect.h = button_h;*/
    
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
                        case 80:    // LEFT - shrink ui scale
                            win_scale -= 0.1f;
                            break;
                        case 79:    // RIGHT - grow ui scale
                            win_scale += 0.1f;
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
                            /*if(inside(x, y, button_exit_rect))
                            {
                                return 0;
                            }*/
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
                           /* button_opt_rect.x = win_width - win_width/10;
                            button_opt_rect.y = win_height - win_height/20;
                            button_exit_rect.x = win_width - win_width/10;
                            button_exit_rect.y = win_height - win_height/10;*/
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
        //SDL_RenderCopy(renderer, button_tex, NULL, &button_opt_rect);
        //SDL_RenderCopy(renderer, button_tex, NULL, &button_exit_rect);
        
        /* Draw the renderer. */
        SDL_RenderPresent(renderer);
    }
   
    /* Clean up stuff. */
    for(i = 0; i < TEX_COUNT; i++)
    {
        SDL_DestroyTexture(textures[i].texture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    return 0;
}