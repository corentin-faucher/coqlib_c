//
//  macos_appdelegate.h
//  App Delegate par défaut (base)
//
//  Created by Corentin Faucher on 2024-10-17.
//

#import <Cocoa/Cocoa.h>
#import "apple_view_metal.h"

@interface AppDelegateBase : NSObject <NSApplicationDelegate> {
@public
    NSWindow*     window;
    CoqMetalView* view;
    id<MTKViewDelegate> renderer;
}

@end
