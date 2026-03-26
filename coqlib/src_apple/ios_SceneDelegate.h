//  ios_SceneDelegate.h
//
// Exemple de SceneDelegate de base pour iOS
// à utiliser avec coqlib.
// Load et unload les textures et bruits quand l'app
// est active.
// Envoie le signal "suspended" à la MetalView.
//
// Corentin Faucher
// 2026-01-09


#import <UIKit/UIKit.h>

@interface SceneDelegateBase : UIResponder <UIWindowSceneDelegate>

@property (strong, nonatomic) UIWindow * window;

@end

