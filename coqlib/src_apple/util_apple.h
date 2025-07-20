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

// MARK: - Conversion pratiques
static inline CGRect    rectangle_toCGRect(Rectangle const rect) {
    return (CGRect) { rect.o_x, rect.o_y, rect.w, rect.h };
}
static inline CGSize    vector2_toCGSize(Vector2 const v) {
    return (CGSize) { v.x, v.y };
}
static inline CGPoint    vector2_toCGPoint(Vector2 const v) {
    return (CGPoint) { v.x, v.y };
}
static inline Rectangle CGRect_toRectangle(CGRect const rect) {
    return (Rectangle) { rect.origin.x, rect.origin.y, rect.size.width, rect.size.height };
}
static inline Vector2   CGSize_toVector2(CGSize const size) {
    return (Vector2) { size.width, size.height };
}
static inline Vector2   CGPoint_toVector2(CGPoint const point) {
    return (Vector2) { point.x, point.y };
}
static inline MTLClearColor vector4_toMTLClearColor(Vector4 v) {
    return (MTLClearColor) { v.x, v.y, v.z, v.w };
}
#if TARGET_OS_OSX != 1
#import <UIKit/UIGeometry.h>
static inline Margins   UIEdgeInsets_toMargins(UIEdgeInsets m) { 
    return (Margins) { m.top, m.left, m.bottom, m.right };
]
#endif

// MARK: - NSApp et NSWindow: Default NSWindow et menu.
#if TARGET_OS_OSX == 1
#import <AppKit/AppKit.h>
/// Menu minimaliste macOS (juste `Quit` avec la localized string "quit").
NSMenu*   NSMenu_createDefault(void);
NSWindow* NSWindow_createDefault(NSString* defaultName, float const fixedRatioOpt);
#endif

#endif /* bundle_util_h */
