//
//  AppDelegate.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-02.
//
#import "AppDelegate.h"

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
    // Init de la window avec view et renderer.
    window = NSWindow_createDefault(@"Coqlib Test", 1.6);
    view = [[CoqMetalView alloc] initWithFrame:[window frame] 
                                        device:CoqGraph_metal_device];
    renderer = [[Renderer alloc] initWithView:view];
    [view setDelegate:renderer];
    [window setContentView:view];
    CoqSystem_setViewSize((ViewSizeInfo) {
        .margins = [view getMargins],
        .framePt = CGSize_toVector2(window.frame.size),
        .framePx = CGSize_toVector2([view drawableSize]), 
        .fullScreen = [window styleMask] & NSWindowStyleMaskFullScreen,
    });
    
    // Inits spécifiques au projet : textures, sons, renderer, structure.
    Texture_loadCoqlibPngs();
    Texture_loadProjectPngs(MyProject_pngInfos, png_total_pngs);
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds, 0);
    CoqEvent_root = Root_initAndGetProjectRoot();
    
    printok("Tout est init.");
    
    // Test...
//    CoqFont_test_printAvailableFonts();
}

@end
