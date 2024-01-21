//
//  MTKView+CoqMetalView.m
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-26.
//

#import "CoqMetalView.h"
#include "coq_timer.h"
#include "coq_event.h"

@implementation CoqMetalView

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device {
    self = [super initWithFrame:frameRect device:device];
    [self setColorPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
    renderer = [[Renderer alloc] initWithView:self];
    [self setDelegate:renderer];
    [self setPaused:YES];
    
    return self;
}
-(BOOL)acceptsFirstResponder {
    return YES;
}

-(void)checkWindowEvents {
    CoqEvent* event = CoqEvent_getWindowEventOpt();
    while(event) {
        bool isFullScreen = [self.window styleMask] & NSWindowStyleMaskFullScreen;
        switch(event->flags & event_types_win_) {
            case event_type_win_windowed: {
                if(isFullScreen) [self.window toggleFullScreen:nil];
                break;
            }
            case event_type_win_full_screen: {
                if(!isFullScreen) [self.window toggleFullScreen:nil];
                break;
            }
            default: {
                printerror("Undefined win event %#010x.", event->flags);
            }
        }
        
        event = CoqEvent_getWindowEventOpt();
    }
}

-(void)startCheckUpDispatchQueue {
    if(checkup_queue == NULL)
        checkup_queue = dispatch_queue_create("coqviewcheckup.queue", NULL);
    dispatch_async(checkup_queue, ^{
        ChronoChecker cc;
        while(true) {
            // Checks
            if(self.isPaused) { break; }
            Root* root = self->root;
            if(root == NULL) { break; }
            if(root->shouldTerminate || [self willTerminate]) { break; }
            chronochecker_set(&cc);
            // Updates prioritaires
            CoqEvent_processEvents(root);
            Timer_check();
//            if(root->iterationUpdate) root->iterationUpdate(root);
            NodeGarbage_burn();
            // Check optionnels
            if(chronochecker_elapsedMS(&cc) > Chrono_UpdateDeltaTMS) {
                printwarning("Overwork?"); continue;
            }
            Texture_checkToFullyDrawAndUnused(&cc, Chrono_UpdateDeltaTMS - 5);
            // Sleep s'il reste du temps.
            int64_t sleepDeltaT = Chrono_UpdateDeltaTMS - chronochecker_elapsedMS(&cc);
            if(sleepDeltaT < 1) sleepDeltaT = 1;
            struct timespec time = {0, sleepDeltaT*ONE_MILLION};
            nanosleep(&time, NULL);
        }
        // Terminate ?
        bool shouldTerminate = true;
        if(self->root) shouldTerminate = self->root->shouldTerminate;
        if(shouldTerminate && ![self willTerminate]) {
#if TARGET_OS_OSX == 1
            dispatch_async(dispatch_get_main_queue(), ^{
                [[NSApplication sharedApplication] terminate:nil];
            });
#else
            printerror("Terminating iOS app.");
#endif
        }
    });
}

/// Pour si on ne veut pas de thread `coqviewcheckup.queue`.
-(BOOL)_checkUp {
    if(self.isPaused) { return false; }
    Root* root = self->root;
    if(root == NULL) { return false; }
    if(root->shouldTerminate || [self willTerminate]) { return false; }
    ChronoChecker cc;
    chronochecker_set(&cc);
    
    // Updates prioritaires
    CoqEvent_processEvents(root);
    Timer_check();
    NodeGarbage_burn();
    
    // Check optionnels
    if(chronochecker_elapsedMS(&cc) > Chrono_UpdateDeltaTMS) {
        printwarning("Overwork?"); return true;
    }
    Texture_checkToFullyDrawAndUnused(&cc, Chrono_UpdateDeltaTMS - 5);
    
    // Sleep s'il reste du temps.
    int64_t sleepDeltaT = Chrono_UpdateDeltaTMS - chronochecker_elapsedMS(&cc);
    if(sleepDeltaT < 1) sleepDeltaT = 1;
    struct timespec time = {0, sleepDeltaT*ONE_MILLION};
    nanosleep(&time, NULL);
    return true;
}

-(void)setSuspended:(BOOL)newSuspended {
    _suspended = newSuspended;
    [self setPaused:newSuspended];
}
-(void)setPaused:(BOOL)paused {
    // Ne peut sortir de pause si l'activité est suspendu...
    if(self.isSuspended && !paused) return;
    ChronoRender_setPaused(paused);  // (empeche d'entrer en veille)
    if(paused == self.isPaused) return;
    // Ok, changement...
    ChronoApp_setPaused(paused);
    [super setPaused:paused];
//    [self setPreferredFramesPerSecond:paused ? 1 : 60];
    if(win_event_timer) [win_event_timer invalidate];
    win_event_timer = nil;
    if(paused) {
        return;
    }
    // Unpause
    if(root) if(root->didResumeActionOpt) root->didResumeActionOpt(root);
    [self startCheckUpDispatchQueue];
    win_event_timer = [NSTimer scheduledTimerWithTimeInterval:0.05 repeats:true block:^(NSTimer * _Nonnull timer) {
        [self checkWindowEvents];
    }];
}

-(void)updateRootFrame:(CGSize)sizePx {
    if(!root) { printerror("root not init"); return; }
#if TARGET_OS_OSX == 1
    NSWindow* window = [self window];
    bool isFullScreen = [window styleMask] & NSWindowStyleMaskFullScreen;
    CGFloat headerHeight = isFullScreen ?
        22 : window.frame.size.height - window.contentLayoutRect.size.height;
    // Ok, le self.frame est déjà à jour quand on call drawableSizeWillChange du renderer...
    CoqEvent_addEvent((CoqEvent){
        event_type_resize, .resize_info = {
            { headerHeight, 0, 0, 0 },
            CGRect_toRectangle(window.frame),
            CGSize_toVector2(sizePx), false, isFullScreen, false
    }});
    window = nil;
#else
    CoqEvent_addEvent((CoqEvent){
        event_type_resize, .resize_info = {
            UIEdgeInsets_toMargins([self safeAreaInsets]),
            CGRect_toRectangle(self.frame),
            CGSize_toVector2(sizePx), false, true, false
    }});
#endif
}

-(void)safeAreaInsetsDidChange {
    [self updateRootFrame: self.drawableSize];
}

/*-- Input Events --------------------------------------*/
#if TARGET_OS_OSX == 1
/*-- Mouse ---------------------------------------------*/
-(void)mouseEntered:(NSEvent *)event {
    // TODO ?
}
-(void)mouseExited:(NSEvent *)event {
}
-(void)updateTrackingAreas {
    if(trackingArea) {
        [self removeTrackingArea:trackingArea];
        trackingArea = nil;
    }
    NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved|
        NSTrackingActiveInKeyWindow|NSTrackingActiveAlways|NSTrackingCursorUpdate;
    trackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds options:options owner:self userInfo:nil];
    [self addTrackingArea:trackingArea];
    [super updateTrackingAreas];
}
-(void)mouseMoved:(NSEvent *)event {
//    if(root == NULL) return;
    [self setPaused:NO];
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    
    CoqEvent coq_event;
    coq_event.flags = event_type_touch_hovering;
    coq_event.touch_pos = root_absposFromViewPos(root, viewPos, false);
    CoqEvent_addEvent(coq_event);
}

-(void)mouseDown:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    
    CoqEvent coq_event;
    coq_event.flags = event_type_touch_down;
    coq_event.touch_pos = root_absposFromViewPos(root, viewPos, false);
    CoqEvent_addEvent(coq_event);
}
-(void)mouseDragged:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    
    CoqEvent coq_event;
    coq_event.flags = event_type_touch_drag;
    coq_event.touch_pos = root_absposFromViewPos(root, viewPos, false);
    CoqEvent_addEvent(coq_event);
}
-(void)mouseUp:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    
    CoqEvent coq_event;
    coq_event.flags = event_type_touch_up;
    CoqEvent_addEvent(coq_event);
}
-(void)scrollWheel:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    // (slidingmenu a sa propre "inertie", si != 0 -> on a un "NSEventPhase" que l'on ignore)
    if(event.momentumPhase != 0)
        return;
    CoqEvent coq_event;
    switch(event.phase) {
        case NSEventPhaseNone:
            coq_event.flags = event_type_scroll;
            coq_event.scroll_deltas = (Vector2) {event.scrollingDeltaX, event.scrollingDeltaY };
            break;
        case NSEventPhaseBegan:
            coq_event.flags = event_type_scrollTrackBegin; break;
        case NSEventPhaseChanged:
            coq_event.flags = event_type_scrollTrack;
            coq_event.scroll_deltas = (Vector2) {event.scrollingDeltaX, event.scrollingDeltaY };
            break;
        case NSEventPhaseEnded:
            coq_event.flags = event_type_scrollTrackEnd; break;
        default:
            coq_event.flags = 0;
            break;
    }
    if(coq_event.flags)
        CoqEvent_addEvent(coq_event);
}

#else

// **-- Touches (similaire a mouse)----------------------------------------
-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    UITouch* touch = [touches anyObject];
    CGPoint location = [touch locationInView:self];
    Vector2 viewPos = { location.x, location.y };
    
    CoqEvent coq_event;
    coq_event.flags = event_type_down;
    coq_event.pos = root_absposFromViewPos(root, viewPos, false);
    CoqEvent_addEvent(coq_event);
}
-(void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    UITouch* touch = [touches anyObject];
    CGPoint location = [touch locationInView:self];
    Vector2 viewPos = { location.x, location.y };
    
    CoqEvent coq_event;
    coq_event.flags = event_type_drag;
    coq_event.pos = root_absposFromViewPos(root, viewPos, false);
    CoqEvent_addEvent(coq_event);
    
}
-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    
    CoqEvent coq_event;
    coq_event.flags = event_type_up;
    CoqEvent_addEvent(coq_event);
}
-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    
    CoqEvent coq_event;
    coq_event.flags = event_type_up;
    CoqEvent_addEvent(coq_event);
}

#endif

// **-- Keyboard input ---------------------------------------------
-(void)keyDown:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    if(event.isARepeat && CoqEvent_ignoreRepeatKeyDown) return;
    CoqEvent coqEvent = { 0 };
    coqEvent.flags = event_type_key_down;
    coqEvent.key.modifiers = (uint32_t)event.modifierFlags;
    coqEvent.key.keycode =   event.keyCode;
    coqEvent.key.mkc =       MKC_of_keycode[event.keyCode];
    strncpy(coqEvent.key.typed.c_str, [event.characters UTF8String], 7);
    CoqEvent_addEvent(coqEvent);
}
-(void)keyUp:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    if(event.isARepeat && CoqEvent_ignoreRepeatKeyDown) return;
    
    CoqEvent coqEvent = { 0 };
    coqEvent.flags = event_type_key_up;
    coqEvent.key.modifiers = (uint32_t)event.modifierFlags;
    coqEvent.key.keycode =   event.keyCode;
    coqEvent.key.mkc =       MKC_of_keycode[event.keyCode];
    CoqEvent_addEvent(coqEvent);
}
-(void)flagsChanged:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    
    CoqEvent coqEvent = { 0 };
    coqEvent.flags = event_type_key_mod;
    coqEvent.key.modifiers = (uint32_t)event.modifierFlags;
    CoqEvent_addEvent(coqEvent);
}

@end
