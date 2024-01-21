#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
#ifdef __linux__
#include <GL/glew.h>
#endif

#include <SDL2/SDL.h>
#include <SDL_image.h> // <SDL2_image/SDL_image.h> ?
#include <SDL_ttf.h>   // <SDL2_ttf/SDL_ttf.h>
#include <time.h>
#include <stdbool.h>

#include "coq_math.h"
#include "coq_graph.h"

#define ONE_MILLION 1000000


int main(int argc, char** argv) {
    Vector3 a = {{ 1, 2, 3 }};
    Vector3 b = {{ 2, 3, 4 }};
    float p = vector3_dot(a, b);

    printdebug("dot product %f.", p);

    // Init SDL
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init error");
        return -1;
    }
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
    // Init Window
    SDL_Window* window = SDL_CreateWindow("Testing SDL...",
                                            100, 100,
                                            600, 480,
                                            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    Texture_init(0, "res/fonts", "arial");
    

    // Boucle principale...
    bool run = true;
    while (run) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
          switch(event.type) {
          case SDL_QUIT:
            run = false; break;
          case SDL_KEYDOWN: {
              if(event.key.keysym.sym == SDLK_ESCAPE)
                run = false;
              break;
            }
          }
          if(event.type == SDL_QUIT)
            run = false;
        }
        // Draw
        glClearColor(0, 0.5, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);

        // Sleep s'il reste du temps.
        struct timespec time = {0, 15*ONE_MILLION};
        nanosleep(&time, NULL);
    }

    // Deinits...
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}