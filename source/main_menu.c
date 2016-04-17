#include "main_menu.h"
#include "math.h"
#include "scalable.h"

static int main_menu_event_loop(SDL_Window* window, SDL_Texture* textures[], Scalable* scalables);

static char* texture_paths[] = {
	"resources/button.pnm"    // 0
};
static int scalables_length;

static float win_scale = 1.0f;

int main_menu_loop(SDL_Renderer* renderer, SDL_Window* window)
{
	/* Save a reference to our window as global. */
	
	/* Create our texture arrays. */
	int textures_count = LENGTH(texture_paths);
	struct SDL_Texture* textures[textures_count];
	
	int errors = load_textures(renderer, textures, texture_paths, textures_count);
	if(0 != errors)
	{
		printf("Errors loading textures:%d\n", errors);
	}

	Scalable scalables[] = {
		 create_scalable(0.8, 0.9, textures, 0),
		 create_scalable(0.9, 0.9, textures, 0),
	};
	scalables_length = LENGTH(scalables);
	
	/* Initial scaling. */
	resize_scalables(window, textures, scalables, scalables_length, win_scale);
	
	while(1)
	{
		/* If there are events in the event queue, process them. */
		int status = main_menu_event_loop(window, textures, scalables);
		if(0 != status)
		{
			break;
		}
			   
		/* Fill the screen with the background color. */
		SDL_RenderClear(renderer);
		
		/* Copy all the scalables to the window. */
		for(int i = 0; i < 2; i++)
		{
			SDL_RenderCopy(renderer, textures[scalables[i].texture_id], NULL, &scalables[i].rect);
		}
		
		/* Draw the renderer. */
		SDL_RenderPresent(renderer);
	}
	
	/* Clean up and end the main function. */
	for(int i = 0; i < textures_count; i++)
	{
		SDL_DestroyTexture(textures[i]);
	}
	return 1;
}

static int main_menu_event_loop(SDL_Window* window, SDL_Texture* textures[], Scalable scalables[])
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
					return 1;
					break;
				case 80:    // LEFT - shrink ui scale
					win_scale -= 0.05f;
					resize_scalables(window, textures, scalables, scalables_length, win_scale);
					break;
				case 79:    // RIGHT - grow ui scale
					win_scale += 0.05f;
					resize_scalables(window, textures, scalables, scalables_length, win_scale);
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
					if(bounded_by(x, y, scalables[1].rect)) return 1;
					break;
				}
			}
			break;
		case SDL_WINDOWEVENT:
			switch(event.window.event)
			{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					resize_scalables(window, textures, scalables, scalables_length, win_scale);
				}
				default:
					break;
			}
		default:
			break;
		}
	}
	return 0;
}
