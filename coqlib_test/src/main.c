// Exemple d'un main avec GLFW et Coqlib.
//
// Corentin Faucher
//
#include "coqlib_glfw.h"

#include "renderer.h"
#include "my_root.h"
#include "my_enums.h"

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#ifdef __linux__
// ** ICI, choisir une font qui existe... **
#define FONT_PATH "/usr/share/fonts/truetype/ubuntu"
#define FONT_NAME "Ubuntu-B.ttf"
#endif

#define WINDOW_WIDTHPX 1200
#define WINDOW_HEIGHTPX 768

int main(int UNUSED(argc), char** UNUSED(argv)) {
    // Inits GLFW et CoqSystem
    GLFWwindow *window = CoqGLFW_and_Coqlib_initAndGetWindow(
            WINDOW_WIDTHPX, WINDOW_HEIGHTPX, 
            NULL, NULL, NULL
    );

    // Inits propre au projet
    Renderer_init();
    Texture_loadCoqlibPngs();
    Texture_loadProjectPngs(MyProject_pngInfos, png_total_pngs);
    GlyphMap_default_init((GlyphMapInit) {});
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds, 0);
    Node_init((NodeInit) {});
    
    CoqEvent_rootOpt = Root_createMyProjectRoot();

    // Ok, start !
    CoqGLFW_setEventsAndStart(window);
    while(!glfwWindowShouldClose(window)) {
        // Events
        glfwPollEvents();

        // Affichage
        Renderer_drawView();
        glfwSwapBuffers(window);
        
        CoqGL_render_checksAfterRendererDraw();
    }
    Renderer_deinit();
    
    CoqGLFW_endAndDestroyWindow(window);
    printok("Clean up done, good bye !");
}

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

