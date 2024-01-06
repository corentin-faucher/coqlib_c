//
//  apple_utils.m
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-16.
//

#include "_utils_apple.h"
#include "_utils_.h"
#import <Carbon/Carbon.h>

static char* _MacOS_tmp_layoutName = NULL;
const char* MacOS_currentLayoutOpt(void) {
    return _MacOS_tmp_layoutName;
}
void        _MacOS_updateCurrentLayout(void) {
#if TARGET_OS_OSX == 1
    if(_MacOS_tmp_layoutName) coq_free(_MacOS_tmp_layoutName);
    _MacOS_tmp_layoutName = NULL;
    TISInputSourceRef inputSrc = TISCopyCurrentKeyboardInputSource();
    if(inputSrc == nil) {
        printwarning("Keyboard input source not found.");
        return;
    }
    CFStringRef property = (CFStringRef)TISGetInputSourceProperty(inputSrc, kTISPropertyInputSourceID);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(
                CFStringGetLength(property), kCFStringEncodingUTF8) + 1;
    _MacOS_tmp_layoutName = coq_calloc(maxSize, sizeof(char));
    CFStringGetCString(property, _MacOS_tmp_layoutName, maxSize, kCFStringEncodingUTF8);
#else
    printwarning("Only for macOS.");
#endif
}

