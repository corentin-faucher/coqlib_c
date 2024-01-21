//
//  AppDelegate.m
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//
#import "AppDelegate.h"

#include "coq_sound.h"
#include "graph__apple.h"

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
    NSString* appName = [NSBundle.mainBundle localizedStringForKey:@"app_name"
                                                             value:@"Demo Xcode" table:nil];
    id quitMenuItem = [[NSMenuItem alloc]
                       initWithTitle:[NSString stringWithFormat:@"%@ %@", quit, appName]
                       action:@selector(terminate:)
                       keyEquivalent:@"q"];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
    [NSApp setMainMenu:menubar];
    // Activation policy... utile ? Non, defaut de toute facon.
//    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    printdebug("🐞🐛🐞-- Debug Mode --🐞🐛🐞");
    // 1. Init des variables `system` et independantes des prefs (fonts manager)
    srand((uint32_t)time(NULL));
    Language_init();
    CoqSystem_updateCurrentLayout();
    CoqSystem_updateCurrentTheme();
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    Font_init();
    Mesh_init(device);
    Texture_init(device);
    
    // 3. Chargement des resources
    Texture_loadPngs(MyProject_pngInfos, png_total_pngs);
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds);
    
    // Creation d'une fenetre
    NSUInteger uistlyle =  NSWindowStyleMaskClosable|NSWindowStyleMaskTitled|
         NSWindowStyleMaskResizable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskFullSizeContentView;
    NSBackingStoreType bss = NSBackingStoreBuffered;
    window = [[NSWindow alloc] initWithContentRect:CGRectMake(0, 0, 800, 500)
        styleMask:uistlyle backing:bss defer:NO];
    [window setTitle:[NSBundle.mainBundle localizedStringForKey:@"app_name"
                                                          value:@"Demo Xcode" table:nil]];
    [window setTitleVisibility:NSWindowTitleHidden];
    [window setTitlebarAppearsTransparent:YES];
    [window makeKeyAndOrderFront:NULL];
    [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [window makeMainWindow];
    [window center];
    [window setContentAspectRatio:NSMakeSize(16, 10)];
    [window setContentMinSize:NSMakeSize(400, 250)];
    
    // Vue Metal
    view = [[CoqMetalView alloc] initWithFrame:[window frame] device: device];
    
    /*-- Tout est init, on peut créer la structure... --*/
    view->root = Root_createMyRoot();
    
    // Fini
    [window setContentView:view];
    [view updateRootFrame:view.drawableSize];
    
    // Chronos, unpause.
    view->renderer->noSleep = YES;
    ChronoApp_setPaused(false);
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    printdebug("Will terminate");
    [view setWillTerminate:YES];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}

-(void)applicationWillResignActive:(NSNotification *)notification {
    [view setSuspended:YES];
    Texture_suspend();
    Sound_suspend();
}
-(void)applicationDidBecomeActive:(NSNotification *)notification {
    Texture_resume();
    Sound_resume();
    [view setSuspended:NO];
}

@end
