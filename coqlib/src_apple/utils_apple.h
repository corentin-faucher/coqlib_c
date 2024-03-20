//
//  utils_apple.h
//  Conversion pratique avec les structure CoreFoundation de Apple.
//  (CG : Core Graphics)
//  (UI : User interface)
//
//  Created by Corentin Faucher on 2023-12-08.
//

#ifndef COQ_UTILS_APPLE_H
#define COQ_UTILS_APPLE_H

#include "maths/math_base.h"
#import <CoreFoundation/CFCGTypes.h>
#if TARGET_OS_OSX != 1
#import <UIKit/UIKit.h>
#endif

CGRect    rectangle_toCGRect(Rectangle rect);
CGSize    vector2_toCGSize(Vector2 v);
Rectangle CGRect_toRectangle(CGRect rect);
Vector2   CGSize_toVector2(CGSize size);
#if TARGET_OS_OSX != 1
Margins   UIEdgeInsets_toMargins(UIEdgeInsets m);
#endif

#endif /* bundle_utils_h */
