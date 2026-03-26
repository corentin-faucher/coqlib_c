//
//  apple_utils.m
//
//  Created by Corentin Faucher on 2023-12-16.
//

#include "util_apple.h"

#if TARGET_OS_OSX == 1
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
NSWindow* NSWindow_createDefault(float const fixedRatioOpt) {
    NSUInteger uistlyle =  NSWindowStyleMaskClosable|NSWindowStyleMaskTitled|
        NSWindowStyleMaskResizable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskFullSizeContentView;
    NSBackingStoreType bss = NSBackingStoreBuffered;
    CGFloat width = fixedRatioOpt ? 500*fixedRatioOpt : 800;
    NSWindow *window = [[NSWindow alloc] initWithContentRect:CGRectMake(100, 100, width, 500)
        styleMask:uistlyle backing:bss defer:NO];
    NSString* title = [NSBundle.mainBundle localizedStringForKey:@"app_name"
                                                          value:nil table:nil];
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

ViewSizeInfo ViewSizeInfo_fromMetalViewAndWindow(CoqMetalView*const view, NSWindow*const window) {
    return (ViewSizeInfo) {
        .margins = [view getMargins],
        .framePt = CGRect_toRectangle(window.frame),
        .frameSizePx = CGSize_toVector2([view drawableSize]), 
        .fullScreen = [window styleMask] & NSWindowStyleMaskFullScreen,
    };
}

#endif
