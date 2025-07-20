//
//  macos_appdelegate.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-17.
//
#import "macos_appdelegate.h"

#include "systems/system_sound.h"
#include "systems/system_base.h"

@implementation AppDelegateBase

- (void)applicationWillBecomeActive:(NSNotification *)notification {
    Texture_resume();
    Sound_resume_();
}
- (void)applicationDidBecomeActive:(NSNotification *)notification {
    [view setSuspended:NO];
    [window makeFirstResponder:view];
}
- (void)applicationWillResignActive:(NSNotification *)notification {
    [view setSuspended:YES];
}
- (void)applicationDidResignActive:(NSNotification *)notification {
    Texture_suspend();
    Sound_suspend_();
}
- (void)applicationWillTerminate:(NSNotification *)aNotification {
    [view setWillTerminate:YES];
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}
-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}


@end
