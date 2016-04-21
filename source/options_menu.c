#include <stdio.h>
#include <stdlib.h>

#include "drawable.h"
#include "file.h"
#include "math.h"
#include "status.h"
#include "functions.h"

/* This function is unpure and accesses global drawable arrays and counts! */
static Return options_menu_event_loop();

static char *layouts[] = {
	"resources/layouts/options_menu.layout",
	"resources/layouts/options_video.layout",
	"resources/layouts/options_audio.layout",
	"resources/layouts/options_controls.layout",
};

static Return submenu = NONE;

static int drawable_counts[4];
static Drawable* drawables[4];

Return options_menu_loop(SDL_Renderer* renderer)
{
	Return status = NORMAL;

	/* BLOCK START */
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
	/* BLOCK END */

	while(NORMAL == status)
	{
		/* If there are events in the event queue, process them. */
		status = options_menu_event_loop();

		/* Fill the screen with the background color. */
		SDL_RenderClear(renderer);

		/* Copy all the scalables to the window. */
		render_drawables(renderer, drawables[0], drawable_counts[0]);
		if(SWITCHTO_OPTIONS_VIDEO == submenu)
			render_drawables(renderer, drawables[1], drawable_counts[1]);
		else if(SWITCHTO_OPTIONS_AUDIO == submenu)
			render_drawables(renderer, drawables[2], drawable_counts[2]);
		else if(SWITCHTO_OPTIONS_CONTROLS == submenu)
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

static Return options_menu_event_loop()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_KEYDOWN:
			switch(event.key.keysym.scancode)
			{
				case 41: // ESC - Close the program.
					return QUIT_PROGRAM;
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

					/* Check clicks for side bar. */
					for(int i = 0; i < drawable_counts[0]; i ++)
					{
						if(bounded_by(x, y, drawables[0][i].rect))
						{
							int index = get_function_index(drawables[0][i].name);
							switch(index)
							{
								case 2:	/* fun_main() */
									return function_pointers[index](0);
								case 3: /* fun_options_video() */
								case 4: /* fun_options_audio() */
								case 5: /* fun_options_controls() */
									submenu = function_pointers[index](0);
									break;
								default:
									break;
							}
						}
					}
				}
			}
			break;
		default:
			break;
		}
	}
	return NORMAL;
}
