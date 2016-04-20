#include "options_menu.h"
#include "drawable.h"
#include "options.h"
#include "math.h"
#include "file.h"
#include "constants.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

static Return options_menu_event_loop(Drawable drawables[]);

Return options_menu_loop(SDL_Renderer* renderer)
{	
	char *layout_file = "resources/layouts/options_menu.layout";

	/** BLOCK START 
	 *  This block is re-usable across layouts but needs to be here.
	 */
	/* Verify the layout file is legit. */
	Return valid = is_valid_layout(layout_file);
	if(NORMAL != valid)
	{
		fprintf(stderr, "%s is not a valid layout file.\n", layout_file);
		return QUIT_PROGRAM;
	}
	
	/* We have a well formated layout file and resources exist. */
	int drawable_count = count_resources(layout_file);
	int texture_count = count_textures(layout_file, drawable_count);
	SDL_Texture* textures[texture_count];
	Drawable drawables[drawable_count];
	load_drawables(renderer, &textures, texture_count, &drawables, drawable_count, layout_file);
	/**
	 *  BLOCK END
	 */
	
	Return status;
	while(1)
	{
		/* If there are events in the event queue, process them. */
		status = options_menu_event_loop(drawables);
		if(QUIT_PROGRAM == status||SWITCHTO_MAINMENU == status)
		{
			/* Break out of the loop and cleanup resources. */
			break;
		}
			   
		/* Fill the screen with the background color. */
		SDL_RenderClear(renderer);
		
		/* Copy all the scalables to the window. */
		for(int i = 0; i < drawable_count; i++)
		{
			//printf("%d,%d\t%d,%d\n", drawables[i].rect->w, drawables[i].rect->h, drawables[i].rect->x, drawables[i].rect->y);
			SDL_RenderCopy(renderer, drawables[i].texture, NULL, drawables[i].rect);
		}
		/* Draw the renderer. */
		SDL_RenderPresent(renderer);
	}
	
	/* Clean up and return to the main function. */
	for(int i = 0; i < drawable_count; i++)
	{
		free(drawables[i].name);
	}
	for(int i = 0; i < texture_count; i++)
	{
		SDL_DestroyTexture(textures[i]);
	}
	return status;
}

static Return options_menu_event_loop(Drawable drawables[])
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
					int time = event.button.timestamp;
					printf("Button %d pressed at %d,%d at %d\n", event.button.button, x, y, time);
					
					/* If the click is within the exit button boundry. */
					if(bounded_by(x, y, drawables[1].rect)) return QUIT_PROGRAM;
					if(bounded_by(x, y, drawables[0].rect)) return SWITCHTO_MAINMENU;
					break;
				}
			}
			break;
		default:
			break;
		}
	}
	return NORMAL;
}
