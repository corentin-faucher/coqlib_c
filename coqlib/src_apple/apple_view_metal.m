//
//  MTKView+CoqMetalView.m
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-26.
//
#import "apple_view_metal.h"

#include "systems/system_base.h"
#include "systems/system_locale.h"
#include "systems/system_language.h"
#include "utils/util_base.h"
#include "utils/util_chrono.h"
#include "utils/util_event.h"
#include "utils/util_timer.h"
#include "util_apple.h"

#import <CloudKit/CloudKit.h>
#import <GameController/GameController.h>
#if TARGET_OS_OSX == 1
#import <Carbon/Carbon.h>
//@interface CoqMetalView()
//@end
#else
//#import "ios_dummyTextField.h"
#import "ios_dummy_scrollview.h"
#import <CoreText/CoreText.h>

@interface CoqMetalView ()

//@property (strong, nonatomic) DummyTextField *   dummyTextField;
//@property (strong, nonatomic) DummySpace *         dummySpace;
@property (strong, nonatomic) DummyScrollView *  dummyScrollView;
@property CGFloat keyboard_height;

@end
#endif

CoqMetalView* mtkView_asCoqMetalViewOpt(MTKView*const view) {
    if(!view) {
        printerror("No view.");
        return NULL;
    }
    if(![view isKindOfClass:[CoqMetalView class]]) {
        printerror("MTKView is not a custom MetalView.");
        return NULL;
    }
    return (CoqMetalView*)view;
}

#if TARGET_OS_OSX != 1
CoqMetalView* uiview_asCoqMetalViewOpt(UIView*const view) {
    if(!view) {
        printerror("No view.");
        return NULL;
    }
    if(![view isKindOfClass:[CoqMetalView class]]) {
        printerror("UIView is not a custom MetalView. Not set in Main.storyboard?");
        return NULL;
    }
    return (CoqMetalView*)view;
}
#endif

@implementation CoqMetalView

-(void)setRenderer:(id<MTKViewDelegate>)renderer {
    _renderer = renderer;
    [self setDelegate:renderer];
}

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device {
    self = [super initWithFrame:frameRect device:device];
    [self setUpNotifications];
    return self;
}
- (void)awakeFromNib {
    [super awakeFromNib];
    [self setDevice:MTLCreateSystemDefaultDevice()];
    [self setUpNotifications];
}
-(void)setUpNotifications {
    [self setPaused:YES];
//    cc = chronochecker_startNew();
#if TARGET_OS_OSX == 1
    // Notifications
    {
        // Notifications Full screen (pause unpause)
        [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowWillEnterFullScreenNotification
                                                          object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
           [self setSuspended:YES];
        }];
        [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidEnterFullScreenNotification
                                                          object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            [self setSuspended:NO];
        }];
        [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowWillExitFullScreenNotification
                                                          object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            [self setSuspended:YES];
        }];
        [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidExitFullScreenNotification
                                                          object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            [self setSuspended:NO];
        }];
        // Notification Window Move (pour resize -> voir updateRootFrame)
        [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidMoveNotification
                                                          object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            NSWindow* window = [self window];
            bool isFullScreen = [window styleMask] & NSWindowStyleMaskFullScreen;
            // Ok, le self.frame est déjà à jour quand on call drawableSizeWillChange du renderer...
            [self addEvent:(CoqEvent){
                .type = eventtype_resize, .resize_info = {
                    .margins = [self getMargins],
                    .framePt = CGRect_toRectangle(window.frame),
                    .frameSizePx = CGSize_toVector2([self drawableSize]),
                    .fullScreen = isFullScreen, 
                    .justMoving = true, 
                    .dontFix = false,
            }}];
        }];
        // Notification Layout, theme, language
        [[NSDistributedNotificationCenter defaultCenter]
         addObserverForName:(__bridge NSString*)kTISNotifySelectedKeyboardInputSourceChanged
         object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            CoqSystem_layoutUpdate();
            CoqEvent_addToRootEvent((CoqEvent) { eventtype_systemChanged, .system_change = { .layoutDidChange = true }});
        }];
        [[NSDistributedNotificationCenter defaultCenter]
         addObserverForName:@"AppleInterfaceThemeChangedNotification"
         object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            CoqSystem_OS_appearanceUpdate();
            CoqEvent_addToRootEvent((CoqEvent) { eventtype_systemChanged, .system_change = { .themeDidChange = true }});
        }];
        [[NSDistributedNotificationCenter defaultCenter]
         addObserverForName:@"AppleLanguagePreferencesChangedNotification"
         object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            CoqLocale_update();
            Language_checkSystemLanguage();
            CoqEvent_addToRootEvent((CoqEvent) { eventtype_systemChanged, .system_change = { .languageRegionDidChange = true }});
        }];
        // Game Controller
        if (@available(macOS 11.0, *)) {
            [[NSNotificationCenter defaultCenter] addObserverForName:GCControllerDidBecomeCurrentNotification 
                                                    object:nil queue:nil usingBlock:^(NSNotification * _Nonnull notification) {
                GCController* controller = [GCController current];
                if(!controller) return;
                GCExtendedGamepad* gamePad = [controller extendedGamepad];
                if(!gamePad) { printwarning("No gamepad in controller."); return; }
                [[gamePad dpad] setValueChangedHandler:^(GCControllerDirectionPad * _Nonnull dpad, float xValue, float yValue) {
                    GamePadInput input = { gamepad_dpad, .vector = { xValue, yValue }};
                    [self addEvent:(CoqEvent) { 
                        .type = eventtype_gamePad_value, 
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad leftThumbstick] setValueChangedHandler:^(GCControllerDirectionPad * _Nonnull dpad, float xValue, float yValue) {
                    GamePadInput input = { gamepad_JoystickLeft, .buttonValue = gamePad.leftThumbstickButton.value, .vector = { xValue, yValue }};
                    [self addEvent:(CoqEvent) { 
                        .type = eventtype_gamePad_value, 
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad rightThumbstick] setValueChangedHandler:^(GCControllerDirectionPad * _Nonnull dpad, float xValue, float yValue) {
                    GamePadInput input = { gamepad_JoystickRight, .buttonValue = gamePad.rightThumbstickButton.value, .vector = { xValue, yValue }};
                    [self addEvent:(CoqEvent) { 
                        .type = eventtype_gamePad_value, 
                        .gamepadInput = input}
                    ];
                }];
                [[gamePad buttonA] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    GamePadInput input = { gamepad_A, value };
                    [self addEvent:(CoqEvent) { 
                        .type = pressed ? eventtype_gamePad_down : eventtype_gamePad_up,
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad buttonB] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    GamePadInput input = { gamepad_B, value };
                    [self addEvent:(CoqEvent) { 
                        .type = pressed ? eventtype_gamePad_down : eventtype_gamePad_up,
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad buttonX] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    GamePadInput input = { gamepad_X, value };
                    [self addEvent:(CoqEvent) { 
                        .type = pressed ? eventtype_gamePad_down : eventtype_gamePad_up,
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad buttonY] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    GamePadInput input = { gamepad_Y, value };
                    [self addEvent:(CoqEvent) { 
                        .type = pressed ? eventtype_gamePad_down : eventtype_gamePad_up,
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad rightTrigger] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    GamePadInput input = { gamepad_ZR, value };
                    [self addEvent:(CoqEvent) { 
                        .type = pressed ? eventtype_gamePad_down : eventtype_gamePad_up,
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad rightShoulder] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    GamePadInput input = { gamepad_R, value };
                    [self addEvent:(CoqEvent) { 
                        .type = pressed ? eventtype_gamePad_down : eventtype_gamePad_up,
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad leftTrigger] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    GamePadInput input = { gamepad_ZL, value };
                    [self addEvent:(CoqEvent) { 
                        .type = pressed ? eventtype_gamePad_down : eventtype_gamePad_up,
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad leftShoulder] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    GamePadInput input = { gamepad_L, value };
                    [self addEvent:(CoqEvent) { 
                        .type = pressed ? eventtype_gamePad_down : eventtype_gamePad_up,
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad buttonMenu] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    GamePadInput input = { gamepad_Plus, value };
                    [self addEvent:(CoqEvent) { 
                        .type = pressed ? eventtype_gamePad_down : eventtype_gamePad_up,
                        .gamepadInput = input 
                    }];
                }];
                [[gamePad buttonOptions] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    GamePadInput input = { gamepad_Minus, value };
                    [self addEvent:(CoqEvent) { 
                        .type = pressed ? eventtype_gamePad_down : eventtype_gamePad_up,
                        .gamepadInput = input 
                    }];
                }];
            }];
        }
    }
#else
//    bool ios13available = false;
//    if(@available(iOS 13.4, *)) {
//        ios13available = true;
//    }
//    if(!ios13available || self.iosForceVirtualKeyboard) {
//        self.dummyTextField = [[DummyTextField alloc]
//                               initWithFrame:CGRectMake(0, 60, 120, 30)
//                               andMetalView:self];
//        [self addSubview:self.dummyTextField];
//        printdebug("Adding dummy text field %s.", [[self.dummyTextField description] UTF8String]);
//    }
    // Notifications
    {
        NSNotificationCenter* dc = [NSNotificationCenter defaultCenter];
        [dc addObserverForName:UIKeyboardDidShowNotification
                        object:nil queue:nil
                    usingBlock:^(NSNotification * _Nonnull note) {
//            printdebug("🐷 Keyboard did show, trans %d.", self.transitioning);
            self.keyboard_height = [note.userInfo[UIKeyboardFrameEndUserInfoKey] CGRectValue].size.height;
            if(self.transitioning) return;
            CoqEvent_addToRootEvent((CoqEvent) {
                .type = eventtype_systemChanged,
                .system_change = {.ios_keyboardUp = true }
            });
            [self updateRootFrame:self.drawableSize dontFix:YES];
        }];
    //    [dc addObserverForName:UIKeyboardWillShowNotification object:nil queue:nil
    //                usingBlock:^(NSNotification * _Nonnull note) {
    //        printdebug("🐷 Keyboard Will show, trans %d.", self.transitioning);
    //    }];
        [dc addObserverForName:UIKeyboardDidChangeFrameNotification object:nil queue:nil
                    usingBlock:^(NSNotification * _Nonnull note) {
//            printdebug("🐷 Keyboard did change, trans %d.", self.transitioning);
            CGRect keyboardRect = [note.userInfo[UIKeyboardFrameEndUserInfoKey] CGRectValue];
            self.keyboard_height = keyboardRect.size.height;
        }];
    //    [dc addObserverForName:UIKeyboardWillChangeFrameNotification object:nil queue:nil
    //                usingBlock:^(NSNotification * _Nonnull note) {
    //        printdebug("🐷 Keyboard Will change, trans %d.", self.transitioning);
    //    }];
    //    [dc addObserverForName:UIKeyboardWillHideNotification object:nil queue:nil
    //                usingBlock:^(NSNotification * _Nonnull note) {
    //        printdebug("🐷 Keyboard Will hide, trans %d.", self.transitioning);
    //    }];
        [dc addObserverForName:UIKeyboardDidHideNotification
                        object:nil queue:nil
                    usingBlock:^(NSNotification * _Nonnull note) {
            // (On remet les marges ordinaires)
            self.keyboard_height = 0;
//            printdebug("🐷 Keyboard did hide, trans %d.", self.transitioning);
            if(self.transitioning) return;
            [self updateRootFrame:self.drawableSize dontFix:YES];
            CoqEvent_addToRootEvent((CoqEvent) {
                .type = eventtype_systemChanged,
                .system_change = {.ios_keyboardDown = true }
            });
        }];
    }
#endif
}

-(BOOL)acceptsFirstResponder {
    return YES;
}

-(void)checkWindowEvents {
    CoqEventWin event = CoqEvent_getNextTodoWindowEvent();
    while(event._todo) {
        switch(event.type & event_types_win_) {
#if TARGET_OS_OSX == 1
                // Events spécifiques à macOS
            case eventtype_win_mac_resize: {
                bool isFullScreen = [self.window styleMask] & NSWindowStyleMaskFullScreen;
                if(event.resize_info.fullScreen) {
                    if(isFullScreen) break;
                    [self.window toggleFullScreen:nil];
                    break;
                }
                if(isFullScreen) [self.window toggleFullScreen:nil];
                CGRect frame = rectangle_toCGRect(event.resize_info.framePt);
                [self.window setFrame:frame display:YES];
                break;
            }
            case eventtype_win_sendEmail: {
                NSSharingService* mailService = [NSSharingService sharingServiceNamed:NSSharingServiceNameComposeEmail];
                if(!mailService) { printerror("Cannot init email service."); break; }
                
                [mailService setRecipients:   @[[NSString stringWithUTF8String:event.email_info.recipient]]];
                [mailService setSubject:        [NSString stringWithUTF8String:event.email_info.subject]];
                [mailService performWithItems:@[[NSString stringWithUTF8String:event.email_info.body]]];
                break;
            }
//            case eventtype_win_ios_keyboardNeeded:
//            case eventtype_win_ios_keyboardNotNeeded:
            case eventtype_win_ios_scrollviewNotNeeded:
            case eventtype_win_ios_scrollviewNeeded:
            case eventtype_win_ios_scrollviewDisable_:
                printwarning("Received an iOS win event : %#x.", event.type);
                break; // (pass)
            
#else
            case eventtype_win_mac_resize:
                printwarning("macOS win event : %#x.", event.type);
                break;
                // Event spécifiques à iOS
            case eventtype_win_sendEmail: {
                if(![MFMailComposeViewController canSendMail]) {
                    printerror("Cannot send mail.");
                    break;
                }
                MFMailComposeViewController* mail = [[MFMailComposeViewController alloc] init];
                [mail setMailComposeDelegate:viewController];
                [mail setToRecipients:@[[NSString stringWithUTF8String:event.email_info.recipient]]];
                [mail setSubject:        [NSString stringWithUTF8String:event.email_info.subject]];
                [mail setMessageBody:[NSString stringWithUTF8String:event.email_info.body] isHTML:NO];
                [viewController presentViewController:mail animated:YES completion:nil];
                break;
            }
//            case eventtype_win_ios_keyboardNeeded: {
//                [self activateDummyTextField];
//                break;
//            }
//            case eventtype_win_ios_keyboardNotNeeded: {
//                [self deactivateDummyTextField];
//                break;
//            }
            case eventtype_win_ios_scrollviewNeeded: {
                CGRect rect = rectangle_toCGRect(event.win_scrollViewInfo.rect);
                CGFloat factor = (CGFloat)event.win_scrollViewInfo.contentFactor;
                CGFloat offset = (CGFloat)event.win_scrollViewInfo.offSetRatio;
                if(_dummyScrollView != nil) [_dummyScrollView removeFromSuperview];
                _dummyScrollView = [[DummyScrollView alloc] initWithFrame:rect contentFactor:factor offSet:offset metalView:self];
                [self addSubview:_dummyScrollView];
                break;
            }
            case eventtype_win_ios_scrollviewNotNeeded: {
                if(_dummyScrollView != nil) [_dummyScrollView removeFromSuperview];
                _dummyScrollView = nil;
                break;
            }
            case eventtype_win_ios_scrollviewDisable_: {
                if(self.dummyScrollView != nil)
                    [self.dummyScrollView setScrollEnabled:NO];
                break;
            }
            case eventtype_win_ios_fonts_install: {
                if (@available(iOS 13.0, *)) {
                    
//                    NSMutableArray* strArr = [[NSMutableArray alloc] init];
//                    for(int i = 0; i < event->win_font_list.count; i++) {
//                        [strArr addObject:[NSString stringWithUTF8String:event->win_font_list.stringArray[i]]];
//                    }
                    
                    NSURL *fontURL = [[NSBundle mainBundle] URLForResource:@"OpenDyslexic3-Regular.ttf" withExtension:nil subdirectory:@"fonts"];
                    NSArray* arr = [[NSArray alloc] initWithObjects:fontURL, nil];
                    CTFontManagerRegisterFontURLs((__bridge CFArrayRef)arr, kCTFontManagerScopePersistent, true, ^bool(CFArrayRef  _Nonnull errors, bool done) {
                        if(done) {
                            printdebug("Did Regiter Font!");
                        } else {
                            printwarning("Font not installed ?");
                        }
                        return true;
                    });
                } else {
                    printwarning("Need iOS 13 to install fonts.");
                }
                break;
            }
            case eventtype_win_ios_fonts_uninstall: {
                if (@available(iOS 13.0, *)) {
                    NSURL *fontURL = [[NSBundle mainBundle] URLForResource:@"OpenDyslexic3-Regular.ttf" withExtension:nil subdirectory:@"fonts"];
                    NSArray* arr = [[NSArray alloc] initWithObjects:fontURL, nil];
                    CTFontManagerUnregisterFontURLs((__bridge CFArrayRef)arr, kCTFontManagerScopePersistent, ^bool(CFArrayRef  _Nonnull errors, bool done) {
                        if(done) {
                            printdebug("Did Uninstall Font...");
                        } else {
                            printwarning("Cannot uninstall font ?");
                        }
                        return true;
                    });
                } else {
                    printwarning("Need iOS 13 to install/uninstall fonts.");
                }
                break;
            }
#endif
            case eventtype_win_cloudDrive_start: {
                CoqSystem_cloudDrive_startWatching_(event.cloudDrive_info.subFolderOpt, event.cloudDrive_info.extensionOpt);
            } break;
            case eventtype_win_cloudDrive_stop: {
                CoqSystem_cloudDrive_stopWatching_();
            } break;
            case eventtype_win_terminate: {
                [self setShouldTerminate:YES];
            } break;
            default: {
                printerror("Undefined win event %#010x.", event.type);
            }
        }
        
        event = CoqEvent_getNextTodoWindowEvent();
    }
}

/// Les events/timers à vérifier durant une `tick`.
-(BOOL)processTick {
    // 0. Checks...
    if(CoqEvent_timeSinceLastEventMS() > 8000)
        [self setPaused:YES];
    if(self.isPaused) { return false; }
    if([self shouldTerminate] || [self willTerminate]) { return false; }
    guard_let(Root*, root, CoqEvent_rootOpt, , false)
    // 1. Setter le temps de la tick.
    EventTimeCapture_update();
    ChronoChecker cc = chronochecker_startNew();
    // 2. Traiter tous les events (keydown, touchDown, ...)
    CoqEvent_processAllQueuedRootEvents();
//    chronochecker_tocWithComment(cc, "Events processed");
    // 3. Traiter les callback de timers, i.e. physique de collisions.
    Timer_check();
//    chronochecker_tocWithComment(cc, "Timers processed");
    // 5. Sleep s'il reste du temps.
    chronochecker_sleepRemaining(cc, EventTimeCapture.deltaTMS, false);
    
    return true;
}
-(void)startProcessTicksDispatchQueue {
    if(checkup_queue == NULL)
        checkup_queue = dispatch_queue_create("coqviewcheckup.queue", NULL);
    dispatch_async(checkup_queue, ^{
        while([self processTick]);
        // Terminate ?
        if([self shouldTerminate] && ![self willTerminate]) {
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

-(void)startDrawMissingPngsDispatchQueue {
    if(drawPng_queue == NULL)
        drawPng_queue = dispatch_queue_create("coqviewdrawpng.queue", NULL);
    dispatch_async(drawPng_queue, ^{
        Texture_drawMissingPngs();
    });
}

/// "Pause" en plus fort. Seuls les event comme "applicationDidBecomeActive" ou "didEnterFullScreenNotification"
/// peuvent entrer ou sortir de l'état "suspendu".
-(void)setSuspended:(BOOL)newSuspended {
    _suspended = newSuspended;
    [self setPaused:newSuspended];
}
-(void)setPaused:(BOOL)paused {
    // Ne peut sortir de pause si l'activité est suspendu...
    if(self.isSuspended && !paused) return;
    if(paused == self.isPaused) return;
    // Ok, changement...
    ChronoApp_setPaused(paused);
    [super setPaused:paused];
    [self setPreferredFramesPerSecond:paused ? 1 : 60];
    if(win_event_timer) [win_event_timer invalidate];
    win_event_timer = nil;
    if(paused) {
        return;
    }
    // Unpause
    if_let(Root*, root, CoqEvent_rootOpt)
    if(root->resumeAfterMSOpt) root->resumeAfterMSOpt(root, ChronoApp_lastSleepTimeMS());
    if_let_end
    [self startProcessTicksDispatchQueue];
    win_event_timer = [NSTimer scheduledTimerWithTimeInterval:0.03 repeats:true block:^(NSTimer * _Nonnull timer) {
        [self checkWindowEvents];
    }];
}
-(void)addEvent:(CoqEvent)coqEvent {
    // Laisser faire si l'app est suspendu.
    if([self isSuspended]) return;
    
    // Ajouter (avant de sortir de pause...)
    CoqEvent_addToRootEvent(coqEvent);
    
    // Réveiller l'app si en pause.
    [self setPaused:NO];
}

-(void)checksAfterRendererDraw {
    // Dans la thread de rendering...
    // S'il faut dessiner, partir une thread pour dessiner les pngs.
    if(Texture_needToDrawPngs()) {
        [self startDrawMissingPngsDispatchQueue];
    }
    // De temps en temps vider les poubelles et libérer les pngs inutilisés.
    if(RendererTimeCapture.tick % 2) return;
    Node_render_burnDownGarbage();
    if(RendererTimeCapture.tick % 100) return;
    Texture_render_releaseUnusedPngs();
}

// MARK: - Resizing view...
-(void)safeAreaInsetsDidChange {
    [self updateRootFrame: self.drawableSize dontFix:YES];
}

- (Margins)getMargins {
#if TARGET_OS_OSX == 1
    NSWindow*const window = [self window];
    bool const isFullScreen = [window styleMask] & NSWindowStyleMaskFullScreen;
    CGFloat const headerHeight = isFullScreen ?
        22 : window.frame.size.height - window.contentLayoutRect.size.height;
    return (Margins) { headerHeight, 0, 0, 0 };
#else
    return UIEdgeInsets_toMargins([self safeAreaInsets]);
#endif
}


- (void)updateRootFrame:(CGSize)sizePx dontFix:(BOOL)dontFix {
#if TARGET_OS_OSX == 1
    // Ok, le self.frame est déjà à jour quand on call drawableSizeWillChange du renderer...
    CoqEvent const coqevent = {
        .type = eventtype_resize, .resize_info = {
            .margins = [self getMargins],
            .framePt = CGRect_toRectangle(self.window.frame),
            .frameSizePx = CGSize_toVector2(sizePx), 
            .fullScreen = [self.window styleMask] & NSWindowStyleMaskFullScreen, 
            .justMoving = false, 
            .dontFix = dontFix,
    }};
#else
    // Marges a priori.
    Margins margins = UIEdgeInsets_toMargins([self safeAreaInsets]);
    // Bottom : vérifier s'il y a le clavier virtuel (et que l'on doit en tenir compte).
    if(!CoqSystem_dontResizeOnVirtualKeyboard && (self.keyboard_height > margins.bottom)) {
        margins.bottom = self.keyboard_height;
    }
    CoqEvent const coqevent = {
        eventtype_resize, .resize_info = {
            .margins = margins, 
            .framePt = CGRect_toRectangle(self.frame),
            .frameSizePx = CGSize_toVector2(sizePx),
            .fullScreen = true, 
            .justMoving = false, 
            .dontFix = dontFix,
    }};
#endif
    [self addEvent:coqevent];
}

#if TARGET_OS_OSX != 1
-(void)traitCollectionDidChange:(UITraitCollection *)previousTraitCollection {
    CoqSystem_OS_appearanceUpdate();
}
#endif

/*-- Input Events --------------------------------------*/
#if TARGET_OS_OSX == 1
// MARK: - macOS Mouse events
/*-- Mouse ---------------------------------------------*/
-(void)mouseEntered:(NSEvent *)event {
    // TODO: Event d'entrée et sortie de la sourie ?
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
    [self addEvent:(CoqEvent){
        .type = eventtype_touch_hovering,
        .touch_info = {
            .pos = { event.locationInWindow.x, 
                     event.locationInWindow.y },
        },
    }];
}

-(void)rightMouseDown:(NSEvent *)event {
    [self addEvent:(CoqEvent){
        .type = eventtype_touch_down,
        .touch_info = {
            .pos = { event.locationInWindow.x, 
                     event.locationInWindow.y },
            .touchId = 1,
        },
    }];
}
-(void)rightMouseDragged:(NSEvent *)event {
    [self addEvent:(CoqEvent){
        .type = eventtype_touch_drag,
        .touch_info = {
            .pos = { event.locationInWindow.x, 
                     event.locationInWindow.y },
            .touchId = 1,
        },
    }];
}
-(void)rightMouseUp:(NSEvent *)event {
    [self addEvent:(CoqEvent){
        .type = eventtype_touch_up,
        .touch_info = {
            .pos = { event.locationInWindow.x, 
                     event.locationInWindow.y },
            .touchId = 1,
        },
    }];
}
-(void)mouseDown:(NSEvent *)event {
    [self addEvent:(CoqEvent){
        .type = eventtype_touch_down,
        .touch_info = {
            .pos = { event.locationInWindow.x, 
                     event.locationInWindow.y },
        },
    }];
}
-(void)mouseDragged:(NSEvent *)event {
    [self addEvent:(CoqEvent){
        .type = eventtype_touch_drag,
        .touch_info = {
            .pos = { event.locationInWindow.x, 
                     event.locationInWindow.y },
        },
    }];
}
-(void)mouseUp:(NSEvent *)event {
    [self addEvent:(CoqEvent){
        .type = eventtype_touch_up,
        .touch_info = {
            .pos = { event.locationInWindow.x, 
                     event.locationInWindow.y },
        },
    }];
}
-(void)scrollWheel:(NSEvent *)event {
    // (slidingmenu a sa propre "inertie", si != 0 -> on a un "NSEventPhase" que l'on ignore)
    if(event.momentumPhase != 0)
        return;
    CoqEvent coq_event = { 0 };
    switch(event.phase) {
        case NSEventPhaseNone:
//            printdebug("Mouse wheel scroll: Dx %f, Dy %f, precise deltas %d.",
//                event.scrollingDeltaX, event.scrollingDeltaY, event.hasPreciseScrollingDeltas);
            if(event.hasPreciseScrollingDeltas) {
                coq_event = (CoqEvent) {
                    .type = eventtype_scroll,
                    .scroll_info = (ScrollInfo) {
                        .scrollType = scrolltype_track,
                        .scroll_deltas = { event.scrollingDeltaX, event.scrollingDeltaY },
                    },
                };
                break;
            }
            coq_event = (CoqEvent) {
                .type = eventtype_scroll,
                .scroll_info = (ScrollInfo) {
                    .scrollType = scrolltype_scroll,
                    .scroll_deltas = { event.scrollingDeltaX, event.scrollingDeltaY },
                },
            };
            break;
        case NSEventPhaseBegan:
            coq_event = (CoqEvent) {
                .type = eventtype_scroll,
                .scroll_info = {
                    .scrollType = scrolltype_trackBegin,
                }
            };
            break;
        case NSEventPhaseChanged:
            coq_event = (CoqEvent) {
                .type = eventtype_scroll,
                .scroll_info = {
                    .scrollType = scrolltype_track,
                    .scroll_deltas = { event.scrollingDeltaX, event.scrollingDeltaY },
                }
            };
            break;
        case NSEventPhaseEnded:
            coq_event = (CoqEvent) {
                .type = eventtype_scroll,
                .scroll_info = { .scrollType = scrolltype_trackEnd, },
            };
            break;
        default:
            coq_event.type = 0;
            break;
    }
    if(coq_event.type)
        [self addEvent:coq_event];
}

#else

// MARK: - iOS Touch events
// **-- Touches (similaire a mouse)----------------------------------------

-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(touches.count == 0) return;
    UITouch* touch = [touches anyObject];
    CGPoint location = [touch locationInView:self];
    [self addEvent:(CoqEvent){
        .type = eventtype_touch_down,
        .touch_info = {
            .pos = { location.x, 
                     location.y },
            .yInverted = true,
        },
    }];
}
-(void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(touches.count == 0) return;
    UITouch* touch = [touches anyObject];
    CGPoint location = [touch locationInView:self];
    [self addEvent:(CoqEvent){
        .type = eventtype_touch_drag,
        .touch_info = {
            .pos = { location.x, 
                     location.y },
            .yInverted = true,
        },
    }];
    
}
-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(touches.count == 0) return;
    if(self.dummyScrollView != nil)
        [self.dummyScrollView setScrollEnabled:YES];
    [self addEvent:(CoqEvent){.type = eventtype_touch_up}];
}
-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(touches.count == 0) return;
    if(self.dummyScrollView != nil)
        [self.dummyScrollView setScrollEnabled:YES];
    [self addEvent:(CoqEvent){.type = eventtype_touch_up}];
}

#endif

// MARK: - macOS Keyboard events
// **-- Keyboard input ---------------------------------------------
#if TARGET_OS_OSX == 1
-(void)keyDown:(NSEvent *)event {
    if(event.isARepeat && CoqEvent_ignoreRepeatKeyDown) return;
    uint32_t mods = (uint32_t)event.modifierFlags;
    uint16_t keycode = event.keyCode;
    uint16_t mkc = MKC_of_keycode[keycode];
//    const char* c_str = [event.characters UTF8String];
    CoqEvent coqEvent = { 
        .type = eventtype_key_down,
        .key = {mods, keycode, mkc, false },
    };
//    if(c_str) strncpy(coqEvent.key.typed.c_str, c_str, CHARACTER_MAX_SIZE);
    [self addEvent:coqEvent];
}
-(void)keyUp:(NSEvent *)event {
    if(event.isARepeat && CoqEvent_ignoreRepeatKeyDown) return;
    uint32_t mods = (uint32_t)event.modifierFlags;
    uint16_t keycode = event.keyCode;
    uint16_t mkc = MKC_of_keycode[keycode];
//    const char* c_str = [event.characters UTF8String];
    CoqEvent coqEvent = { 
        .type = eventtype_key_up,
        .key = {mods, keycode, mkc, false },
    };
//    if(c_str) strncpy(coqEvent.key.typed.c_str, c_str, CHARACTER_MAX_SIZE);
    [self addEvent:coqEvent];
}
-(void)flagsChanged:(NSEvent *)event {
    CoqEvent coqEvent = {
        .type = eventtype_key_mod,
        .key = { .modifiers = (uint32_t)event.modifierFlags },
    };
    [self addEvent:coqEvent];
}
#else
// MARK: - iOS Keyboard events
-(void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    [super pressesBegan:presses withEvent:event];
    if(@available(iOS 13.4, *)) {for(UIPress* press in presses) {
        UIKey*const key = press.key;
        uint16_t const keycode = key.keyCode;
        uint32_t const event_type = (keycode >= 0xE0 && keycode <= 0xE7) ?
                                eventtype_key_mod : eventtype_key_down;
        CoqEvent const coqevent = {
            .type = event_type,
            .key = {
                .modifiers = (uint32_t)key.modifierFlags,
                .keycode = keycode,
                .mkc = MKC_of_keycode[keycode],
            },
        };
//        const char*const c_str = [key.characters UTF8String];
//        if(c_str) strncpy(coqevent.key.typed.c_str, c_str, CHARACTER_MAX_SIZE);
        [self addEvent:coqevent];
    }}
}
-(void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    [super pressesEnded:presses withEvent:event];
    if(@available(iOS 13.4, *)) {for(UIPress* press in presses) {
        UIKey *key = press.key;
        uint32_t const mods = (uint32_t)key.modifierFlags & (~modifier_capslock);
        uint16_t const keycode = key.keyCode;
        if(keycode == keycode_capsLock) continue;
        uint32_t const event_type = (keycode >= 0xE0 && keycode <= 0xE7) ?
                                eventtype_key_mod : eventtype_key_up;
        CoqEvent coqevent = {
            .type = event_type,
            .key = {
                .modifiers = mods,
                .keycode = keycode,
                .mkc = MKC_of_keycode[keycode],
            },
        };
        [self addEvent:coqevent];
    }}
}
-(void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    [super pressesCancelled:presses withEvent:event];
    if(@available(iOS 13.4, *)) {for(UIPress* press in presses) {
//#warning A tester.
        UIKey *const key = press.key;
        uint32_t const mods = (uint32_t)key.modifierFlags & (~modifier_capslock);
        uint16_t const keycode = key.keyCode;
        if(keycode == keycode_capsLock) continue;
        uint32_t const event_type = (keycode >= 0xE0 && keycode <= 0xE7) ?
                                eventtype_key_mod : eventtype_key_up;
        CoqEvent coqevent = {
            .type = event_type,
            .key = {
                .modifiers = mods,
                .keycode = keycode,
                .mkc = MKC_of_keycode[keycode],
            },
        };
        [self addEvent:coqevent];
    }}
}

// Dummy text field pour keyboard event avec iOS < 13.5...
//-(void)activateDummyTextField {
//    if(self.dummyTextField == nil) return;
//    if([self.dummyTextField isFirstResponder]) return;
//    
//    [self.dummyTextField becomeFirstResponder];
//}
//-(void)deactivateDummyTextField {
//    if(self.dummyTextField == nil) return;
//    if(![self.dummyTextField isFirstResponder]) return;
//    
//    // Il faut `unlock` avant de resigner.
//    [self.dummyTextField setLocked:NO];
//    [self.dummyTextField resignFirstResponder];
//}

#endif

@end
