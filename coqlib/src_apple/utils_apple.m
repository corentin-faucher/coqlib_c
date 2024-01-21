//
//  apple_utils.m
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-16.
//

#import <Carbon/Carbon.h>
#import <AppKit/AppKit.h>
#include "coq_utils.h"

static char* macOS_tmp_layoutName_ = NULL;
const char* CoqSystem_currentLayoutOpt(void) {
    return macOS_tmp_layoutName_;
}
void        CoqSystem_updateCurrentLayout(void) {
#if TARGET_OS_OSX == 1
    if(macOS_tmp_layoutName_) coq_free(macOS_tmp_layoutName_);
    macOS_tmp_layoutName_ = NULL;
    TISInputSourceRef inputSrc = TISCopyCurrentKeyboardInputSource();
    if(inputSrc == nil) {
        printwarning("Keyboard input source not found.");
        return;
    }
    CFStringRef property = (CFStringRef)TISGetInputSourceProperty(inputSrc, kTISPropertyInputSourceID);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(
                CFStringGetLength(property), kCFStringEncodingUTF8) + 1;
    macOS_tmp_layoutName_ = coq_calloc(maxSize, sizeof(char));
    CFStringGetCString(property, macOS_tmp_layoutName_, maxSize, kCFStringEncodingUTF8);
#else
    printwarning("Only for macOS.");
#endif
}

static BOOL current_theme_is_dark_ = false;
void        CoqSystem_updateCurrentTheme(void) {
    NSAppearance* appearance;
    if (@available(macOS 11.0, *)) {
        appearance = NSAppearance.currentDrawingAppearance;
    } else {
        appearance = NSAppearance.currentAppearance;
    }
    current_theme_is_dark_ = [@[
        NSAppearanceNameDarkAqua, NSAppearanceNameVibrantDark,
        NSAppearanceNameAccessibilityHighContrastDarkAqua,
        NSAppearanceNameAccessibilityHighContrastVibrantDark
    ] containsObject:[appearance name]];
}
bool        CoqSystem_currentThemeIsDark(void) {
    return current_theme_is_dark_;
}

CGRect  rectangle_toCGRect(Rectangle rect) {
    return (CGRect) { rect.o_x, rect.o_y, rect.w, rect.h };
}
CGSize  vector2_toCGSize(Vector2 v) {
    return (CGSize) { v.x, v.y };
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
