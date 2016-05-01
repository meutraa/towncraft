#include <stdio.h>
#include <string.h>

#include "main_menu.h"
#include "SDL_mixer.h"
#include "drawable.h"
#include "status.h"

Status main_menu_loop(SDL_Renderer* renderer)
{
    const char* layout_file = "resources/layouts/main_menu.csv";
    Drawable* drawables = load_drawables(renderer, layout_file);

    Status status = NORMAL;
    SDL_Event event;

    while (NORMAL == status)
    {
        /* If there are events in the event queue, process them. */
        while (SDL_PollEvent(&event))
        {
            if (SDL_KEYDOWN == event.type)
            {
                if (41 == event.key.keysym.scancode) // ESC - Close the program.
                {
                    status = QUIT_PROGRAM;
                }
                else if (82 == event.key.keysym.scancode)
                {
                    Mix_VolumeMusic(MIX_MAX_VOLUME);
                }
                else if (81 == event.key.keysym.scancode)
                {
                    Mix_VolumeMusic(0);
                }
            }
            else if (SDL_MOUSEBUTTONDOWN == event.type && SDL_BUTTON_LEFT == event.button.button)
            {
                for (int i = 0; (drawables + i)->texture; i++)
                {
                    SDL_Point point = {event.button.x, event.button.y};
                    if (SDL_TRUE == SDL_PointInRect(&point, (drawables + i)->rect))
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

        /* Copy all the scalables to the window. */
        render_drawables(renderer, drawables);

        /* Draw the renderer. */
        SDL_RenderPresent(renderer);
    }

    /* Clean up and return to the main function. */
    destroy_drawables(drawables);
    return status;
}
