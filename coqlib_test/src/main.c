#include "renderer.h"
#include <unistd.h>

/*-- Callbacks d'events  -------------*/
static void error_callback(int error, const char* description)
{
    printerror("%s", description);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
static void cursor_position_callback(GLFWwindow* window, double x, double y) {
  if(glfwGetWindowAttrib(window, GLFW_HOVERED))
    printf(" 🐔 Cursor at %f, %f.\n", x, y);
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    printf(" 🐷 Mouse press.\n");
}



int main(int argc, char** argv) {
    // Init GLFW
    glfwSetErrorCallback(error_callback);
    if(!glfwInit()) { return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // Init Window et event callbacks
    GLFWwindow* window = glfwCreateWindow(800, 600, "Testing GLFW", NULL, NULL);
    if(window == NULL) { 
        glfwTerminate();
        printf("failed to create window.\n");
        return -1; 
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, Renderer_resize);
    // Init renderer/OpenGL
    Renderer_initWithWindow(window);
    

    // Boucle principale...
    while (!glfwWindowShouldClose(window)) {
        Renderer_drawView(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
