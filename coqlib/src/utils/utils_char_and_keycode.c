//
//  char_and_keycode.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//

#include "utils/utils_char_and_keycode.h"

const Character spchar_delete = { "\b" };
const Character spchar_deleteSymbol = { "␈" };
const Character spchar_tab = { "\t" };
const Character spchar_tabSymbol = { "␉" };
const Character spchar_return_ = { "\r" };
const Character spchar_returnSymbol = { "␍" };
const Character spchar_space = { " " };
const Character spchar_spaceSymbol = { "␠" };
const Character spchar_spaceNobreak = { " " };
const Character spchar_spaceIdeographic = { "　" };
const Character spchar_spaceThin = { "\u2009" };
const Character spchar_bottomBracket = { "⎵" };
const Character spchar_underscore = { "_" };
const Character spchar_openBox = { "␣" };
const Character spchar_interpunct = { "·" };
const Character spchar_dot = { "•" };
const Character spchar_butterfly = { "🦋" };
const Character spchar_dodo = { "🦤" };

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
    keycode_empty, keycode_escape, keycode_JIS_Eisu, keycode_JIS_kana, keycode_empty,    // 69~73
    // Clavier numerique
#if TARGET_OS_OSX == 1
    // 74~79  -> ".", enter, "+", "-", "*", "/".
    65, 76, 69, 78, 67, 75,
    // 80~89  -> 0 à 9
    82, 83, 84, 85, 86, 87, 88, 89, 91, 92,
#else
#warning TODO
    // 74~79  -> ".", enter, "+", "-", "*", "/".
    65, 76, 69, 78, 67, 75,
    // 80~89  -> 0 à 9
    82, 83, 84, 85, 86, 87, 88, 89, 91, 92,
#endif
    keycode_empty, keycode_empty, keycode_empty, keycode_empty, keycode_empty,  // 90~99
    keycode_empty, keycode_empty, keycode_empty, keycode_empty, keycode_empty,
};

const uint16_t MKC_of_keycode[] = {
#if TARGET_OS_OSX == 1
    24, 25, 26, 27, 29, 28, 35, 36, 37, 38, 46, 39, 12, 13, 14, 15, 17, 16, 0, 1, 2, 3, 5, 4, 11,
    8, 6, 10, 7, 9, 23, 20, 18, 22, 19, 21, 53, 32, 30, 34, 31, 33, 50, 42, 44, 40, 41, 43,
    54, 51, 45, 52, 99, 70, 68, 64, 62, 60, 63, 61, 66, 67, 65, 99, 99, 74, 99, 78, 99, 76,
    99, 99, 99, 99, 99, 79, 75, 99, 77, 99, 99, 99, 80, 81, 82, 83, 84, 85, 86, 87, 99,
    88, 89, 48, 47, 99, 99, 99, 99, 99, 99, 99, 71, 99, 72, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
#else
#warning TODO
    24, 25, 26, 27, 29, 28, 35, 36, 37, 38, 46, 39, 12, 13, 14, 15, 17, 16, 0, 1, 2, 3, 5, 4, 11,
    8, 6, 10, 7, 9, 23, 20, 18, 22, 19, 21, 53, 32, 30, 34, 31, 33, 50, 42, 44, 40, 41, 43,
    54, 51, 45, 52, 99, 70, 68, 64, 62, 60, 63, 61, 66, 67, 65, 99, 99, 74, 99, 78, 99, 76,
    99, 99, 99, 99, 99, 79, 75, 99, 77, 99, 99, 99, 80, 81, 82, 83, 84, 85, 86, 87, 99,
    88, 89, 48, 47, 99, 99, 99, 99, 99, 99, 99, 71, 99, 72, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
#endif
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


void test_print_mkcOfKeycode_(void) {
    uint16_t mkcOfKeycode[256] = {[0 ... 255] = mkc_empty};
    for(uint16_t mkc = 0; mkc < mkc_total_mkcs; mkc ++) {
        uint16_t keycode = MKC_keycode_of_mkc[mkc];
        mkcOfKeycode[keycode] = mkc;
    }
    printdebug("mkc of keycodes:");
    for(uint16_t keycode = 0; keycode < 256; keycode++) {
        printf("%d, ", mkcOfKeycode[keycode]);
    }
    printf("\n🐔\n");
}

