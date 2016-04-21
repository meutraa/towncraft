#include <stdio.h>
#include <stdlib.h>

#include "drawable.h"
#include "status.h"
#include "file.h"
#include "math.h"
#include "SDL_mixer.h"

static Status main_menu_event_loop(Drawable drawables[], int drawable_count);

Status main_menu_loop(SDL_Renderer* renderer)
{
	char *layout_file = "resources/layouts/main_menu.csv";

	Drawable* drawables = (Drawable*) calloc(count_lines(layout_file), sizeof(Drawable));
	int drawable_count = load_drawables(renderer, &drawables, layout_file);
	if(0 == drawable_count)
	{
		return QUIT_PROGRAM;
	}

	Status status = NORMAL;
	while(NORMAL == status)
	{
		/* If there are events in the event queue, process them. */
		status = main_menu_event_loop(drawables, drawable_count);

		/* Copy all the scalables to the window. */
		render_drawables(renderer, drawables, drawable_count);

		/* Draw the renderer. */
		SDL_RenderPresent(renderer);
	}

	/* Clean up and return to the main function. */
	for(int i = 0; i < drawable_count; i++)
	{
		free(drawables[i].name);
		SDL_DestroyTexture(drawables[i].texture);
	}
	free(drawables);
	return status;
}

static Status main_menu_event_loop(Drawable drawables[], int drawable_count)
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
			for(int i = 0; i < drawable_count; i ++)
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
