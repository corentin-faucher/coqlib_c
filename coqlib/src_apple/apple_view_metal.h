//
//  MTKView+CoqMetalView.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-26.
//

#import <MetalKit/MetalKit.h>
#include "nodes/node_root.h"
#if TARGET_OS_OSX != 1
#import "ios_viewcontroller.h"
#endif

@interface CoqMetalView : MTKView {
    
@public
#if TARGET_OS_OSX == 1
    NSTrackingArea*     trackingArea;
#else
    ViewControllerBase* viewController;
#endif
    dispatch_queue_t    checkup_queue;
    dispatch_queue_t    drawPng_queue;
    NSTimer*            win_event_timer;
}

@property (nonatomic) id <MTKViewDelegate> renderer;
@property (nonatomic, getter=isSuspended) BOOL suspended;
@property (nonatomic) BOOL willTerminate;
@property (nonatomic) BOOL shouldTerminate;
@property (nonatomic) BOOL transitioning;
@property (nonatomic) BOOL didTransition;
@property (nonatomic) BOOL iosForceVirtualKeyboard;

- (void)addEvent:(CoqEvent)coqEvent;
- (void)startDrawMissingPngsDispatchQueue;
- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device;
- (void)setUpNotifications;
- (void)updateRootFrame:(CGSize)sizePx dontFix:(BOOL)dontFix;
- (Margins)getMargins;
- (void)checksAfterRendererDraw;

@end

CoqMetalView* mtkView_asCoqMetalViewOpt(MTKView* view);
#if TARGET_OS_OSX != 1
CoqMetalView* uiview_asCoqMetalViewOpt(UIView* view);
#endif

//@protocol CoqViewDelegate <NSObject>
//
//// Méthodes à overrider.
//- (NodeRoot *)getRootNode;
//- (const PngInfo *)getPngInfoList;
//- (uint)getPngInfoListCount;
//
//@end

