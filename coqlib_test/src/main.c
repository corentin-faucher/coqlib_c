// Version glfw d'un main pour coqlib_test.
//
// Corentin Faucher
//
#include <OpenGL/gl3.h>

#include "util_glfw.h"
#include "coq_timer.h"
#include "coq_sound.h"
#include "coq_event.h"
#include "graphs/graph_base.h"
#include "graphs/graph_texture.h"
#include "graphs/graph__opengl.h"
#include "coq_chrono.h"
#include "nodes/node_base.h"
#include "nodes/node_root.h"
#include "utils/util_system.h"

#include "renderer.h"
#include "my_root.h"
#include "my_enums.h"

#ifdef __APPLE__
#define FONT_PATH "/System/Library/Fonts"
#define FONT_NAME "Courier.ttc"
#endif
#ifdef __linux__
#define FONT_PATH "/usr/share/fonts/truetype/freefont"
#define FONT_NAME "FreeSans.ttf"
#endif

/// Structure de l'app.
static Root* root = NULL;

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
    if(chronochecker_elapsedMS(*cc) > ChronosEvent.deltaTMS) {
        // Pas le temps pour les autres checks ??
        // printwarning("Overwork?");
        return true;
    }
    Texture_checkToFullyDrawAndUnused(cc, ChronosEvent.deltaTMS - 5);
    return true;
}

int main(int argc, char** argv) {
    // Inits GLFW
    glfwSetErrorCallback(Coq_glfw_error_callback);
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
    Coq_glfw_setEventsCallbacks(window);
    #ifdef __linux__
        // Une fois le context init...
        glewInit();
    #endif

    // Autre inits...
    CoqSystem_init();
    CoqGraph_opengl_init(NULL, NULL);
    CoqFont_freetype_init(FONT_PATH, FONT_NAME);

    // Chargement des resources du projet (png, wav, localized string)
    Texture_loadCoqlibPngs();
    Texture_loadProjectPngs(MyProject_pngInfos, png_total_pngs);
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds);
    // Init app structure (voir my_root.c)
    root = Root_initAndGetProjectRoot();
    
    // Tests...
//    CoqFont_test_printAvailableFonts();

    // Unpause, refresh rate, set view size...
    glfwGetFramebufferSize(window, &CoqGLFW_viewWidthPx, &CoqGLFW_viewHeightPx);
    ChronoApp_setPaused(false);

    // Boucle principale
    ChronoChecker cc;
    while(!glfwWindowShouldClose(window)) 
    {
        cc = chronochecker_startNew();
        // Affichage
        Renderer_drawView(root);
        glfwSwapBuffers(window);
        // Events
        glfwPollEvents();
        // Timers, process events, generate texture, ...
        if(!main_checkUp(root, &cc)) {
            glfwSetWindowShouldClose(window, 1);
        }
        // Sleep s'il reste du temps. (16 ms par frame)
        int64_t sleepDeltaT = 16 - chronochecker_elapsedMS(cc);
        if(sleepDeltaT < 1) sleepDeltaT = 1;
        struct timespec time = {0, sleepDeltaT*ONE_MILLION};
        nanosleep(&time, NULL);
    }

    // Clean up
    CoqFont_freetype_quit();
    CoqGraph_opengl_deinit();
    glfwDestroyWindow(window);
    glfwTerminate();
}
