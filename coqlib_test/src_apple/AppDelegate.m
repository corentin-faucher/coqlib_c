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
    // Init de Window, View, System...
    srand((uint32_t)time(NULL));
    [NSApp setMenu:NSMenu_createDefault()];
    window = NSWindow_createDefault(1.6);
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    view = [[CoqMetalView alloc] initWithFrame:[window frame] 
                                        device:device];
    [window setContentView:view];
    ViewSizeInfo viewSizeInfo = ViewSizeInfo_fromMetalViewAndWindow(view, window);
    CoqSystem_init(viewSizeInfo);
    
    // Inits spécifiques au projet : Renderer, textures, sons, structure.
    CoqMtl_init(device, 0, 0);
    [view setRenderer:[[Renderer alloc] initWithView:view]];
    Texture_loadCoqlibPngs();
    Texture_loadProjectPngs(MyProject_pngInfos, png_total_pngs);
    GlyphMap_default_init((GlyphMapInit) {});
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds, 0);
    Node_init((NodeInit) {});
    
    CoqEvent_rootOpt = Root_createMyProjectRoot();
    
    printok("Tout est init.");
}

@end
