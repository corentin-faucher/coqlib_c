//
//  apple_utils.m
//
//  Created by Corentin Faucher on 2023-12-16.
//

#include "util_apple.h"

CGRect  rectangle_toCGRect(Rectangle rect) {
    return (CGRect) { rect.o_x, rect.o_y, rect.w, rect.h };
}
CGSize  vector2_toCGSize(Vector2 v) {
    return (CGSize) { v.x, v.y };
}
MTLClearColor vector4_toMTLClearColor(Vector4 v) {
    return (MTLClearColor) { v.x, v.y, v.z, v.w };
}
Rectangle  CGRect_toRectangle(CGRect rect) {
    return (Rectangle) { rect.origin.x, rect.origin.y, rect.size.width, rect.size.height };
}
Vector2  CGSize_toVector2(CGSize v) {
    return (Vector2) { v.width, v.height };
}
#if TARGET_OS_OSX != 1
Margins UIEdgeInsets_toMargins(UIEdgeInsets m) {
    return (Margins) { m.top, m.left, m.bottom, m.right };
}
#endif
