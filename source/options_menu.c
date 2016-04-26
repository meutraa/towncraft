#include "options_menu.h"

#include <stdio.h>
#include <string.h>

#include "drawable.h"
#include "status.h"

/* This function is unpure and accesses global drawable arrays and counts! */
static Status options_menu_event_loop(void);

#define DRAWABLE_ARRAYS 4
static const char* layouts[DRAWABLE_ARRAYS] = {
    "resources/layouts/options_menu.csv",
    "resources/layouts/options_video.csv",
    "resources/layouts/options_audio.csv",
    "resources/layouts/options_controls.csv",
};

static int counts[DRAWABLE_ARRAYS];
static Drawable* drawables[DRAWABLE_ARRAYS];

/* 1 = video, 2 = audio, 3 = controls */
static int menu = 1;

Status options_menu_loop(SDL_Renderer* renderer)
{
    Status status = NORMAL;

    for (int i = 0; i < DRAWABLE_ARRAYS; i++) {
        counts[i] = load_drawables(renderer, &drawables[i], layouts[i]);
    }

    while (NORMAL == status) {
        /* If there are events in the event queue, process them. */
        status = options_menu_event_loop();

        /* Copy all the scalables to the window. */
        for (int i = 0; i < DRAWABLE_ARRAYS; i++) {
            render_drawables(renderer, drawables[i], counts[i]);
        }

        /* Draw the renderer. */
        SDL_RenderPresent(renderer);
    }

    /* Clean up and return to the main function. */
    for (int i = 0; i < DRAWABLE_ARRAYS; i++) {
        destroy_drawables(&drawables[i], counts[i]);
    }
    return status;
}

static Status options_menu_event_loop()
{
    SDL_Event event;
    while (1 == SDL_PollEvent(&event)) {
        if (SDL_KEYDOWN == event.type) {
            if (41 == event.key.keysym.scancode) // ESC - Close the program.
                return QUIT_PROGRAM;
            else
                printf("Key %d pressed\n", event.key.keysym.scancode);
        }
        if (SDL_MOUSEBUTTONDOWN == event.type && SDL_BUTTON_LEFT == event.button.button) {
            /* Loop through the drawables in the side bar. */
            for (int i = 0; i < counts[0]; i++) {
                if (bounded_by(event.button.x, event.button.y, drawables[0][i].rect)) {
                    if (0 == strcmp(drawables[0][i].name, "button_options_video")) {
                        show_drawables(&drawables[menu], counts[menu], 0);
                        menu = 1;
                        show_drawables(&drawables[menu], counts[menu], 1);
                    } else if (0 == strcmp(drawables[0][i].name, "button_options_audio")) {
                        show_drawables(&drawables[menu], counts[menu], 0);
                        menu = 2;
                        show_drawables(&drawables[menu], counts[menu], 1);
                    } else if (0 == strcmp(drawables[0][i].name, "button_options_controls")) {
                        show_drawables(&drawables[menu], counts[menu], 0);
                        menu = 3;
                        show_drawables(&drawables[menu], counts[menu], 1);
                    }
                    if (0 == strcmp(drawables[0][i].name, "button_options_back"))
                        return SWITCHTO_MAINMENU;
                }
            }
        }
    }
    return NORMAL;
}
