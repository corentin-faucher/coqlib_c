//
//  AppDelegate.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import <Cocoa/Cocoa.h>
#import "CoqMetalView.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    // Il semble qu'il faut garder une (strong) reference
    // a la window... ? Sinon problem d'autorelease... ?
    NSWindow*     window;
    CoqMetalView* view;
}

@end

