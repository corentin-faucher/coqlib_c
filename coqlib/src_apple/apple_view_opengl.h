//
//  apple_opengl_view.h
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-08.
//
#import <Cocoa/Cocoa.h>
#include "nodes/node_root.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

@interface CoqOpenGLView : NSOpenGLView {
    
@public
    Root*            root;
    ChronoChecker    cc;
#if TARGET_OS_OSX == 1
    NSTrackingArea*  trackingArea;
#else
    ViewController*  viewController;
#endif
    dispatch_queue_t checkup_queue;
    NSTimer*         win_event_timer;
}


@end

#pragma clang diagnostic pop
