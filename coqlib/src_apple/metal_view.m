//
//  MTKView+CoqMetalView.m
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-26.
//
#import "metal_view.h"
#include "coq_timer.h"
#include "coq_event.h"

#include "util_apple.h"

#import <CloudKit/CloudKit.h>
#import <GameController/GameController.h>
#if TARGET_OS_OSX == 1
#import <Carbon/Carbon.h>
//@interface CoqMetalView()
//@end
#else
#import "ios_dummyTextField.h"
#import "ios_dummy_scrollview.h"
#import <CoreText/CoreText.h>
#import <MessageUI/MessageUI.h>

@interface CoqMetalView ()

@property (strong, nonatomic) DummyTextField *   dummyTextField;
//@property (strong, nonatomic) DummySpace *         dummySpace;
@property (strong, nonatomic) DummyScrollView *  dummyScrollView;
@property CGFloat keyboard_height;

@end
#endif

@implementation CoqMetalView

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device {
    self = [super initWithFrame:frameRect device:device];
    [self setUpRendererAndNotifications];
    return self;
}
- (void)awakeFromNib {
    [super awakeFromNib];
    [self setDevice:MTLCreateSystemDefaultDevice()];
    [self setUpRendererAndNotifications];
}
-(void)setUpRendererAndNotifications {
    renderer = [[Renderer alloc] initWithView:self];
    [self setDelegate:renderer];
    [self setPaused:YES];
    chronochecker_set(&(self->cc));
#if TARGET_OS_OSX == 1
    // Notifications
    {
        // Notifications Full screen (pause unpause)
        [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowWillEnterFullScreenNotification
                                                          object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            [self setPaused:YES];
        }];
        [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidEnterFullScreenNotification
                                                          object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            [self setPaused:NO];
        }];
        [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowWillExitFullScreenNotification
                                                          object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            [self setPaused:YES];
        }];
        [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidExitFullScreenNotification
                                                          object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            [self setPaused:NO];
        }];
        // Notification Window Move (pour resize -> voir updateRootFrame)
        [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidMoveNotification
                                                          object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            NSWindow* window = [self window];
            bool isFullScreen = [window styleMask] & NSWindowStyleMaskFullScreen;
            CGFloat headerHeight = isFullScreen ?
            22 : window.frame.size.height - window.contentLayoutRect.size.height;
            // Ok, le self.frame est déjà à jour quand on call drawableSizeWillChange du renderer...
            CoqEvent_addToRootEvent((CoqEvent){
                event_type_resize, .resize_info = {
                    { headerHeight, 0, 0, 0 },
                    CGRect_toRectangle(window.frame),
                    CGSize_toVector2([self drawableSize]),
                    isFullScreen, true, false,
                }});
        }];
        // Notification Layout, theme, language
        [[NSDistributedNotificationCenter defaultCenter]
         addObserverForName:(__bridge NSString*)kTISNotifySelectedKeyboardInputSourceChanged
         object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            CoqSystem_layoutUpdate();
            CoqEvent_addToRootEvent((CoqEvent) { event_type_systemChanged, .system_change = { .layoutDidChange = true }});
        }];
        [[NSDistributedNotificationCenter defaultCenter]
         addObserverForName:@"AppleInterfaceThemeChangedNotification"
         object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            CoqSystem_theme_OsThemeUpdate();
            CoqEvent_addToRootEvent((CoqEvent) { event_type_systemChanged, .system_change = { .themeDidChange = true }});
        }];
        [[NSDistributedNotificationCenter defaultCenter]
         addObserverForName:@"AppleLanguagePreferencesChangedNotification"
         object:nil queue:nil usingBlock:^(NSNotification * _Nonnull note) {
            CoqLocale_update();
            Language_checkSystemLanguage();
            CoqEvent_addToRootEvent((CoqEvent) { event_type_systemChanged, .system_change = { .languageRegionDidChange = true }});
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
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_dpad, .vector = { xValue, yValue }};
                    CoqEvent_addToRootEvent((CoqEvent) { event_type_gamePad_value, .gamepadInput = input });
                }];
                [[gamePad leftThumbstick] setValueChangedHandler:^(GCControllerDirectionPad * _Nonnull dpad, float xValue, float yValue) {
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_JoystickLeft, .buttonValue = gamePad.leftThumbstickButton.value, .vector = { xValue, yValue }};
                    CoqEvent_addToRootEvent((CoqEvent) { event_type_gamePad_value, .gamepadInput = input });
                }];
                [[gamePad rightThumbstick] setValueChangedHandler:^(GCControllerDirectionPad * _Nonnull dpad, float xValue, float yValue) {
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_JoystickRight, .buttonValue = gamePad.rightThumbstickButton.value, .vector = { xValue, yValue }};
                    CoqEvent_addToRootEvent((CoqEvent) { event_type_gamePad_value, .gamepadInput = input });
                }];
                [[gamePad buttonA] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_A, value };
                    CoqEvent_addToRootEvent((CoqEvent) { pressed ? event_type_gamePad_down : event_type_gamePad_up, .gamepadInput = input });
                }];
                [[gamePad buttonB] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_B, value };
                    CoqEvent_addToRootEvent((CoqEvent) { pressed ? event_type_gamePad_down : event_type_gamePad_up, .gamepadInput = input });
                }];
                [[gamePad buttonX] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_X, value };
                    CoqEvent_addToRootEvent((CoqEvent) { pressed ? event_type_gamePad_down : event_type_gamePad_up, .gamepadInput = input });
                }];
                [[gamePad buttonY] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_Y, value };
                    CoqEvent_addToRootEvent((CoqEvent) { pressed ? event_type_gamePad_down : event_type_gamePad_up, .gamepadInput = input });
                }];
                [[gamePad rightTrigger] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_ZR, value };
                    CoqEvent_addToRootEvent((CoqEvent) { pressed ? event_type_gamePad_down : event_type_gamePad_up, .gamepadInput = input });
                }];
                [[gamePad leftTrigger] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_ZL, value };
                    CoqEvent_addToRootEvent((CoqEvent) { pressed ? event_type_gamePad_down : event_type_gamePad_up, .gamepadInput = input });
                }];
                [[gamePad buttonMenu] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_Plus, value };
                    CoqEvent_addToRootEvent((CoqEvent) { pressed ? event_type_gamePad_down : event_type_gamePad_up, .gamepadInput = input });
                }];
                [[gamePad buttonOptions] setPressedChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
                    [self setPaused:NO]; if(self.isPaused) return;
                    GamePadInput input = { gamepad_Minus, value };
                    CoqEvent_addToRootEvent((CoqEvent) { pressed ? event_type_gamePad_down : event_type_gamePad_up, .gamepadInput = input });
                }];
            }];
        }
    }
#else
    bool ios13available = false;
    if(@available(iOS 13.4, *)) {
        ios13available = true;
    }
    if(!ios13available || self.iosForceVirtualKeyboard) {
        self.dummyTextField = [[DummyTextField alloc]
                               initWithFrame:CGRectMake(0, 60, 120, 30)
                               andMetalView:self];
        [self addSubview:self.dummyTextField];
        
//        printdebug("Adding dummy text field %s.", [[self.dummyTextField description] UTF8String]);
    }
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
                .type = event_type_systemChanged,
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
                .type = event_type_systemChanged,
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
            case event_type_win_mac_resize: {
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
            case event_type_win_sendEmail: {
                NSSharingService* mailService = [NSSharingService sharingServiceNamed:NSSharingServiceNameComposeEmail];
                if(!mailService) { printerror("Cannot init email service."); break; }
                
                [mailService setRecipients:   @[[NSString stringWithUTF8String:event.email_info.recipient]]];
                [mailService setSubject:        [NSString stringWithUTF8String:event.email_info.subject]];
                [mailService performWithItems:@[[NSString stringWithUTF8String:event.email_info.body]]];
                break;
            }
            case event_type_win_ios_keyboardNeeded:
            case event_type_win_ios_keyboardNotNeeded:
            case event_type_win_ios_scrollviewNotNeeded:
            case event_type_win_ios_scrollviewNeeded:
            case event_type_win_ios_scrollviewDisable_:
                printwarning("Received an iOS win event : %#x.", event.type);
                break; // (pass)
            
#else
            case event_type_win_mac_resize:
                printwarning("macOS win event : %#x.", event.type);
                break;
                // Event spécifiques à iOS
            case event_type_win_sendEmail: {
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
            case event_type_win_ios_keyboardNeeded: {
                [self activateDummyTextField];
                break;
            }
            case event_type_win_ios_keyboardNotNeeded: {
                [self deactivateDummyTextField];
                break;
            }
            case event_type_win_ios_scrollviewNeeded: {
                CGRect rect = rectangle_toCGRect(event.win_scrollViewInfo.rect);
                CGFloat factor = (CGFloat)event.win_scrollViewInfo.contentFactor;
                CGFloat offset = (CGFloat)event.win_scrollViewInfo.offSetRatio;
                if(_dummyScrollView != nil) [_dummyScrollView removeFromSuperview];
                _dummyScrollView = [[DummyScrollView alloc] initWithFrame:rect contentFactor:factor offSet:offset metalView:self];
                [self addSubview:_dummyScrollView];
                break;
            }
            case event_type_win_ios_scrollviewNotNeeded: {
                if(_dummyScrollView != nil) [_dummyScrollView removeFromSuperview];
                _dummyScrollView = nil;
                break;
            }
            case event_type_win_ios_scrollviewDisable_: {
                if(self.dummyScrollView != nil)
                    [self.dummyScrollView setScrollEnabled:NO];
                break;
            }
            case event_type_win_ios_fonts_install: {
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
            case event_type_win_ios_fonts_uninstall: {
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
            case event_type_win_cloudDrive_start: {
                CoqSystem_cloudDrive_startWatching_(event.cloudDrive_info.subFolderOpt, event.cloudDrive_info.extensionOpt);
            } break;
            case event_type_win_cloudDrive_stop: {
                CoqSystem_cloudDrive_stopWatching_();
            } break;
            default: {
                printerror("Undefined win event %#010x.", event.type);
            }
        }
        
        event = CoqEvent_getNextTodoWindowEvent();
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
                if(chronochecker_elapsedMS(&cc) > 200)
                    printwarning("Overwork? %lld ms.", chronochecker_elapsedMS(&cc)); 
                continue;
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
    [self setPreferredFramesPerSecond:paused ? 1 : 60];
    if(win_event_timer) [win_event_timer invalidate];
    win_event_timer = nil;
    if(paused) {
        return;
    }
    // Unpause
    if(root) if(root->resumeAfterMSOpt) root->resumeAfterMSOpt(root, ChronoApp_lastSleepTimeMS());
    [self startCheckUpDispatchQueue];
    win_event_timer = [NSTimer scheduledTimerWithTimeInterval:0.03 repeats:true block:^(NSTimer * _Nonnull timer) {
        [self checkWindowEvents];
    }];
}

#pragma mark - Resizing view...

-(void)safeAreaInsetsDidChange {
    [self updateRootFrame: self.drawableSize dontFix:YES];
}
- (void)updateRootFrame:(CGSize)sizePx dontFix:(BOOL)dontFix {
    if(!root) { return; }
    [self setPaused:NO];
#if TARGET_OS_OSX == 1
    NSWindow* window = [self window];
    bool isFullScreen = [window styleMask] & NSWindowStyleMaskFullScreen;
    CGFloat headerHeight = isFullScreen ?
        22 : window.frame.size.height - window.contentLayoutRect.size.height;
    // Ok, le self.frame est déjà à jour quand on call drawableSizeWillChange du renderer...
    CoqEvent_addToRootEvent((CoqEvent){
        event_type_resize, .resize_info = {
            { headerHeight, 0, 0, 0 },
            CGRect_toRectangle(window.frame),
            CGSize_toVector2(sizePx), isFullScreen, false, dontFix,
    }});
    window = nil;
#else
    // Marges a priori.
    Margins margins = UIEdgeInsets_toMargins([self safeAreaInsets]);
    // Bottom : vérifier s'il y a le clavier virtuel (et que l'on doit en tenir compte).
    if(!CoqSystem_dontResizeOnVirtualKeyboard && (self.keyboard_height > margins.bottom)) {
        margins.bottom = self.keyboard_height;
    }
    CoqEvent_addToRootEvent((CoqEvent){
        event_type_resize, .resize_info = {
            margins, CGRect_toRectangle(self.frame),
            CGSize_toVector2(sizePx), true, false, dontFix,
    }});
#endif
}

#if TARGET_OS_OSX != 1
-(void)traitCollectionDidChange:(UITraitCollection *)previousTraitCollection {
    CoqSystem_theme_OsThemeUpdate();
}
#endif

/*-- Input Events --------------------------------------*/
#if TARGET_OS_OSX == 1
#pragma mark - macOS Mouse events
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
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    
    CoqEvent coq_event = {
        .type = event_type_touch_hovering, 
        .touch_pos = root_absposFromViewPos(root, viewPos, false)
    };
    CoqEvent_addToRootEvent(coq_event);
}

-(void)mouseDown:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    
    CoqEvent coq_event = {
        .type = event_type_touch_down, 
        .touch_pos = root_absposFromViewPos(root, viewPos, false)
    };
    CoqEvent_addToRootEvent(coq_event);
}
-(void)mouseDragged:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    NSPoint viewPosNSP = event.locationInWindow;
    Vector2 viewPos = { viewPosNSP.x, viewPosNSP.y };
    CoqEvent coq_event = {
        .type = event_type_touch_drag, 
        .touch_pos = root_absposFromViewPos(root, viewPos, false)
    };
    CoqEvent_addToRootEvent(coq_event);
}
-(void)mouseUp:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    CoqEvent coq_event = {
        .type = event_type_touch_up,
    };
    CoqEvent_addToRootEvent(coq_event);
}
-(void)scrollWheel:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    // (slidingmenu a sa propre "inertie", si != 0 -> on a un "NSEventPhase" que l'on ignore)
    if(event.momentumPhase != 0)
        return;
    CoqEvent coq_event = { 0 };
    switch(event.phase) {
        case NSEventPhaseNone:
            coq_event.type = event_type_scroll;
            coq_event.scroll_info = (ScrollInfo){
                .scrollType = scroll_type_scroll,
                .scroll_deltas = {event.scrollingDeltaX, event.scrollingDeltaY}
            };
            break;
        case NSEventPhaseBegan:
            coq_event = (CoqEvent) {
                .type = event_type_scroll,
                .scroll_info = {
                    .scrollType = scroll_type_trackBegin,
                }
            };
            break;
        case NSEventPhaseChanged:
            coq_event = (CoqEvent) {
                .type = event_type_scroll,
                .scroll_info = {
                    .scrollType = scroll_type_track,
                    .scroll_deltas = { event.scrollingDeltaX, event.scrollingDeltaY },
                }
            };
            break;
        case NSEventPhaseEnded:
            coq_event = (CoqEvent) {
                .type = event_type_scroll,
                .scroll_info = { .scrollType = scroll_type_trackEnd, },
            };
            break;
        default:
            coq_event.type = 0;
            break;
    }
    if(coq_event.type)
        CoqEvent_addToRootEvent(coq_event);
}

#else

#pragma mark - iOS Touch events
// **-- Touches (similaire a mouse)----------------------------------------

-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    UITouch* touch = [touches anyObject];
    CGPoint location = [touch locationInView:self];
    Vector2 viewPos = { location.x, location.y };
    
    CoqEvent coq_event = {
        .type = event_type_touch_down,
        .touch_pos = root_absposFromViewPos(root, viewPos, true),
    };
    CoqEvent_addToRootEvent(coq_event);
}
-(void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    UITouch* touch = [touches anyObject];
    CGPoint location = [touch locationInView:self];
    Vector2 viewPos = { location.x, location.y };
    
    CoqEvent coq_event = {
        .type = event_type_touch_drag,
        .touch_pos = root_absposFromViewPos(root, viewPos, true),
    };
    CoqEvent_addToRootEvent(coq_event);
    
}
-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    if(self.dummyScrollView != nil)
        [self.dummyScrollView setScrollEnabled:YES];
    CoqEvent coq_event = { .type = event_type_touch_up };
    CoqEvent_addToRootEvent(coq_event);
}
-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if(root == NULL || touches.count == 0) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    if(self.dummyScrollView != nil)
        [self.dummyScrollView setScrollEnabled:YES];
    CoqEvent coq_event = { .type = event_type_touch_up };
    CoqEvent_addToRootEvent(coq_event);
}

#endif

#pragma mark - macOS Keyboard events
// **-- Keyboard input ---------------------------------------------
#if TARGET_OS_OSX == 1
-(void)keyDown:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    if(event.isARepeat && CoqEvent_ignoreRepeatKeyDown) return;
    uint32_t mods = (uint32_t)event.modifierFlags;
    uint16_t keycode = event.keyCode;
    uint16_t mkc = MKC_of_keycode[keycode];
    const char* c_str = [event.characters UTF8String];
    CoqEvent coqEvent = { 
        .type = event_type_key_down,
        .key = {mods, keycode, mkc, false },
    };
    if(c_str) strncpy(coqEvent.key.typed.c_str, c_str, CHARACTER_MAX_SIZE);
    CoqEvent_addToRootEvent(coqEvent);
}
-(void)keyUp:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    if(event.isARepeat && CoqEvent_ignoreRepeatKeyDown) return;
    uint32_t mods = (uint32_t)event.modifierFlags;
    uint16_t keycode = event.keyCode;
    uint16_t mkc = MKC_of_keycode[keycode];
    const char* c_str = [event.characters UTF8String];
    CoqEvent coqEvent = { 
        .type = event_type_key_up,
        .key = {mods, keycode, mkc, false },
    };
    if(c_str) strncpy(coqEvent.key.typed.c_str, c_str, CHARACTER_MAX_SIZE);
    
    CoqEvent_addToRootEvent(coqEvent);
}
-(void)flagsChanged:(NSEvent *)event {
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    CoqEvent coqEvent = {
        .type = event_type_key_mod,
        .key = { .modifiers = (uint32_t)event.modifierFlags },
    };
    CoqEvent_addToRootEvent(coqEvent);
}
#else
#pragma mark - iOS Keyboard events
-(void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    [super pressesBegan:presses withEvent:event];
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    if(@available(iOS 13.4, *)) {for(UIPress* press in presses) {
        UIKey *key = press.key;
        uint32_t mods = (uint32_t)key.modifierFlags;
        uint16_t keycode = key.keyCode;
        uint16_t mkc = MKC_of_keycode[keycode];
        const char* c_str = [key.characters UTF8String];
        
        uint32_t event_type = (keycode >= 0xE0 && keycode <= 0xE7) ?
                                event_type_key_mod : event_type_key_down;
        CoqEvent coqevent = {
            .type = event_type,
            .key = { mods, keycode, mkc, false, }
        };
        if(c_str)
            strncpy(coqevent.key.typed.c_str, c_str, CHARACTER_MAX_SIZE);
        CoqEvent_addToRootEvent(coqevent);
    }}
}
-(void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    [super pressesEnded:presses withEvent:event];
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    if(@available(iOS 13.4, *)) {for(UIPress* press in presses) {
        UIKey *key = press.key;
        uint32_t mods = (uint32_t)key.modifierFlags & (~modifier_capslock);
        uint16_t keycode = key.keyCode;
        if(keycode == keycode_capsLock) continue;
        uint32_t event_type = (keycode >= 0xE0 && keycode <= 0xE7) ?
                                event_type_key_mod : event_type_key_up;
        CoqEvent coqevent = {
            .type = event_type,
            .key = { mods, keycode, MKC_of_keycode[keycode], false, }
        };
        CoqEvent_addToRootEvent(coqevent);
    }}
}
-(void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    [super pressesCancelled:presses withEvent:event];
    if(root == NULL) return;
    [self setPaused:NO];
    if(self.isPaused) return;
    if(@available(iOS 13.4, *)) {for(UIPress* press in presses) {
//#warning A tester.
        UIKey *key = press.key;
        uint32_t mods = (uint32_t)key.modifierFlags & (~modifier_capslock);
        uint16_t keycode = key.keyCode;
        if(keycode == keycode_capsLock) continue;
        uint32_t event_type = (keycode >= 0xE0 && keycode <= 0xE7) ?
                                event_type_key_mod : event_type_key_up;
        CoqEvent coqevent = {
            .type = event_type,
            .key = { mods, keycode, MKC_of_keycode[keycode], false, }
        };
        CoqEvent_addToRootEvent(coqevent);
    }}
}

// Dummy text field pour keyboard event avec iOS < 13.5...
-(void)activateDummyTextField {
    if(self.dummyTextField == nil) return;
    if([self.dummyTextField isFirstResponder]) return;
    
    [self.dummyTextField becomeFirstResponder];
}
-(void)deactivateDummyTextField {
    if(self.dummyTextField == nil) return;
    if(![self.dummyTextField isFirstResponder]) return;
    
    // Il faut `unlock` avant de resigner.
    [self.dummyTextField setLocked:NO];
    [self.dummyTextField resignFirstResponder];
}

#endif

@end
