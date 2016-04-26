#include "options_menu.h"

#include <stdio.h>
#include <string.h>

#include "drawable.h"
#include "status.h"

/* 1 = video, 2 = audio, 3 = controls */
static int menu = 1;

Status options_menu_loop(SDL_Renderer* renderer)
{
    static const char* layouts[] = {
        "resources/layouts/options_menu.csv",
        "resources/layouts/options_video.csv",
        "resources/layouts/options_audio.csv",
        "resources/layouts/options_controls.csv",
    };

    const int options_count = count_drawables(layouts[0]);
    const int video_count = count_drawables(layouts[1]);
    const int audio_count = count_drawables(layouts[2]);
    const int control_count = count_drawables(layouts[3]);

    Drawable options_menu[options_count];
    Drawable video_menu  [video_count];
    Drawable audio_menu  [audio_count];
    Drawable control_menu[control_count];

    load_drawables(renderer, options_menu, layouts[0], 0);
    load_drawables(renderer, video_menu,   layouts[1], 0);
    load_drawables(renderer, audio_menu,   layouts[2], 0);
    load_drawables(renderer, control_menu, layouts[3], 0);

    Status status = NORMAL;
    while (NORMAL == status) {
        /* If there are events in the event queue, process them. */
        SDL_Event event;
        while (1 == SDL_PollEvent(&event))
        {
            if (SDL_KEYDOWN == event.type)
            {
                if (41 == event.key.keysym.scancode) // ESC - Close the program.
                {
                    return QUIT_PROGRAM;
                }
                else
                {
                    printf("Key %d pressed\n", event.key.keysym.scancode);
                }
            }
            if (SDL_MOUSEBUTTONDOWN == event.type && SDL_BUTTON_LEFT == event.button.button)
            {
                /* Loop through the drawables in the side bar. */
                for (int i = 0; i < options_count; i++)
                {
                    if (bounded_by(event.button.x, event.button.y, options_menu[i].rect))
                    {
                        if (0 == strcmp(options_menu[i].name, "button_options_video"))
                        {
                            menu = 1;
                        }
                        else if (0 == strcmp(options_menu[i].name, "button_options_audio"))
                        {
                            menu = 2;
                        }
                        else if (0 == strcmp(options_menu[i].name, "button_options_controls"))
                        {
                            menu = 3;
                        }
                        if (0 == strcmp(options_menu[i].name, "button_options_back"))
                        {
                            status = SWITCHTO_MAINMENU;
                            break;
                        }
                    }
                }
            }
        }

        /* Copy the drawables to the window. */
        render_drawables(renderer, options_menu, options_count);
        switch (menu)
        {
            case 1:
                render_drawables(renderer, video_menu, video_count);
                break;
            case 2:
                render_drawables(renderer, audio_menu, audio_count);
                break;
            case 3:
                render_drawables(renderer, control_menu, control_count);
                break;
        }

        /* Draw the renderer. */
        SDL_RenderPresent(renderer);
    }

    /* Clean up and return to the main function. */
    destroy_drawables(options_menu, options_count);
    destroy_drawables(video_menu, video_count);
    destroy_drawables(audio_menu, audio_count);
    destroy_drawables(control_menu, control_count);
    return status;
}
