//
//  util_apple.h
//  Conversion pratique avec les structure CoreFoundation de Apple.
//  Init de d'un menu et window de base
//  (CG : Core Graphics)
//  (UI : User interface)
//
//  Created by Corentin Faucher on 2023-12-08.
//

#ifndef COQ_UTIL_APPLE_H
#define COQ_UTIL_APPLE_H

#include "math_base.h"
// Juste les import pour avoir les types CGRect, CGSize, MTLClearColor, UIEdgeInsets.
#import <CoreFoundation/CFCGTypes.h>
#import <Metal/MTLRenderPass.h>

#pragma mark - Conversion pratiques
CGRect    rectangle_toCGRect(Rectangle rect);
CGSize    vector2_toCGSize(Vector2 v);
Rectangle CGRect_toRectangle(CGRect rect);
Vector2   CGSize_toVector2(CGSize size);
MTLClearColor vector4_toMTLClearColor(Vector4 v);
#if TARGET_OS_OSX != 1
#import <UIKit/UIGeometry.h>
Margins   UIEdgeInsets_toMargins(UIEdgeInsets m);
#endif

#pragma mark - NSApp et NSWindow: Default NSWindow et menu.
#if TARGET_OS_OSX == 1
#import <AppKit/AppKit.h>
void      NSApp_createDefaultMenu(void);
NSWindow* NSWindow_createDefault(NSString* defaultName, float const fixedRatioOpt);
#endif

#endif /* bundle_util_h */
