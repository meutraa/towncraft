#include "main_menu.h"
#include "drawable.h"
#include "file.h"
#include "constants.h"
#include "math.h"
#include "status.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

static Return main_menu_event_loop(SDL_Renderer* renderer, Drawable drawables[]);

static float win_scale = 1.0f;

Return main_menu_loop(SDL_Renderer* renderer)
{
	/* Create our texture arrays. */
	int texture_count = 1;
	int drawable_count = 3;
	SDL_Texture* textures[texture_count];
	Drawable drawables[drawable_count];
	
	char *layout_file = "resources/layouts/main_menu.layout";
	Return valid = is_valid_layout(layout_file);
	if(NORMAL != valid)
	{
		fprintf(stderr, "%s is not a valid layout file.\n", layout_file);
		return QUIT_PROGRAM;
	}
	load_drawables_unchecked(renderer, &textures, texture_count, &drawables, drawable_count, layout_file);

    Mix_Music* chiptune = Mix_LoadMUS("resources/audio/music/Super_Locomotive.ogg");
    Mix_PlayMusic(chiptune, -1);
	
	Return status;
	while(1)
	{
		/* If there are events in the event queue, process them. */
		status = main_menu_event_loop(renderer, drawables);
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

static Return main_menu_event_loop(SDL_Renderer* renderer, Drawable drawables[])
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
					if(bounded_by(x, y, drawables[1].rect)) return QUIT_PROGRAM;
					if(bounded_by(x, y, drawables[0].rect)) return SWITCHTO_OPTIONS;
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
