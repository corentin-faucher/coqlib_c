//
//  SceneDelegate.m
//  coqlib_testIOS
//
//  Created by Corentin Faucher on 2026-01-09.
//

#import "ios_SceneDelegate.h"

#include "systems/system_sound.h"
#import "apple_view_metal.h"

@interface SceneDelegateBase ()

@end

@implementation SceneDelegateBase


- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
    // Use this method to optionally configure and attach the UIWindow `window` to the provided UIWindowScene `scene`.
    // If using a storyboard, the `window` property will automatically be initialized and attached to the scene.
    // This delegate does not imply the connecting scene or session are new (see `application:configurationForConnectingSceneSession` instead).
}


- (void)sceneDidDisconnect:(UIScene *)scene {
    // Called as the scene is being released by the system.
    // This occurs shortly after the scene enters the background, or when its session is discarded.
    // Release any resources associated with this scene that can be re-created the next time the scene connects.
    // The scene may re-connect later, as its session was not necessarily discarded (see `application:didDiscardSceneSessions` instead).
}


- (void)sceneDidBecomeActive:(UIScene *)scene {
    // Called when the scene has moved from an inactive state to an active state.
    // Use this method to restart any tasks that were paused (or not yet started) when the scene was inactive.
    
}


- (void)sceneWillResignActive:(UIScene *)scene {
    // Called when the scene will move from an active state to an inactive state.
    // This may occur due to temporary interruptions (ex. an incoming phone call).
    guard_let(CoqMetalView*, metalView, 
        uiview_asCoqMetalViewOpt([[self.window rootViewController] view]), , )
    [metalView setSuspended:YES];
}


- (void)sceneWillEnterForeground:(UIScene *)scene {
    // Called as the scene transitions from the background to the foreground.
    // Use this method to undo the changes made on entering the background.
    Texture_resume();
    Sound_resume_();
    guard_let(CoqMetalView*, metalView, 
        uiview_asCoqMetalViewOpt([[self.window rootViewController] view]), , )
    [metalView setSuspended:NO];
}


- (void)sceneDidEnterBackground:(UIScene *)scene {
    // Called as the scene transitions from the foreground to the background.
    // Use this method to save data, release shared resources, and store enough scene-specific state information
    // to restore the scene back to its current state.
    Texture_suspend();
    Sound_suspend_();
}


@end
