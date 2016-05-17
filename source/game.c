#include <epoxy/gl.h>
#include <epoxy/glx.h>

#include "game.h"

#include "map.h"
#include "options.h"
#include "constants.h"

static GLenum render_modes[] = { GL_FILL, GL_FILL, GL_LINE, GL_POINT };
static int render_mode = 0;

static GLuint vbo_id;
static unsigned long triangle_count;

static GLfloat scale, dx, dy;

static void render_grid(GLFWwindow* window)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(2, GL_INT, 0, 0);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, (void*) (triangle_count * NUM_VERTEX * sizeof(GLint)));

    glDrawArrays(GL_TRIANGLES, 0, triangle_count * 3);

    glDisableClientState(GL_COLOR_ARRAY);

    if(render_mode % 4 == 1)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor4ub(0, 0, 0, 16);
        glDrawArrays(GL_TRIANGLES, 0, triangle_count * 3);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glfwSwapBuffers(window);
}

static void update_view(GLFWwindow* window)
{
    glLoadIdentity();
    glOrtho(dx, (DESIGN_WIDTH * scale) + dx, dy, (DESIGN_HEIGHT * scale) + dy, -1, 1);
    render_grid(window);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(GLFW_KEY_ENTER == key && action == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, render_modes[++render_mode % 4]);
        render_grid(window);
    }
    if(GLFW_KEY_N == key && action == GLFW_PRESS)
    {
        triangle_count = create_map(&vbo_id);
        render_grid(window);
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    double mouse_x, mouse_y;
    int zoomin = yoffset > 0;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    float oldscale = scale;
    scale *= zoomin ? 0.5 : 2.0f;
    dx += mouse_x * (zoomin ? scale : -oldscale);
    dy += (DESIGN_HEIGHT - mouse_y) * (zoomin ? scale : -oldscale);
    update_view(window);
}

int game_loop(GLFWwindow* window)
{
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    scale = 1.0f;
    dx = 0.0f, dy = 0.0f;

    triangle_count = create_map(&vbo_id);

    glMatrixMode(GL_PROJECTION);
    update_view(window);
    while(GLFW_PRESS != glfwGetKey(window, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float speed = scroll_speed * scale;
        int r = 0;
        if(GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)) { dx -= speed; r = 1; }
        if(GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)) { dx += speed; r = 1; }
        if(GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)) { dy += speed / 2; r = 1; }
        if(GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)) { dy -= speed / 2; r = 1; }
        if(r) update_view(window);
    }

    /* Free any allocated memory. */
    glDeleteBuffers(1, &vbo_id);
    return 0;
}
