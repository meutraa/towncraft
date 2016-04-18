#include "main_menu.h"
#include "math.h"
#include "scalable.h"

static int main_menu_event_loop(SDL_Window* window, SDL_Renderer* renderer, Drawable drawables[]);

static char* texture_paths[] = {
	"resources/button.pnm"    // 0
};
static int drawables_length;

static float win_scale = 1.0f;

int main_menu_loop(SDL_Renderer* renderer, SDL_Window* window)
{
	/* Create our texture arrays. */
	int textures_count = LENGTH(texture_paths);
	struct SDL_Texture* textures[textures_count];
	
	int errors = load_textures(renderer, textures, texture_paths, textures_count);
	if(0 != errors)
	{
		printf("Errors loading textures:%d\n", errors);
	}

	Drawable options = create_drawable(1000, 600, textures[0]);
	Drawable exit    = create_drawable(1150, 600, textures[0]);

	Drawable drawables[] = {
		options,
		exit,
	};
	drawables_length = LENGTH(drawables);
    
    SDL_Color text_color = {255,255,255,0};
    Drawable test_text = create_text_drawable(renderer, 50, 50, "balls are tasty", "resources/fonts/DejaVuSans.ttf", 16, text_color);
    
    Mix_Music* chiptune = Mix_LoadMUS("resources/audio/music/Super_Locomotive.ogg");
    Mix_PlayMusic(chiptune, -1);
    
	while(1)
	{
		/* If there are events in the event queue, process them. */
		int status = main_menu_event_loop(window, renderer, drawables);
		if(0 != status)
		{
			break;
		}
			   
		/* Fill the screen with the background color. */
		SDL_RenderClear(renderer);
		
		/* Copy all the scalables to the window. */
		for(int i = 0; i < drawables_length; i++)
		{
			SDL_RenderCopy(renderer, drawables[i].texture, NULL, &drawables[i].rect);
		}
        
        SDL_RenderCopy(renderer, test_text.texture, NULL, &test_text.rect);

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

static int main_menu_event_loop(SDL_Window* window, SDL_Renderer* renderer, Drawable drawables[])
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
					if(bounded_by(x, y, drawables[1].rect)) return 1;
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
	return 0;
}
