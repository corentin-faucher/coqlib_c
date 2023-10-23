//
//  AppDelegate.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import "AppDelegate.h"
#import "MetalView.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@implementation AppDelegate

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    // Creation d'un menu simple (juste quit)
    id menubar = [NSMenu new];
    id appMenuItem = [NSMenuItem new];
    [menubar addItem:appMenuItem];
    id appMenu = [NSMenu new];
    id appName = [[NSProcessInfo processInfo] processName];
    id quitTitle = [@"Quit " stringByAppendingString:appName];
    id quitMenuItem = [[NSMenuItem alloc]
                       initWithTitle:quitTitle
                       action:@selector(terminate:)
                       keyEquivalent:@"q"];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
    [NSApp setMainMenu:menubar];
    
    // Activation policy... utile ?
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    // Taille de la view/window
    CGRect rect = CGRectMake(0, 0, 400, 400);
    CGRect rect2 = CGRectMake(0, 0, 800, 500);
    // Creation d'une fenetre
    NSUInteger uistlyle = NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable;
    NSBackingStoreType bss = NSBackingStoreBuffered;
    window = [[NSWindow alloc] initWithContentRect:rect2
        styleMask:uistlyle backing:bss defer:NO];
    [window setTitle:@"Hello Window"];
    [window makeKeyAndOrderFront:NULL];
    [window makeMainWindow];
    [window center];
    
    // Vue Metal
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    MetalView *view = [[MetalView alloc] initWithFrame:rect device: device];
    [window setContentView:view];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}


@end
