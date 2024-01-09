#include "_graph/_graph__opengl.h"
#include "_utils/_utils_file.h"
#include "_nodes/_node_root.h"

#include <GLFW/glfw3.h>

// En pixels...
extern int   Renderer_width;
extern int   Renderer_height;

void   Renderer_initWithWindow(GLFWwindow* window, const char* font_path, 
                               const char* font_name);
void   Renderer_drawView(GLFWwindow* window, Root* root);