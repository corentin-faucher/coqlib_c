//
//  MTKView+CoqMetalView.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-26.
//

#import <MetalKit/MetalKit.h>
#import "Renderer.h"
#include "_node_root.h"

//@protocol CoqViewDelegate;

@interface CoqMetalView : MTKView {
    
@public
    Renderer*        renderer;
    Root*            root;
#if TARGET_OS_OSX == 1
    NSTrackingArea*  trackingArea;
#endif
    dispatch_queue_t _my_queue;
}

@property (nonatomic, getter=isSuspended) BOOL suspended;
@property (nonatomic) BOOL willTerminate;
@property (nonatomic) BOOL transitioning;
@property (nonatomic) BOOL didTransition;

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device;
- (void)updateRootFrame:(CGSize)sizePx;

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

