#include <stdio.h>
#include <stdlib.h>

#include "drawable.h"
#include "status.h"
#include "file.h"
#include "math.h"
#include "SDL_mixer.h"

static Status main_menu_event_loop();

static char *layout_file = "resources/layouts/main_menu.csv";
static int count;
static Drawable* drawables;

Status main_menu_loop(SDL_Renderer* renderer)
{
	count = load_drawables(renderer, &drawables, layout_file);

	Status status = NORMAL;
	while(NORMAL == status)
	{
		/* If there are events in the event queue, process them. */
		status = main_menu_event_loop();

		/* Copy all the scalables to the window. */
		render_drawables(renderer, drawables, count);

		/* Draw the renderer. */
		SDL_RenderPresent(renderer);
	}

	/* Clean up and return to the main function. */
	destroy_drawables(&drawables, count);
	return status;
}

static Status main_menu_event_loop()
{
	SDL_Event event;
	while(1 == SDL_PollEvent(&event))
	{
		if(SDL_KEYDOWN == event.type)
		{
			if(41 == event.key.keysym.scancode) // ESC - Close the program.
				return QUIT_PROGRAM;
			if(40 == event.key.keysym.scancode)
				return SWITCHTO_GAME;
			if(82 == event.key.keysym.scancode)
				Mix_VolumeMusic(MIX_MAX_VOLUME);
			if(81 == event.key.keysym.scancode)
				Mix_VolumeMusic(0);
			else
				printf("Key %d pressed\n", event.key.keysym.scancode);
		}
		if(SDL_MOUSEBUTTONDOWN == event.type && SDL_BUTTON_LEFT == event.button.button)
		{
			for(int i = 0; i < count; i ++)
			{
				if(bounded_by(event.button.x, event.button.y, drawables[i].rect))
				{
					if(0 == strcmp(drawables[i].name, "button_quit"))
						return QUIT_PROGRAM;
					if(0 == strcmp(drawables[i].name, "button_options"))
						return SWITCHTO_OPTIONS;
				}
			}
		}
	}
	return NORMAL;
}
