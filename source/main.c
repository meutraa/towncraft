#include "game.h"
#include "options.h"

#include <GLFW/glfw3.h>
#include "constants.h"

int main(void)
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(resolution_width, resolution_height, GAME_NAME, NULL, NULL);
    glfwMakeContextCurrent(window);

    glClearColor(0, 0, 0, 1);
    glShadeModel(GL_FLAT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DITHER);
    glDisable(GL_TEXTURE_2D);

    /* Start the main loop. */
    while(game_loop(window));

    glfwTerminate();
    return 0;
}
