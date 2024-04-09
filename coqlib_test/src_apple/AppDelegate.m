//
//  AppDelegate.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-02.
//

#import "AppDelegate.h"
#include "graph__apple.h"
#include "coq_sound.h"

#include "my_enums.h"
#include "my_root.h"


@implementation AppDelegate

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    // Creation d'un menu simple (juste quit)
    id menubar = [NSMenu new];
    id appMenuItem = [NSMenuItem new];
    [menubar addItem:appMenuItem];
    id appMenu = [NSMenu new];
//    id appName = [[NSProcessInfo processInfo] processName];
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
    printdebug("🐞🐛🐞-- Debug Mode -- (applicationDidFinishLaunching) 🐞🐛🐞");
    // 1. Init des variables `system` et independantes des prefs
    srand((uint32_t)time(NULL));
    CoqSystem_init();
    Language_init();
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    Font_init();
    CoqGraph_MTLinit(device);
    // 2. Chargement des textures et sons du projet.
    Texture_init(MyProject_pngInfos, png_total_pngs);    
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds);
    Chrono_UpdateDeltaTMS = 50;
    
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
//    [window center];
    [window setContentAspectRatio:NSMakeSize(16, 10)];
    [window setContentMinSize:NSMakeSize(400, 250)];
    
    // 4. Vue metal
    view = [[CoqMetalView alloc] initWithFrame:[window frame] device:device];
    
    // 5. Structure
    view->root = Root_initAndGetProjectRoot();
    
    // Fini    
    [window setContentView:view];
    [view updateRootFrame:[window frame].size dontFix:NO];
}


- (void)applicationWillBecomeActive:(NSNotification *)notification {
    Texture_resume();
    Sound_resume();
}
- (void)applicationDidBecomeActive:(NSNotification *)notification {
    [view setSuspended:NO];
    [window makeFirstResponder:view];
}
- (void)applicationWillResignActive:(NSNotification *)notification {
    [view setSuspended:YES];
}
- (void)applicationDidResignActive:(NSNotification *)notification {
    Texture_suspend();
    Sound_suspend();
}
- (void)applicationWillTerminate:(NSNotification *)aNotification {
    [view setWillTerminate:YES];
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}
-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}


@end
