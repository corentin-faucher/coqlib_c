//
//  bundle_utils.h
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//

#ifndef COQ_UTILS_APPLE_H
#define COQ_UTILS_APPLE_H

#include "maths/math_base.h"
#import <CoreFoundation/CFCGTypes.h>


/// Vérifie le layout courant dans macOS.
/// Doit être callé dans la thread principale.
void        CoqSystem_updateCurrentLayout(void);
const char* CoqSystem_currentLayoutOpt(void);
void        CoqSystem_updateCurrentTheme(void);
bool        CoqSystem_currentThemeIsDark(void);

CGRect  rectangle_toCGRect(Rectangle rect);
CGSize  vector2_toCGSize(Vector2 v);
Rectangle CGRect_toRectangle(CGRect rect);
Vector2  CGSize_toVector2(CGSize size);
#if TARGET_OS_OSX != 1
Margins UIEdgeInsets_toMargins(UIEdgeInsets m) {
    return (Margins) { m.top, m.left, m.bottom, m.right };
}
#endif

#endif /* bundle_utils_h */
