// Version glfw d'un main pour coqlib_test.
//
// Corentin Faucher
//
#include "coqlib_gl.h"

#include "renderer.h"
#include "my_root.h"
#include "my_enums.h"

#ifdef __APPLE__
#define FONT_PATH "/System/Library/Fonts"
#define FONT_NAME "Supplemental/Chalkboard.ttc"// "Courier.ttc"
#endif
#ifdef __linux__
// ** ICI, choisir une font qui existe... **
#define FONT_PATH "/usr/share/fonts/truetype/ubuntu"
#define FONT_NAME "Ubuntu-B.ttf"
#endif

#define FRAME_TIMEMS 10
#define WINDOW_WIDTHPX 1200
#define WINDOW_HEIGHTPX 768

/// La fonction `update` de la boucle principale.
/// Retourne false si on doit sortir.
/// (normalement dans une thread separee...)
static bool main_checkUp(ChronoChecker* cc) {
    // Set le temps de la tic.
    ChronoEvent_update();
    // Updates prioritaires
    // 1. Les events en premiers.
    CoqEvent_processAllQueuedRootEvents();
    // 2. Ensuite les callback des timers.
    Timer_check();
    // 3. Ménage de la structure.
    NodeGarbage_burn();
    // 4. Event pour la window (juste terminate pour l'instant)
    for(CoqEventWin event = CoqEvent_getNextTodoWindowEvent(); event.type;
        event = CoqEvent_getNextTodoWindowEvent()) {
        if(event.type == eventtype_win_terminate) return false;
    }
    
    // Check optionnels
    // (en fait c'est juste de dessiner les textures en pleine grandeur)
    if(chronochecker_elapsedMS(*cc) > ChronosEvent.deltaTMS) {
        // Reste déjà plus de temps ?
        // Ici on laisse faire le warning, on est déjà dans la même thread que l'affichage...
//        printwarning("Overwork?");
        return true;
    }
    Texture_checkToFullyDrawAndUnused(cc, ChronosEvent.deltaTMS - 5);
    return true;
}

int main(int argc, char** argv) {
    // Init system, langue...
    CoqSystem_init();
    // Inits GLFW
    glfwSetErrorCallback(CoqGlfw_error_callback);
    if(!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    char const* app_name = CoqSystem_appDisplayNameOpt() ? 
        CoqSystem_appDisplayNameOpt() : "Testing OpenGL";
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTHPX, WINDOW_HEIGHTPX, 
                            app_name, NULL, NULL);
    if(window == NULL) {
        glfwTerminate();
        printf("⚠️ Failed to create window.\n");
        return -1;
    }
    glfwMakeContextCurrent(window);
    // Init de OpenGL dans linux... (après le context GLFW)
    #ifdef __linux__
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    	glfwTerminate();
        printf("⚠️ Glad failed.\n");
        return -1;
    }
    #endif

    // Autre inits...
    CoqGlfw_Init(window); // Set window size. Callback d'events : mouse, keyboard, resize.
    CoqGraph_opengl_init(NULL, NULL); // Shaders, texture, mesh.
    CoqFont_freetype_init(FONT_PATH, FONT_NAME); // Fonts.

    // Chargement des resources du projet (png, wav)
    Texture_loadCoqlibPngs();
    Texture_loadProjectPngs(MyProject_pngInfos, png_total_pngs);
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds, 0);
    // Init app structure
    CoqEvent_root = Root_initAndGetProjectRoot();

    // Unpause, refresh rate
    // Ici, on va juste updater les events/timers en même temps que les frames du renderer,
    // i.e. au 16 ms (normalement on met à jour aux 50 ms... ?).
    ChronoApp_setPaused(false);
    ChronoEvent_setTicDeltaT(FRAME_TIMEMS);

    // Boucle principale
    ChronoChecker cc;
    while(!glfwWindowShouldClose(window)) 
    {
        cc = chronochecker_startNew();
        // Events
        glfwPollEvents();
        if(!main_checkUp(&cc)) {
            glfwSetWindowShouldClose(window, 1);
        }
        // Affichage
        Renderer_drawView();
        glfwSwapBuffers(window);
        
        // Sleep s'il reste du temps. (16 ms par frame)
        int64_t sleepDeltaT = FRAME_TIMEMS - chronochecker_elapsedMS(cc);
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
