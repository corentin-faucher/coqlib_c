#include "_graph__opengl.h"
#include "_utils_file.h"

#include <GLFW/glfw3.h>

void         Renderer_initWithWindow(GLFWwindow* window);
void         Renderer_drawView(GLFWwindow* window);
void         Renderer_resize(GLFWwindow* window, int width, int height);
GLuint const Renderer_glProgram(void);
