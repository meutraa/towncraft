#include "game.h"
#include "options.h"

#include <GLFW/glfw3.h>
#include "constants.h"

int main(void)
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(resolution_width, resolution_height, GAME_NAME, NULL, NULL);
    glfwMakeContextCurrent(window);

    /* Start the main loop. */
    while(game_loop(window));

    glfwTerminate();
    return 0;
}
