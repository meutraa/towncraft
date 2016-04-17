#include "SDL.h"
#include "SDL_image.h"

/* Function declarations. */
void main_menu_event_loop();

void resize_scalables();

/* Free up memory. */
void quit();

#define GAME_NAME "Towncraft"
#define SCA_COUNT 2     // Number of Scalables
#define TEX_COUNT 1     // Number of Textures

#define FALLBACK_RES_WIDTH 640
#define FALLBACK_RES_HEIGHT 480