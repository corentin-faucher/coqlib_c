//
//  MTKView+CoqMetalView.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-26.
//

#import <MetalKit/MetalKit.h>
//#import "metal_renderer.h"
#if TARGET_OS_OSX != 1
#import "ViewController.h"
#endif

#include "nodes/node_root.h"

//@protocol CoqViewDelegate;

@interface CoqMetalView : MTKView {
    
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

@property (nonatomic, getter=isSuspended) BOOL suspended;
@property (nonatomic) BOOL willTerminate;
@property (nonatomic) BOOL transitioning;
@property (nonatomic) BOOL didTransition;
@property (nonatomic) BOOL iosForceVirtualKeyboard;

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device;
- (void)setUpNotifications;
- (void)updateRootFrame:(CGSize)sizePx dontFix:(BOOL)dontFix;

// Méthodes à overrider. Superflu ?
//- (NodeRoot *)getRootNode;
//- (const PngInfo *)getPngInfoList;
//- (uint)getPngInfoListCount;

@end

//@protocol CoqViewDelegate <NSObject>
//
//// Méthodes à overrider.
//- (NodeRoot *)getRootNode;
//- (const PngInfo *)getPngInfoList;
//- (uint)getPngInfoListCount;
//
//@end

