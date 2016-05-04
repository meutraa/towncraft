#include <stdio.h>
#include <string.h>

#include "options_menu.h"
#include "drawable.h"
#include "status.h"
#include "macros.h"

static const char* layouts[] = {
    "resources/layouts/options_menu.csv",
    "resources/layouts/options_video.csv",
    "resources/layouts/options_audio.csv",
    "resources/layouts/options_controls.csv",
    NULL,
};

/* 1 = video, 2 = audio, 3 = controls */
static int menu = 1;

Status options_menu_loop(SDL_Renderer* renderer)
{
    Drawable* menus[4];
    for(int i = 0; layouts[i]; i++)
    {
        menus[i] = load_drawables(renderer, layouts[i]);
    }

    Status status = NORMAL;
    while (NORMAL == status) {
        /* If there are events in the event queue, process them. */
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if(SDL_MOUSEBUTTONDOWN == event.type && SDL_BUTTON_LEFT == event.button.button)
            {
                /* Loop through the drawables in the side bar. */
                for(int i = 0; NULL != (menus[0] + i)->texture; i++)
                {
                    SDL_Point point = {event.button.x, event.button.y};
                    if(SDL_TRUE == SDL_PointInRect(&point, (menus[0] + i)->rect))
                    {
                        char* name = (menus[0] + i)->name;
                             SETEQ(name, "button_options_video",    menu, 1)
                        else SETEQ(name, "button_options_audio",    menu, 2)
                        else SETEQ(name, "button_options_controls", menu, 3)
                        else SETEQ(name, "button_options_back", status, SWITCHTO_MAINMENU)
                    }
                }
            }
        }

        /* Copy the drawables to the window. */
        render_drawables(renderer, menus[0], 1);
        render_drawables(renderer, menus[menu], 1);
        SDL_RenderPresent(renderer);
    }

    /* Clean up and return to the main function. */
    for(int i = 0; layouts[i]; destroy_drawables(menus[i++]));
    return status;
}
