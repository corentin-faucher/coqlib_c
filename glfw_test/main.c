#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
#ifdef __linux__
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <SDL2/SDL.h>
#include <SDL_image.h> // <SDL2_image/SDL_image.h> ?
#include <SDL_ttf.h>   // <SDL2_ttf/SDL_ttf.h>

#define ONE_MILLION 1000000

/*-- Callbacks d'events  -------------*/
static void error_callback(int error, const char* description) {
    printf("%s\n", description);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if( action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GL_TRUE);
        return;
    }
}

int main(int argc, char** argv) {
    // Init SDL et GLFW
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
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

    // Boucle principale...
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Draw
        glClearColor(0, 0.5, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);

        // Sleep s'il reste du temps.
        struct timespec time = {0, 15*ONE_MILLION};
        nanosleep(&time, NULL);
    }

    // Deinits...
    glfwDestroyWindow(window);
    glfwTerminate();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}