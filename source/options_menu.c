#include "options_menu.h"

#include <stdio.h>
#include <string.h>

#include "drawable.h"
#include "status.h"

#define MENU_LENGTH 4
static const char* layouts[MENU_LENGTH] = {
    "resources/layouts/options_menu.csv",
    "resources/layouts/options_video.csv",
    "resources/layouts/options_audio.csv",
    "resources/layouts/options_controls.csv",
};

/* 1 = video, 2 = audio, 3 = controls */
static int menu = 1;

Status options_menu_loop(SDL_Renderer* renderer)
{
    Drawable* menus[4];
    for(int i = 0; i < MENU_LENGTH; i++)
    {
        menus[i] = load_drawables(renderer, layouts[i]);
    }

    Status status = NORMAL;
    while (NORMAL == status) {
        /* If there are events in the event queue, process them. */
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (SDL_KEYDOWN == event.type)
            {
                if (41 == event.key.keysym.scancode) // ESC - Close the program.
                {
                    status = QUIT_PROGRAM;
                    break;
                }
            }
            if (SDL_MOUSEBUTTONDOWN == event.type && SDL_BUTTON_LEFT == event.button.button)
            {
                /* Loop through the drawables in the side bar. */
                for (int i = 0; NULL != (menus[0] + i)->texture; i++)
                {
                    if (SDL_PointInRect(&((SDL_Point){event.button.x, event.button.y}), (menus[0] + i)->rect))
                    {
                        if (!strcmp((menus[0] + i)->name, "button_options_video"))
                        {
                            menu = 1;
                        }
                        else if (!strcmp((menus[0] + i)->name, "button_options_audio"))
                        {
                            menu = 2;
                        }
                        else if (!strcmp((menus[0] + i)->name, "button_options_controls"))
                        {
                            menu = 3;
                        }
                        else if (!strcmp((menus[0] + i)->name, "button_options_back"))
                        {
                            status = SWITCHTO_MAINMENU;
                        }
                    }
                }
            }
        }

        /* Copy the drawables to the window. */
        render_drawables(renderer, menus[0]);
        render_drawables(renderer, menus[menu]);
        SDL_RenderPresent(renderer);
    }

    /* Clean up and return to the main function. */
    for(int i = 0; i < MENU_LENGTH; i++)
    {
        destroy_drawables(menus[i]);
    }
    return status;
}
