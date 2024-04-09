#include "coq_graph.h"
#include "coq_utils.h"
#include "coq_nodes.h"
#include <SDL2/SDL.h>
#include <SDL_image.h> // <SDL2_image/SDL_image.h> ?
#include <SDL_ttf.h>   // <SDL2_ttf/SDL_ttf.h>

// En pixels...
extern int   Renderer_width;
extern int   Renderer_height;

void   Renderer_initWithWindow(SDL_Window* window, const char* font_path, 
                               const char* font_name);
void   Renderer_drawView(SDL_Window* window, Root* root);