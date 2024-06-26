#include "renderer.h"
#include "my_root.h"
#include "my_enums.h"
#include "coq_sound.h"
#include "coq_event.h"
#ifdef __APPLE__
#define FONT_PATH "/Library/Fonts"
#define FONT_NAME "FiraCode-Medium"
#endif
#ifdef __linux__
#define FONT_PATH "/usr/share/fonts/truetype/freefont"
#define FONT_NAME "FreeSans"
#endif

// #define GL_MAJOR_VERSION_ 3 // 2
// #define GL_MINOR_VERSION_ 0 // 1

/// Structure de l'app.
static Root* root = NULL;

/*-- Callbacks d'events  -------------*/
static void srcoll_callback(double delta_x, double delta_y) {
    CoqEvent coq_event = {
      .type = event_type_scroll,
      .scroll_info = {
        .scrollType = scroll_type_scroll,
        .scroll_deltas = {{ delta_x, delta_y }}
      }
    };
    CoqEvent_addToRootEvent(coq_event);
}

static void resize_callback(SDL_Window* window, int widthPx, int heightPx) {
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
  Renderer_drawView(window, root);
}
static bool MouseLeftDown_ = false;
static void cursor_position_callback(double x, double y) {
  if(!root) { printwarning("No root"); return; }
  Vector2 viewPos = {{ x, y }};
  // !! Ici y est inversé !!
  CoqEvent coq_event = {
      .type = MouseLeftDown_ ? event_type_touch_drag : event_type_touch_hovering, 
      .touch_pos = root_absposFromViewPos(root, viewPos, true)
  };
  CoqEvent_addToRootEvent(coq_event);
}
static void mouse_button_callback(int x, int y) {
  if(!root) { printwarning("No root"); return; }
  MouseLeftDown_ = true;
  Vector2 viewPos = {{ x, y }};
  // !! Ici y est inversé !!
  CoqEvent coq_event = {
      .type = event_type_touch_down, 
      .touch_pos = root_absposFromViewPos(root, viewPos, true)
  };
  CoqEvent_addToRootEvent(coq_event);
}
static void mouse_button_up_callback(int x, int y) {
  if(!root) { printwarning("No root"); return; }
  MouseLeftDown_ = false;
  CoqEvent coq_event = {
      .type = event_type_touch_up,
  };
  CoqEvent_addToRootEvent(coq_event);
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
    // Init SDL
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printerror("SDL_Init error");
        return -1;
    }
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
    // Init Window
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_MAJOR_VERSION_);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_MINOR_VERSION_);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Window* window = SDL_CreateWindow("Testing SDL...",
                                          100, 100,
                                          600, 480,
                                          SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if(!window) {
        printerror("No window init.");
        return -1;
    }
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if(!context) {
        printerror("No context opengl.");
        return -1;
    }
#ifdef __linux__
    // Une fois le context init...
    glewInit();
#endif
    printdebug("GL version %s.", glGetString(GL_VERSION));
    printdebug("GLSL version %s.", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Init renderer/OpenGL
    Renderer_initWithWindow(window, FONT_PATH, FONT_NAME);

    // Chargement des resources du projet (png, wav, localized string)
    Texture_init(MyProject_pngInfos, png_total_pngs, true);
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds);

    Language_init();

    // Init app structure (voir my_root.c)
    root = Root_initAndGetProjectRoot();
    
    // Unpause, refresh rate, set view size...
    ChronoApp_setPaused(false);
    Chrono_UpdateDeltaTMS = 16;
    resize_callback(window, 600, 480);

    // Boucle principale...
    bool run = true;
    ChronoChecker cc;
    while (run) {
        chronochecker_set(&cc);
        // Events de la window...
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
          switch(event.type) {
          case SDL_QUIT: run = false; break;
          case SDL_KEYDOWN: {
              if(event.key.keysym.sym == SDLK_ESCAPE)
                run = false;
            } break;
          case SDL_WINDOWEVENT: {
            if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
              resize_callback(window, event.window.data1, event.window.data2);
            }
          } break;
          case SDL_MOUSEMOTION: {
              cursor_position_callback(event.motion.x, event.motion.y);
          } break;
          case SDL_MOUSEBUTTONDOWN: {
              if(event.button.button != SDL_BUTTON_LEFT) break;
              mouse_button_callback(event.button.x, event.button.y);
          } break;
          case SDL_MOUSEBUTTONUP: {
              if(event.button.button != SDL_BUTTON_LEFT) break;
              mouse_button_up_callback(event.button.x, event.button.y);
          } break;
          case SDL_MOUSEWHEEL: {
          srcoll_callback(event.wheel.x, event.wheel.y);
          } break;
          }
        }

        // Timers, process events, generate texture, ...
        if(!main_checkUp(root, &cc))
            run = false;
        // Draw frame.
        Renderer_drawView(window, root);
        // Sleep s'il reste du temps.
        int64_t sleepDeltaT = Chrono_UpdateDeltaTMS - chronochecker_elapsedMS(&cc);
        if(sleepDeltaT < 1) sleepDeltaT = 1;
        struct timespec time = {0, sleepDeltaT*ONE_MILLION};
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


/*
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
*/