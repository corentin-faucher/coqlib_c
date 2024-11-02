//
//  apple_utils.m
//
//  Created by Corentin Faucher on 2023-12-16.
//

#include "util_apple.h"

CGRect  rectangle_toCGRect(Rectangle rect) {
    return (CGRect) { rect.o_x, rect.o_y, rect.w, rect.h };
}
CGSize  vector2_toCGSize(Vector2 v) {
    return (CGSize) { v.x, v.y };
}
MTLClearColor vector4_toMTLClearColor(Vector4 v) {
    return (MTLClearColor) { v.x, v.y, v.z, v.w };
}
Rectangle  CGRect_toRectangle(CGRect rect) {
    return (Rectangle) { rect.origin.x, rect.origin.y, rect.size.width, rect.size.height };
}
Vector2  CGSize_toVector2(CGSize v) {
    return (Vector2) { v.width, v.height };
}
#if TARGET_OS_OSX != 1
Margins UIEdgeInsets_toMargins(UIEdgeInsets m) {
    return (Margins) { m.top, m.left, m.bottom, m.right };
}
#endif

void  NSApp_createDefaultMenu(void) {
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
