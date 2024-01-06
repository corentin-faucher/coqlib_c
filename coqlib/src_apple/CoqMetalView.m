//
//  MTKView+CoqMetalView.m
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-26.
//

#import "CoqMetalView.h"
#include "_utils_apple.h"
#include "coq_timer.h"
#include "_graph__apple.h"
#include "_graph_font_manager.h"
#include "coq_event.h"

@implementation CoqMetalView

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device {
    self = [super initWithFrame:frameRect device:device];
    [self setColorPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
    renderer = [[Renderer alloc] initWithView:self];
    [self setDelegate:renderer];
    node_updateModelAndGetAsDrawableOpt = node_defaultUpdateModelAndGetAsDrawableOpt;
    
    // Init de base dans l'ordre.
    printdebug("🐞🐛🐞-- Debug Mode --🐞🐛🐞");
    [self setPaused:YES];
    Language_init();
    Font_init();
    Mesh_init(device);
    _Texture_init(device);
    
    ChronoApp_setPaused(false);
    srand((uint32_t)time(NULL));
#if TARGET_OS_OSX == 1
    _MacOS_updateCurrentLayout();
#endif
    
    return self;
}
-(BOOL)acceptsFirstResponder {
    return YES;
}
-(void)startCheckUpDispatchQueue {
    if(_my_queue == NULL)
        _my_queue = dispatch_queue_create("coqviewcheckup.queue", NULL);
    dispatch_async(_my_queue, ^{
        ChronoChecker cc;
        while(true) {
            // Checks
            if(self.isPaused) { break; }
            Root* root = self->root;
            if(root == NULL) { break; }
            if(root->shouldTerminate || [self willTerminate]) { break; }
            _chronochecker_set(&cc);
            // Updates prioritaires
            CoqEvent_processEvents(root);
            Timer_check();
            if(root->iterationUpdate) root->iterationUpdate(root);
            NodeGarbage_burn();
            // Check optionnels
            if(_chronochceker_elapsedMS(&cc) > Chrono_UpdateDeltaTMS) {
                printwarning("Overwork?"); continue;
            }
            _Texture_checkToFullyDrawAndUnused(&cc, Chrono_UpdateDeltaTMS - 5);
            // Sleep s'il reste du temps.
            int64_t sleepDeltaT = Chrono_UpdateDeltaTMS - _chronochceker_elapsedMS(&cc);
            if(sleepDeltaT < 1) sleepDeltaT = 1;
            struct timespec time = {0, sleepDeltaT*ONE_MILLION};
            nanosleep(&time, NULL);
            // Event sur la MetalView / NSWindow
#if TARGET_OS_OSX == 1
#warning TODO: Ajouter des event Root -> Window (on a deja les event ordinaire Window -> root)
//            if(prefs_shouldToggleFullScreen(root->prefs)) {
//                dispatch_async(dispatch_get_main_queue(), ^{
//                    [self toggleFullScreen];
//                });
//            }
#endif
        }
        // Terminate ?
        bool shouldTerminate = true;
        if(self->root) shouldTerminate = self->root->shouldTerminate;
        printdebug("loop ended should terminate %d, will terminate %d.",
                   shouldTerminate, [self willTerminate]);
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
    _chronochecker_set(&cc);
    // Updates prioritaires
    CoqEvent_processEvents(root);
    Timer_check();
    if(root->iterationUpdate) root->iterationUpdate(root);
    NodeGarbage_burn();
    // Check optionnels
    if(_chronochceker_elapsedMS(&cc) > Chrono_UpdateDeltaTMS) {
        printwarning("Overwork?"); return true;
    }
    _Texture_checkToFullyDrawAndUnused(&cc, Chrono_UpdateDeltaTMS - 5);
    // Sleep s'il reste du temps.
    int64_t sleepDeltaT = Chrono_UpdateDeltaTMS - _chronochceker_elapsedMS(&cc);
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
    if(paused) return;
    // Unpause
    if(root) if(root->didResume) root->didResume(root);
    [self startCheckUpDispatchQueue];
}

-(void)updateRootFrame {
    if(!root) { printerror("root not init"); return; }
    CoqEvent coq_event;
    coq_event.flags = event_type_resize;
#if TARGET_OS_OSX == 1
    NSWindow* window = [self window];
    CGFloat headerHeight = (window.styleMask & NSWindowStyleMaskFullScreen) ?
        22 : window.frame.size.height - window.contentLayoutRect.size.height;
    window = nil;
    
    coq_event.resize_margins = (Margins) { headerHeight, 0, 0, 0 };
    coq_event.resize_sizesPx = (Vector2) { self.frame.size.width, self.frame.size.height };
    coq_event.resize_inTransition = false;
#else
    UIEdgeInsets m =  [self safeAreaInsets];
    
    coq_event.margins = (Margins) { m.top, m.left, m.bottom, m.right };
    coq_event.sizesPx = (Vector2) { self.frame.size.width, self.frame.size.height };
    coq_event.inTransition = false;
#endif
    CoqEvent_addEvent(coq_event);
}

-(void)safeAreaInsetsDidChange {
    [self updateRootFrame];
}

-(void)toggleFullScreen {
#if TARGET_OS_OSX == 1
    NSWindow* window = [self window];
//    if(window.styleMask & NSWindowStyleMaskFullScreen) {
        [window toggleFullScreen:nil];
//    }
#endif
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
    coq_event.flags = event_type_hovering;
    coq_event.touch_pos = root_absposFromViewPos(root, viewPos, false);
    CoqEvent_addEvent(coq_event);
}

-(void)mouseDown:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    
    CoqEvent coq_event;
    coq_event.flags = event_type_down;
    coq_event.touch_pos = root_absposFromViewPos(root, viewPos, false);
    CoqEvent_addEvent(coq_event);
}
-(void)mouseDragged:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    
    CoqEvent coq_event;
    coq_event.flags = event_type_drag;
    coq_event.touch_pos = root_absposFromViewPos(root, viewPos, false);
    CoqEvent_addEvent(coq_event);
}
-(void)mouseUp:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    
    CoqEvent coq_event;
    coq_event.flags = event_type_up;
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
    
    const char* c_str = [event.characters UTF8String];
    CoqEvent coqEvent;
    coqEvent.flags = event_type_keyDown;
    coqEvent.key.keycode = event.keyCode;
    coqEvent.key.modifiers = (uint32_t)event.modifierFlags;
    coqEvent.key.isVirtual = false;
    for(uint32_t i = 0; i < uminu(8, (uint32_t)strlen(c_str)); i++)
        coqEvent.key.typedChar[i] = c_str[i];
    CoqEvent_addEvent(coqEvent);
}
-(void)keyUp:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    if(event.isARepeat && CoqEvent_ignoreRepeatKeyDown) return;
    
    CoqEvent coqEvent;
    coqEvent.flags = event_type_keyUp;
    coqEvent.key.keycode = event.keyCode;
    coqEvent.key.modifiers = (uint32_t)event.modifierFlags;
    coqEvent.key.isVirtual = false;
    CoqEvent_addEvent(coqEvent);
}
-(void)flagsChanged:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    
    CoqEvent coqEvent;
    coqEvent.flags = event_type_keyMod;
    coqEvent.key.modifiers = (uint32_t)event.modifierFlags;
    CoqEvent_addEvent(coqEvent);
}

@end
