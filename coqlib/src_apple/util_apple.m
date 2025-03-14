//
//  apple_utils.m
//
//  Created by Corentin Faucher on 2023-12-16.
//

#include "util_apple.h"

NSMenu* NSMenu_createDefault(void) {
    // Creation d'un menu simple (juste quit)
    NSMenu* menubar = [NSMenu new];
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
    return menubar;
}
NSWindow* NSWindow_createDefault(NSString* defaultName, float const fixedRatioOpt) {
    NSUInteger uistlyle =  NSWindowStyleMaskClosable|NSWindowStyleMaskTitled|
        NSWindowStyleMaskResizable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskFullSizeContentView;
    NSBackingStoreType bss = NSBackingStoreBuffered;
    CGFloat width = fixedRatioOpt ? 500*fixedRatioOpt : 800;
    NSWindow *window = [[NSWindow alloc] initWithContentRect:CGRectMake(100, 100, width, 500)
        styleMask:uistlyle backing:bss defer:NO];
    NSString* title = [NSBundle.mainBundle localizedStringForKey:@"app_name"
                                                          value:defaultName table:nil];
    [window setTitle:title];
    [window setTitleVisibility:NSWindowTitleHidden];
    [window setTitlebarAppearsTransparent:YES];
    [window makeKeyAndOrderFront:NULL];
    [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [window makeMainWindow];
//    [window center];
    if(fixedRatioOpt)
        [window setContentAspectRatio:NSMakeSize(10*fixedRatioOpt, 10)];
    [window setContentMinSize:NSMakeSize(400, 250)];
    
    return window;
}
