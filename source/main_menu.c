#include <stdio.h>
#include <stdlib.h>

#include "drawable.h"
#include "file.h"
#include "math.h"
#include "functions.h"
#include "SDL_mixer.h"

static Return main_menu_event_loop(Drawable drawables[], int drawable_count);

Return main_menu_loop(SDL_Renderer* renderer)
{
	char *layout_file = "resources/layouts/main_menu.layout";

	/* BLOCK START */
	int drawable_count = count_valid_drawables(layout_file);
	if(0 == drawable_count)
	{
		fprintf(stderr, "%s is not a valid layout file.\n", layout_file);
		return QUIT_PROGRAM;
	}
	
	Drawable drawables[drawable_count];
	load_drawables(renderer, &drawables, layout_file);
	/* BLOCK END */

    Mix_Music* chiptune = Mix_LoadMUS("resources/audio/music/Super_Locomotive.ogg");
    Mix_PlayMusic(chiptune, -1);
	
	Return status = NORMAL;
	while(NORMAL == status)
	{
		/* If there are events in the event queue, process them. */
		status = main_menu_event_loop(drawables, drawable_count);
		
		/* Fill the screen with the background color. */
		SDL_RenderClear(renderer);
		
		/* Copy all the scalables to the window. */
		for(int i = 0; i < drawable_count; i++)
		{
			SDL_RenderCopy(renderer, drawables[i].texture, NULL, drawables[i].rect);
		}
			   
		/* Draw the renderer. */
		SDL_RenderPresent(renderer);
	}
	
	/* Clean up and return to the main function. */
	for(int i = 0; i < drawable_count; i++)
	{
		free(drawables[i].name);
		SDL_DestroyTexture(drawables[i].texture);
	}
	Mix_HaltMusic();
	Mix_FreeMusic(chiptune);
	return status;
}

static Return main_menu_event_loop(Drawable drawables[], int drawable_count)
{
	SDL_Event event; 
	while(1 == SDL_PollEvent(&event))
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
					
					/* If the click is within the exit button boundry. */
					for(int i = 0; i < drawable_count; i ++)
					{
						if(bounded_by(x, y, drawables[i].rect))
						{
							int index = get_function_index(drawables[i].name);
							switch(index)
							{
								case 0: /* fun_quit() */
									return function_pointers[index](0);
								case 1:	/* fun_options() */
									return function_pointers[index](0);
								case 2: /* fun_print() */
									function_pointers[index](1, "Click not captured");
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
