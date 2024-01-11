//
//  char_and_keycode.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//

#include "_utils/_utils_char_and_keycode.h"

const char spchar_delete[] = "\b";
const char spchar_deleteSymbol[] = "␈";
const char spchar_tab[] = "\t";
const char spchar_tabSymbol[] = "␉";
const char spchar_return_[] = "\r";
const char spchar_returnSymbol[] = "␍";
const char spchar_space[] = " ";
const char spchar_spaceSymbol[] = "␠";
const char spchar_nobreakSpace[] = " ";
const char spchar_ideographicSpace[] = "　";
const char spchar_thinSpace[] = "\u2009";
const char spchar_bottomBracket[] = "⎵";
const char spchar_underscore[] = "_";
const char spchar_openBox[] = "␣";
const char spchar_interpunct[] = "·";
const char spchar_dot[] = "•";
const char spchar_butterfly[] = "🦋";
const char spchar_dodo[] = "🦤";

const uint16_t MKC_keycode_of_mkc[] = {
#if TARGET_OS_OSX == 1
    0x12, 0x13, 0x14, 0x15, 0x17, 0x16, 0x1A, 0x1C, 0x19, 0x1D, 0x1B, 0x18, // 1, 2, 3, ...
    0x0C, 0x0D, 0x0E, 0x0F, 0x11, 0x10, 0x20, 0x22, 0x1F, 0x23, 0x21, 0x1E, // Q, W, E, ...
    0x00, 0x01, 0x02, 0x03, 0x05, 0x04, 0x26, 0x28, 0x25, 0x29, 0x27,       // A, S, D, ...
    0x06, 0x07, 0x08, 0x09, 0x0B, 0x2D, 0x2E, 0x2B, 0x2F, 0x2C,             // Z, X, ...
    0x32, 0x0A, 0x5E, 0x5D, 0x2A, 0x2A, keycode_space,
#else
    0x1E,0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x2D, 0x2E, // 1, 2, 3, ...
    0x14,0x1A, 0x08, 0x15, 0x17, 0x1C, 0x18, 0x0C, 0x12, 0x13, 0x2F, 0x30, // Q, W, E, ...
    0x04,0x16, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x33, 0x34,       // A, S, D, ...
    0x1D,0x1B, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38,             // Z, X, ...
    0x35,0x64, 0x87, 0x89, 0x31, 0x32, keycode_space,
#endif
    // ANSI_Grave, ISO_Section, JIS_Underscore, JIS_Yen, ANSI_Backslash, ISO_Backslash, Space
    keycode_delete, keycode_return_, keycode_tab,
    keycode_empty, keycode_empty, keycode_empty, keycode_empty, keycode_empty, // 55~59
    keycode_capsLock, keycode_control, keycode_shift, keycode_option, keycode_command, // 60~64
    keycode_rightControl, keycode_rightShift, keycode_rightOption, keycode_rightCommand, // 65~68
    keycode_empty, keycode_escape, keycode_JIS_Eisu, keycode_JIS_kana, keycode_empty, // 69~73.
};

const uint16_t MKC_homerow_mkcs[] = {
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
};

const uint16_t MKC_prefered_order_mkcs[] = {
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
    45, 46, 47, 48, 49, 50, 51,
};


