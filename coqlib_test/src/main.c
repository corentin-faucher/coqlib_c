#include "renderer.h"
#include "my_root.h"
#include "my_enums.h"
#include "coq_sound.h"
#include "coq_event.h"
#include <SDL2/SDL.h>
#include <SDL_image.h> // <SDL2_image/SDL_image.h> ?
#include <SDL_ttf.h>   // <SDL2_ttf/SDL_ttf.h>

#define FONT_PATH "/Library/Fonts"
#define FONT_NAME "FiraCode-Medium"

/// Structure de l'app.
static Root* root = NULL;

/*-- Callbacks d'events  -------------*/
static void error_callback(int error, const char* description)
{
    printerror("%s", description);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if( action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GL_TRUE);
        return;
    }
    // Le "scancode" de GLFW semble etre le keycode de l'OS.
    // Le "key" de GLFW semble etre un correspondance 
    // entre le Qwerty-US et les ASCII, 
    // e.g. la touch "A" (sur qwerty) est key = 65.
    // Ici, on va garder les "scancodes..."
    CoqEvent coqEvent;
    coqEvent.flags = event_type_keyDown;
    coqEvent.key.keycode = scancode;
    // Correspondance des modifiers...
    uint32_t modifiers = 0;
    if(mods & GLFW_MOD_SHIFT) modifiers |= modifier_shift;
    if(mods & GLFW_MOD_ALT) modifiers |= modifier_option;
    if(mods & GLFW_MOD_SUPER) modifiers |= modifier_command;
    if(mods & GLFW_MOD_CONTROL) modifiers |= modifier_control;
    if(mods & GLFW_MOD_CAPS_LOCK) modifiers |= modifier_capslock;
    coqEvent.key.modifiers = modifiers;
    coqEvent.key.isVirtual = false;
    CoqEvent_addEvent(coqEvent);
}
static void cursor_position_callback(GLFWwindow* window, double x, double y) {
  if(!glfwGetWindowAttrib(window, GLFW_HOVERED))
    return;
  if(!root) { printwarning("No root"); return; }
  // !! Ici y est inversé !!
  glfwGetCursorPos(window, &x, &y);
  Vector2 viewPos = {{ x, y }};

  CoqEvent coq_event;
  coq_event.flags = event_type_hovering;
  coq_event.touch_pos = root_absposFromViewPos(root, viewPos, true);
  CoqEvent_addEvent(coq_event);
}
static void srcoll_callback(GLFWwindow* window, double delta_x, double delta_y) {
    CoqEvent coq_event;
    coq_event.flags = event_type_scroll;
    coq_event.scroll_deltas = (Vector2) {{ delta_x, delta_y }};
    CoqEvent_addEvent(coq_event);
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  if(!root) { printwarning("No root"); return; }
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double x, y;
    // !! Ici y est inversé !!
    glfwGetCursorPos(window, &x, &y);
    Vector2 viewPos = {{ x, y }};

    CoqEvent coq_event;
    coq_event.flags = event_type_down;
    coq_event.touch_pos = root_absposFromViewPos(root, viewPos, true);
    CoqEvent_addEvent(coq_event);
    return;
  }
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    CoqEvent coq_event;
    coq_event.flags = event_type_up;
    CoqEvent_addEvent(coq_event);
    return;
  }
}
static void resize_callback(GLFWwindow* window, int widthPx, int heightPx) {
  Renderer_width = widthPx;
  Renderer_height = heightPx;
  if(!root) { printwarning("No root"); return; }
  int widthPt, heightPt;
  glfwGetWindowSize(window, &widthPt, &heightPt);
  CoqEvent coq_event;
  coq_event.flags = event_type_resize;
  coq_event.resize_margins = (Margins) { 0, 0, 0, 0 };
  coq_event.resize_sizesPt = (Vector2) {{ widthPt, heightPt }};
  coq_event.resize_sizesPix = (Vector2) {{ widthPx, heightPx }};
  coq_event.resize_inTransition = false;
  CoqEvent_addEvent(coq_event);

  // GLFW semble bloquer la boucle principale durant un resize ???
  // Il faut donc checker les event et dessiner ici... ??
  CoqEvent_processEvents(root);
  Renderer_drawView(window, root);
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
    if(_chronochecker_elapsedMS(cc) > Chrono_UpdateDeltaTMS) {
        // Pas le temps pour les autres checks ??
        printwarning("Overwork?"); return true;
    }
    _Texture_checkToFullyDrawAndUnused(cc, Chrono_UpdateDeltaTMS - 5);

    return true;
}




int main(int argc, char** argv) {
    // Mettre a 60 fps.
    Chrono_UpdateDeltaTMS = 16;

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

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
    glfwSetScrollCallback(window, srcoll_callback);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    // Init renderer/OpenGL
    Renderer_initWithWindow(window, FONT_PATH, FONT_NAME);

    // Chargement des resources du projet (png, wav, localized string)
    Texture_loadPngs(MyProject_pngInfos, png_total_pngs);
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds);
    Language_init();
    // Font_init();

    // Init root (voir my_root.c)
    root = Root_createMyRoot();
    // Unpause, et set view size
    ChronoApp_setPaused(false);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    resize_callback(window, width, height);

    // Boucle principale...
    bool run = true;
    ChronoChecker cc;
    while (run) {
        _chronochecker_set(&cc);
        glfwPollEvents();
        run = main_checkUp(root, &cc) && !glfwWindowShouldClose(window);
        Renderer_drawView(window, root);

        // Sleep s'il reste du temps.
        int64_t sleepDeltaT = Chrono_UpdateDeltaTMS - _chronochecker_elapsedMS(&cc);
        if(sleepDeltaT < 1) sleepDeltaT = 1;
        struct timespec time = {0, sleepDeltaT*ONE_MILLION};
        nanosleep(&time, NULL);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
