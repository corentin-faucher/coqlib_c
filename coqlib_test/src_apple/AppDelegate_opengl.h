//
//  AppDelegate_opengl.h
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-08.
//

#import <Cocoa/Cocoa.h>
#import "apple_view_opengl.h"

@interface AppDelegateOpenGL : NSObject <NSApplicationDelegate> {
    NSWindow*      window;
    CoqOpenGLView* view;
}

@end
