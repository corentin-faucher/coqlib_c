//
//  AppDelegate.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import "AppDelegate.h"
#include "MyRoot.h"
#include "my_enums.h"
#include "sound.h"


@implementation AppDelegate

static dispatch_queue_t      _my_queue = NULL;

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
    // Taille de la window
    CGRect rect = CGRectMake(0, 0, 800, 500);
    // Creation d'une fenetre
    NSUInteger uistlyle =  NSWindowStyleMaskClosable|NSWindowStyleMaskTitled|
         NSWindowStyleMaskResizable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskFullSizeContentView;
    NSBackingStoreType bss = NSBackingStoreBuffered;
    window = [[NSWindow alloc] initWithContentRect:rect
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
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    view = [[CoqMetalView alloc] initWithFrame:[window frame] device: device];
    
    
    /*-- Chargement des resources du projet. --*/
    Texture_loadPngs(MyProject_pngInfos, png_total_pngs);
    Sound_initWithWavNames(MyProject_wavNames, sound_total_sounds);
    
    /*-- Tout est init, on peut créer la structure... --*/
    view->root = Root_createMyRoot();
    
    // Fini
    [window setContentView:view];
    [view updateRootFrame];
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
    printdebug("Resign active !");
    NSView* view = [window contentView];
    if(![view isKindOfClass:[CoqMetalView class]]) {
        printerror("MTKView is not a custom MetalView.");
        return;
    }
    CoqMetalView* metalView = (CoqMetalView*)view;
    [metalView setSuspended:YES];
    Texture_suspend();
    _Sound_suspend();
}
-(void)applicationDidBecomeActive:(NSNotification *)notification {
    printdebug("become active !");
    NSView* view = [window contentView];
    if(![view isKindOfClass:[CoqMetalView class]]) {
        printerror("MTKView is not a custom MetalView.");
        return;
    }
    CoqMetalView* metalView = (CoqMetalView*)view;
    Texture_resume();
    _Sound_resume();
    [metalView setSuspended:NO];
}


@end
