//
//  AppDelegate.h
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-02.
//

#import <Cocoa/Cocoa.h>
#import "metal_view.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow*     window;
    CoqMetalView* view;
}

@end
