// Version glfw d'un main pour coqlib_test.
//
// Corentin Faucher
//
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>

#include "coq_timer.h"
#include "coq_sound.h"
#include "coq_event.h"
#include "graphs/graph_base.h"
#include "graphs/graph_texture.h"
#include "graphs/graph__opengl.h"
#include "maths/math_base.h"
#include "maths/math_chrono.h"
#include "nodes/node_base.h"
#include "nodes/node_root.h"
#include "utils/util_base.h"
#include "utils/util_system.h"

#include "renderer.h"
#include "my_root.h"
#include "my_enums.h"

#ifdef __APPLE__
#define FONT_PATH "/Library/Fonts"
#define FONT_NAME "FiraCode-Medium"
#endif
#ifdef __linux__
#define FONT_PATH "/usr/share/fonts/truetype/freefont"
#define FONT_NAME "FreeSans"
#endif

/// Structure de l'app.
static Root* root = NULL;

/*-- Callbacks d'events  -------------*/
void glfw_error_callback_(int const error, const char* const description) {
    printf("⚠️ Error: %s.\n", description);
}
void glfw_key_callback_(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if( action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
        return;
    }
    // Le "scancode" de GLFW semble etre le keycode de l'OS.
    // Le "key" de GLFW semble etre la correspondance
    // entre le Qwerty-US et les ASCII,
    // e.g. la touch "A" (sur qwerty) est key = 65.
    // Ici, on va garder les "scancodes..."
    uint32_t modifiers = 0;
    if(mods & GLFW_MOD_SHIFT) modifiers |= modifier_shift;
    if(mods & GLFW_MOD_ALT) modifiers |= modifier_option;
    if(mods & GLFW_MOD_SUPER) modifiers |= modifier_command;
    if(mods & GLFW_MOD_CONTROL) modifiers |= modifier_control;
    if(mods & GLFW_MOD_CAPS_LOCK) modifiers |= modifier_capslock;
    CoqEvent event = {
        .type = event_type_key_down,
        .key = {
            .modifiers = modifiers,
            .keycode = scancode,
        },
    };
    CoqEvent_addToRootEvent(event);
}
void glfw_scroll_callback_(GLFWwindow* window, double xoffset, double yoffset) {
    CoqEvent coq_event = {
      .type = event_type_scroll,
      .scroll_info = {
        .scrollType = scroll_type_scroll,
        .scroll_deltas = {{ xoffset, yoffset }}
      }
    };
    CoqEvent_addToRootEvent(coq_event);
}
void glfw_framebufferSize_callback_(GLFWwindow* window, int widthPx, int heightPx)  {
  Renderer_width = widthPx;
  Renderer_height = heightPx;
  if(!root) { printwarning("No root"); return; }
  // int widthPt, heightPt;
  // glfwGetWindowSize(window, &widthPt, &heightPt);
  CoqEvent_addToRootEvent((CoqEvent){
        event_type_resize, .resize_info = {
            { 0, 0, 0, 0 },
            {{ 0, 0, widthPx, heightPx }}, {{ widthPx, heightPx }},
            false, false, false,
  }});
  // GLFW semble bloquer la boucle principale durant un resize ???
  // Il faut donc checker les event et dessiner ici... ??
  CoqEvent_processEvents(root);
  Renderer_drawView(root);
}
static bool MouseLeftDown_ = false;
static Vector2 mousePos = {};
void glfw_cursorPos_callback_(GLFWwindow* window, double x, double y) {
  if(!root) { printwarning("No root"); return; }
  // !! Ici y est inversé !!
  mousePos = root_absposFromViewPos(root, (Vector2){{ x, y }}, true);
  CoqEvent_addToRootEvent((CoqEvent){
      .type = MouseLeftDown_ ? event_type_touch_drag : event_type_touch_hovering,
      .touch_pos = mousePos,
  });
}
static void glfw_mouseButton_callback_(GLFWwindow* window, int button, int action, int mods) {
    if(!root) { printwarning("No root"); return; }
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    MouseLeftDown_ = action; // GLFW_RELEASE = 0
    if(action == GLFW_PRESS) {
        CoqEvent_addToRootEvent((CoqEvent){
            .type = event_type_touch_down,
            .touch_pos = mousePos,
        });
    } else if(action == GLFW_RELEASE) {
        CoqEvent_addToRootEvent((CoqEvent){
            .type = event_type_touch_up,
            // .touch_pos = mousePos, // superflu pour touchUp.
        });
    }
}

/// La fonction `update` de la boucle principale.
/// Retourne false si on doit sortir.
/// (normalement dans une thread separee...)
static bool main_checkUp(Root* root, ChronoChecker* cc) {
    if(root == NULL) { return false; }
    if(root->shouldTerminate) { return false; }


    // Updates prioritaires
    CoqEvent_processEvents(root);
    Timer_check();
    NodeGarbage_burn();

    // Check optionnels
    // (en fait c'est juste de dessiner les textures en pleine grandeur)
    if(chronochecker_elapsedMS(cc) > Chrono_UpdateDeltaTMS) {
        // Pas le temps pour les autres checks ??
        // printwarning("Overwork?");
        return true;
    }
    Texture_checkToFullyDrawAndUnused(cc, Chrono_UpdateDeltaTMS - 5);

    return true;
}

int main(int argc, char** argv) {
    // Inits...
    glfwSetErrorCallback(glfw_error_callback_);
    if(!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Testing OpenGL", NULL, NULL);
    if(window == NULL) {
        glfwTerminate();
        printf("⚠️ Failed to create window.\n");
        return -1;
    }
    glfwMakeContextCurrent(window);
    #ifdef __linux__
        // Une fois le context init...
        glewInit();
    #endif
    // Les callback d'events pour glfw...
    glfwSetKeyCallback(window, glfw_key_callback_);
    glfwSetScrollCallback(window, glfw_scroll_callback_);
    // glfwSetWindowSizeCallback(window, glfw_windowSize_callback_); -> coordinates (pas pixels)
    glfwSetFramebufferSizeCallback(window, glfw_framebufferSize_callback_);
    glfwSetCursorPosCallback(window, glfw_cursorPos_callback_);
    glfwSetMouseButtonCallback(window, glfw_mouseButton_callback_);

    // Autre inits...
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
    CoqSystem_init();
    CoqGraph_opengl_init(true);
    CoqFont_sdlttf_setFontsPath(FONT_PATH, FONT_NAME);

    // Chargement des resources du projet (png, wav, localized string)
    Texture_loadProjectPngs(MyProject_pngInfos, png_total_pngs);
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds);
    // Init app structure (voir my_root.c)
    root = Root_initAndGetProjectRoot();

    // Unpause, refresh rate, set view size...
    glfwGetFramebufferSize(window, &Renderer_width, &Renderer_height);
    ChronoApp_setPaused(false);
    Chrono_UpdateDeltaTMS = 16;

    // Boucle principale
    ChronoChecker cc;
    while(!glfwWindowShouldClose(window)) {
        chronochecker_set(&cc);
        // Affichage
        Renderer_drawView(root);
        glfwSwapBuffers(window);
        // Events
        glfwPollEvents();
        // Timers, process events, generate texture, ...
        if(!main_checkUp(root, &cc)) {
            glfwSetWindowShouldClose(window, 1);
        }
        // Sleep s'il reste du temps.
        int64_t sleepDeltaT = Chrono_UpdateDeltaTMS - chronochecker_elapsedMS(&cc);
        if(sleepDeltaT < 1) sleepDeltaT = 1;
        struct timespec time = {0, sleepDeltaT*ONE_MILLION};
        nanosleep(&time, NULL);
    }

    // Clean up
    CoqFont_sdlttf_quit();
    CoqGraph_opengl_deinit();
    IMG_Quit();
    TTF_Quit();
    glfwDestroyWindow(window);
    glfwTerminate();
}
