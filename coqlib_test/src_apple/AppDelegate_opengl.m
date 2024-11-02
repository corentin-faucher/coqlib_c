//
//  AppDelegate_opengl.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-08.
//

#import "AppDelegate_opengl.h"
#include "graph__opengl.h"
#include "coq_sound.h"
#include "utils/util_system.h"

// Includes du projet
//#include "metal_renderer.h"
//#include "my_enums.h"
//#include "my_root.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

@implementation AppDelegateOpenGL

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    // Creation d'un menu simple (juste quit)
    id menubar = [NSMenu new];
    id appMenuItem = [NSMenuItem new];
    [menubar addItem:appMenuItem];
    id appMenu = [NSMenu new];
    NSString* quit = [NSBundle.mainBundle localizedStringForKey:@"quit" value:@"Quit" table:nil];
    id quitMenuItem = [[NSMenuItem alloc]
                       initWithTitle:quit
                       action:@selector(terminate:)
                       keyEquivalent:@"q"];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
    [NSApp setMainMenu:menubar];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    printdebug("🐞🐛🐞-- Debug Mode -- 🐞🐛🐞");
    
    // 3. Init de la window
    NSUInteger uistlyle =  NSWindowStyleMaskClosable|NSWindowStyleMaskTitled|
        NSWindowStyleMaskResizable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskFullSizeContentView;
    NSBackingStoreType bss = NSBackingStoreBuffered;
    window = [[NSWindow alloc] initWithContentRect:CGRectMake(100, 100, 1600, 1000)
        styleMask:uistlyle backing:bss defer:NO];
    [window setTitle:[NSBundle.mainBundle localizedStringForKey:@"app_name"
                                                          value:@"Coqlib Test" table:nil]];
    [window setTitleVisibility:NSWindowTitleHidden];
    [window setTitlebarAppearsTransparent:YES];
    [window makeKeyAndOrderFront:NULL];
    [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [window makeMainWindow];
    [window center];
    [window setContentAspectRatio:NSMakeSize(16, 10)];
    [window setContentMinSize:NSMakeSize(400, 250)];
    
    // 1. Init des variables `system`, Graphs, Sound.
    srand((uint32_t)time(NULL));
    CoqSystem_init();
    Sound_initWithWavNames(NULL, 0);
    
    // 4. Vue OpenGL
    GLuint attribs[] = { //PF: PixelAttibutes
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAWindow,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAStencilSize, 8,
            NSOpenGLPFAAccumSize, 0,
            0
        };
    NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: (NSOpenGLPixelFormatAttribute*) attribs];
    view = [[CoqOpenGLView alloc] initWithFrame:[window frame] pixelFormat:fmt];
    [window setContentView:view];
//    [view display];
    
//    view = [[CoqMetalView alloc] initWithFrame:[window frame]                                                         device:CoqGraph_getMTLDevice()];
//    renderer = [[Renderer alloc] initWithView:view];
//    [view setDelegate:renderer];
    
    // 5. Structure
//    view->root = Root_initAndGetProjectRoot();
    
    // Fini setter la view comme le contenue de la fenêtre...
//    [window setContentView:view];
//    [view updateRootFrame:[window frame].size dontFix:NO];
}


- (void)applicationWillBecomeActive:(NSNotification *)notification {
    Texture_resume();
    Sound_resume();
}
- (void)applicationDidBecomeActive:(NSNotification *)notification {
//    [view setSuspended:NO];
    [window makeFirstResponder:view];
}
- (void)applicationWillResignActive:(NSNotification *)notification {
//    [view setSuspended:YES];
}
- (void)applicationDidResignActive:(NSNotification *)notification {
    Texture_suspend();
    Sound_suspend();
}
- (void)applicationWillTerminate:(NSNotification *)aNotification {
//    [view setWillTerminate:YES];
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}
-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}


@end

#pragma clang diagnostic pop
