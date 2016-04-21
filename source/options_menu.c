#include <stdio.h>
#include <stdlib.h>

#include "drawable.h"
#include "file.h"
#include "math.h"
#include "status.h"

/* This function is unpure and accesses global drawable arrays and counts! */
static Status options_menu_event_loop();

static char *layouts[] = {
	"resources/layouts/options_menu.csv",
	"resources/layouts/options_video.csv",
	"resources/layouts/options_audio.csv",
	"resources/layouts/options_controls.csv",
};

static Status submenu = NONE;

static int drawable_counts[4];
static Drawable* drawables[4];

Status options_menu_loop(SDL_Renderer* renderer)
{
	Status status = NORMAL;

	for(int i = 0; i < 4; i++)
	{
		drawables[i] = calloc(count_lines(layouts[i]), sizeof(Drawable));
		drawable_counts[i] = load_drawables(renderer, &drawables[i], layouts[i]);
		if(0 == drawable_counts[i])
		{
			status = QUIT_PROGRAM;
			break;
		}
	}

	while(NORMAL == status)
	{
		/* If there are events in the event queue, process them. */
		status = options_menu_event_loop();

		/* Fill the screen with the background color. */
		SDL_RenderClear(renderer);

		/* Copy all the scalables to the window. */
		render_drawables(renderer, drawables[0], drawable_counts[0]);
		if(SHOW_OPTIONS_VIDEO == submenu)
			render_drawables(renderer, drawables[1], drawable_counts[1]);
		else if(SHOW_OPTIONS_AUDIO == submenu)
			render_drawables(renderer, drawables[2], drawable_counts[2]);
		else if(SHOW_OPTIONS_CONTROLS == submenu)
			render_drawables(renderer, drawables[3], drawable_counts[3]);

		/* Draw the renderer. */
		SDL_RenderPresent(renderer);
	}

	/* Clean up and return to the main function. */
	for(int i = 0; i < 4; i ++)
	{
		for(int j = 0; j < drawable_counts[i]; j++)
		{
			free(drawables[i][j].name);
			SDL_DestroyTexture(drawables[i][j].texture);
		}
		free(drawables[i]);
	}
	return status;
}

static Status options_menu_event_loop()
{
	SDL_Event event;
	while(1 == SDL_PollEvent(&event))
	{
		if(SDL_KEYDOWN == event.type)
		{
			if(41 == event.key.keysym.scancode) // ESC - Close the program.
				return QUIT_PROGRAM;
			else
				printf("Key %d pressed\n", event.key.keysym.scancode);
		}
		if(SDL_MOUSEBUTTONDOWN == event.type && SDL_BUTTON_LEFT == event.button.button)
		{
			/* Loop through the drawables in the side bar. */
			for(int i = 0; i < drawable_counts[0]; i ++)
			{
				if(bounded_by(event.button.x, event.button.y, drawables[0][i].rect))
				{
					if(0 == strcmp(drawables[0][i].name, "button_options_video"))
						submenu = SHOW_OPTIONS_VIDEO;
					else if(0 == strcmp(drawables[0][i].name, "button_options_audio"))
						submenu = SHOW_OPTIONS_AUDIO;
					else if(0 == strcmp(drawables[0][i].name, "button_options_controls"))
						submenu = SHOW_OPTIONS_CONTROLS;
					if(0 == strcmp(drawables[0][i].name, "button_options_back"))
						return SWITCHTO_MAINMENU;
				}
			}
		}
	}
	return NORMAL;
}
