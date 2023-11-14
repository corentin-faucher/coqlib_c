//
//  MTKView+CoqMetalView.m
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-26.
//

#import "CoqMetalView.h"
#include "graph__apple.h"
#include "graph_mesh.h"
#include "language.h"
#include "utils.h"
#include "font_manager.h"
#include "chronometers.h"
#include "node_tree.h"
#include "sound.h"
#include "sliding_menu.h"
#include <math.h>

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
    Texture_init(device);
    ChronoApp_setPaused(false);
    srand((uint32_t)time(NULL));
    [self acceptsFirstResponder];
    
#if TARGET_OS_OSX == 1
    [[self window] makeFirstResponder:self];
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
        int64_t lastDeltaT = 0;
        ChronoChecker cc;
        while(true) {
            int64_t sleepDeltaT = Chrono_UpdateDeltaTMS - lastDeltaT;
            if(sleepDeltaT < 1) sleepDeltaT = 1;
            struct timespec time = {0, sleepDeltaT*ONE_MILLION};
            nanosleep(&time, NULL);
            if(self.isPaused) { break; }
            Root* root = self->root;
            if(root == NULL) { break; }
            if(root->shouldTerminate || [self willTerminate]) { break; }
            _chronochecker_set(&cc);
            Timer_check();
            Texture_checkToFullyDraw(&cc);
            Texture_checkUnused();
            if(root->iterationUpdate) root->iterationUpdate(root);
            NodeGarbage_burn();
            lastDeltaT = _chronochceker_elapsedMS(&cc);
        }
        // Terminate ?
        Bool shouldTerminate = true;
        if(self->root) shouldTerminate = self->root->shouldTerminate;
        //        printdebug("loop ended should terminate %d, will terminate %d.",
        //                   shouldTerminate, [self willTerminate]);
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
#if TARGET_OS_OSX == 1
    NSWindow* window = [self window];
    CGFloat headerHeight = (window.styleMask & NSWindowStyleMaskFullScreen) ?
        22 : window.frame.size.height - window.contentLayoutRect.size.height;
    window = nil;
    root->margins = (Margins) { headerHeight, 0, 0, 0 };
    root_setFrame(root,  self.frame.size.width, self.frame.size.height, false);
#else
    UIEdgeInsets m =  [self safeAreaInsets];
    root->margins = (Margins) { m.top, m.left, m.bottom, m.right };
    root_setFrame(root,  self.frame.size.width, self.frame.size.height, false);
#endif
}

-(void)safeAreaInsetsDidChange {
    [self updateRootFrame];
}

/*-- Input Events --------------------------------------*/
#if TARGET_OS_OSX == 1
/*-- Mouse ---------------------------------------------*/
-(void)mouseEntered:(NSEvent *)event {
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
    if(root == NULL) return;
    [self setPaused:NO];
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    Vector2 abspos = root_absposFromViewPos(root, viewPos, false);
    
    Button* const hovered = root_searchButtonOpt(root, abspos, NULL);
    Button* const lastHovered = root->selectedButton;
    if(lastHovered == hovered)
        return;
    if(lastHovered) if(lastHovered->stopHovering)
        lastHovered->stopHovering(lastHovered);
    root->selectedButton = hovered;
    if(hovered) if(hovered->startHovering)
        hovered->startHovering(hovered);
}
-(void)mouseDown:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    Vector2 abspos = root_absposFromViewPos(root, viewPos, false);
    root->lastTouchedPos = abspos;
    Button* const lastSelected = root->selectedButton;
    if(lastSelected) if(lastSelected->stopHovering)
        lastSelected->stopHovering(lastSelected);
    root->selectedButton = NULL;
    // 0. Trouver un bouton sélectionable
    Button* const toTouch = root_searchButtonOpt(root, abspos, NULL);
    if(toTouch == NULL) return;
    // 1. Grab le noeud draggable (si on drag on ne select pas)
    if(toTouch->grab) {
        root->selectedButton = toTouch;
        Vector2 relpos = vector2_absPosToPosInReferentialOfNode(abspos, &toTouch->n);
        toTouch->grab(toTouch, relpos);
        return;
    }
    // 2. Sinon activer le noeud sélectionné (non grabbable)
    if(toTouch->action) {
        toTouch->action(toTouch);
        return;
    }
}
-(void)mouseDragged:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    Button* const grabbed = root->selectedButton;
    if(grabbed == NULL) return;
    if(grabbed->drag == NULL) {
        printwarning("Grabbed node without drag function.");
        return;
    }
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    Vector2 abspos = root_absposFromViewPos(root, viewPos, false);
    Vector2 relpos = vector2_absPosToPosInReferentialOfNode(abspos, &grabbed->n);
    grabbed->drag(root->selectedButton, relpos);
}
-(void)mouseUp:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    Button* const grabbed = root->selectedButton;
    if(grabbed == NULL) return;
    if(grabbed->letGo) {
        grabbed->letGo(root->selectedButton);
    } else {
        printwarning("Grabbed node without letGo function.");
    }
    root->selectedButton = NULL;
}
-(void)scrollWheel:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    // (slidingmenu a sa propre "inertie", si != 0 -> on a un "NSEventPhase" que l'on ignore)
    if(event.momentumPhase != 0)
        return;
    SlidingMenu* sm = root_searchFirstSlidingMenuOpt(root);
    if(sm == NULL) return;
    switch(event.phase) {
        case NSEventPhaseNone:
            slidingmenu_scroll(sm, event.scrollingDeltaY > 0);
        case NSEventPhaseBegan:
            slidingmenu_trackPadScrollBegan(sm); break;
        case NSEventPhaseChanged:
            slidingmenu_trackPadScroll(sm, (float)event.scrollingDeltaY); break;
        case NSEventPhaseEnded:
            slidingmenu_trackPadScrollEnded(sm);
        default: break;
    }    
}
#else
/*-- Touches (similaire a mouse)----------------------------------------*/
-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    UITouch* touch = [touches anyObject];
    CGPoint location = [touch locationInView:self];
    Vector2 viewPos = { location.x, location.y };
    Vector2 abspos = root_absposFromViewPos(root, viewPos, true);
    root->lastTouchedPos = abspos;
    // 0. Trouver un bouton sélectionable
    root->selectedButton = NULL;
    // 0. Trouver un bouton sélectionable
    Button* const toTouch = root_searchButtonOpt(root, abspos, NULL);
    if(toTouch == NULL) return;
    // 1. Grab le noeud draggable (si on drag on ne select pas)
    if(toTouch->grab) {
        root->selectedButton = toTouch;
        Vector2 relpos = vector2_absPosToPosInReferentialOfNode(abspos, &toTouch->n);
        toTouch->grab(toTouch, relpos);
        return;
    }
    // 2. Sinon activer le noeud sélectionné (non grabbable)
    if(toTouch->action) {
        toTouch->action(toTouch);
        return;
    }
}
-(void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    Button* const grabbed = root->selectedButton;
    if(grabbed == NULL) return;
    if(grabbed->drag == NULL) {
        printwarning("Grabbed node without drag function.");
        return;
    }
    UITouch* touch = [touches anyObject];
    CGPoint location = [touch locationInView:self];
    Vector2 viewPos = { location.x, location.y };
    Vector2 abspos = root_absposFromViewPos(root, viewPos, true);
    Vector2 relpos = vector2_absPosToPosInReferentialOfNode(abspos, &grabbed->n);
    grabbed->drag(root->selectedButton, relpos);
}
-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    Button* const grabbed = root->selectedButton;
    if(grabbed == NULL) return;
    if(grabbed->letGo) {
        grabbed->letGo(root->selectedButton);
    } else {
        printwarning("Grabbed node without letGo function.");
    }
    root->selectedButton = NULL;
}
-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    Button* const grabbed = root->selectedButton;
    if(grabbed == NULL) return;
    if(grabbed->letGo) {
        grabbed->letGo(root->selectedButton);
    } else {
        printwarning("Grabbed node without letGo function.");
    }
    root->selectedButton = NULL;
}
#endif
/*-- Keyboard input ---------------------------------------------*/


@end
