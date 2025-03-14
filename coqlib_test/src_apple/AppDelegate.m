//
//  AppDelegate.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-02.
//
#import "AppDelegate.h"
#include "coq_sound.h"
#include "utils/util_system.h"
#include "graph__metal.h"
#include "util_apple.h"

// Includes du projet
#include "metal_renderer.h"
#include "my_enums.h"
#include "my_root.h"

@implementation AppDelegate

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    [NSApp setMenu:NSMenu_createDefault()];
    // Init des variables `system`, Graphs, Sound.
    srand((uint32_t)time(NULL));
    CoqSystem_init();
    CoqGraph_metal_init(MTLCreateSystemDefaultDevice(), 0, 0, NULL, NULL);
    // Init de la window
    window = NSWindow_createDefault(@"Coqlib Test", 1.6);
    view = [[CoqMetalView alloc] initWithFrame:[window frame] 
                                        device:CoqGraph_metal_device];
    
    // Inits spécifiques au projet : textures, sons, renderer, structure.
    Texture_loadCoqlibPngs();
    Texture_loadProjectPngs(MyProject_pngInfos, png_total_pngs);
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds);
    renderer = [[Renderer alloc] initWithView:view];
    view->rootOpt = Root_initAndGetProjectRoot();
    
    // Setter renderer, view et window ensemble.
    [view setDelegate:renderer];
    [window setContentView:view];
    [view updateRootFrame:[window frame].size dontFix:NO];
    printok("Tout est init.");
    
    // Test...
//    CoqFont_test_printAvailableFonts();
}

@end
