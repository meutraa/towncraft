#include "main_menu.h"
#include "drawable.h"
#include "file.h"
#include "constants.h"
#include "math.h"
#include "status.h"
#include "functions.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

static Return main_menu_event_loop(SDL_Renderer* renderer, Drawable drawables[], int drawable_count);

static float win_scale = 1.0f;

Return main_menu_loop(SDL_Renderer* renderer)
{
	char *layout_file = "resources/layouts/main_menu.layout";

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

    Mix_Music* chiptune = Mix_LoadMUS("resources/audio/music/Super_Locomotive.ogg");
    Mix_PlayMusic(chiptune, -1);
	
	Return status;
	while(1)
	{
		/* If there are events in the event queue, process them. */
		status = main_menu_event_loop(renderer, drawables, drawable_count);
		if(QUIT_PROGRAM == status||SWITCHTO_OPTIONS == status)
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
	Mix_HaltMusic();
	Mix_FreeMusic(chiptune);
	return status;
}

static Return main_menu_event_loop(SDL_Renderer* renderer, Drawable drawables[], int drawable_count)
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
				case 80:    // LEFT - shrink ui scale
					win_scale -= 0.05f;
					SDL_RenderSetScale(renderer, win_scale, win_scale);
					break;
				case 79:    // RIGHT - grow ui scale
					win_scale += 0.05f;
					SDL_RenderSetScale(renderer, win_scale, win_scale);
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
					if(bounded_by(x, y, drawables[1].rect))
					{
						for(int j = 0; j < FUNCTION_COUNT; j++)
						{
							if(0 == strncmp(drawables[1].name, function_strings[j], strlen(function_strings[j])))
							{
								int status = (*function_pointers)[j](0);
								if(-1 == status) return QUIT_PROGRAM;
							}
						}
					}
					else
					{
						int ind = getfunctionindex();
						if(-1 != ind) ;
					}
					//if(bounded_by(x, y, drawables[0].rect)) return SWITCHTO_OPTIONS;
					break;
				}
			}
			break;
		case SDL_WINDOWEVENT:
			switch(event.window.event)
			{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					float ratio_w = (float) event.window.data1 / (float) DESIGN_WIDTH;
					float ratio_h = (float) event.window.data2 / (float) DESIGN_HEIGHT;
					float ratio = MIN(ratio_w, ratio_h)*win_scale;
					SDL_RenderSetScale(renderer, ratio, ratio);
				}
				default:
					break;
			}
		default:
			break;
		}
	}
	return NORMAL;
}
