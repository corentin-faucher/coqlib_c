//
//  AppDelegate.h
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//

#import <Cocoa/Cocoa.h>
#import "CoqMetalView.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    // Il faut garder une (strong) reference
    // a la window... Sinon problem d'autorelease...
    NSWindow*     window;
    CoqMetalView* view;
}

@end

