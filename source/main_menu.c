#include <stdio.h>
#include <string.h>

#include "main_menu.h"
#include "SDL_mixer.h"
#include "drawable.h"
#include "status.h"

static Status main_menu_event_loop(Drawable* drawables)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (SDL_KEYDOWN == event.type)
        {
            if (41 == event.key.keysym.scancode) // ESC - Close the program.
            {
                return QUIT_PROGRAM;
            }
            if (82 == event.key.keysym.scancode)
            {
                Mix_VolumeMusic(MIX_MAX_VOLUME);
            }
            if (81 == event.key.keysym.scancode)
            {
                Mix_VolumeMusic(0);
            }
            else
            {
                printf("Key %d pressed\n", event.key.keysym.scancode);
            }
        }
        if (SDL_MOUSEBUTTONDOWN == event.type && SDL_BUTTON_LEFT == event.button.button)
        {
            for (int i = 0; (drawables + i)->texture; i++)
            {
                if (bounded_by(event.button.x, event.button.y, (drawables + i)->rect))
                {
                    if (!strcmp((drawables + i)->name, "button_quit"))
                    {
                        return QUIT_PROGRAM;
                    }
                    if (!strcmp((drawables + i)->name, "button_options"))
                    {
                        return SWITCHTO_OPTIONS;
                    }
                    if (!strcmp((drawables + i)->name, "button_new"))
                    {
                        return SWITCHTO_GAME;
                    }
                }
            }
        }
    }
    return NORMAL;
}

Status main_menu_loop(SDL_Renderer* renderer)
{
    const char* layout_file = "resources/layouts/main_menu.csv";
    Drawable* drawables = load_drawables(renderer, layout_file);

    Status status = NORMAL;
    while (NORMAL == status)
    {
        /* If there are events in the event queue, process them. */
        status = main_menu_event_loop(drawables);

        /* Copy all the scalables to the window. */
        render_drawables(renderer, drawables);

        /* Draw the renderer. */
        SDL_RenderPresent(renderer);
    }

    /* Clean up and return to the main function. */
    destroy_drawables(drawables);
    return status;
}
