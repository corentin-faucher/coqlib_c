//
//  char_and_keycode.h
//  Constantes du clavier:
//  char importants, keycodes, mods...
//
//  Created by Corentin Faucher on 2023-12-08.
//

#ifndef COQ_UTILS_CHAR_AND_KEYCODES_H
#define COQ_UTILS_CHAR_AND_KEYCODES_H

#include "utils_base.h"

/// Structure (union) pour mini-string, i.e. typiquement un char utf8.
typedef union {
    char     c_str[8]; // A priori un 'Character' est une mini string de 8 bytes
                       // (au moins 5 pour avoir le null terminal dans le cas des emojis.)
    uint64_t c_data;
    char     first;    // Dans la plupart des cas c'est juste un ascii...
} Character;

/** Les char spéciaux et "importans" */
extern const Character spchar_delete; // = "\u{8}"
extern const Character spchar_deleteSymbol; // = "␈"
extern const Character spchar_tab; // = "\t"
extern const Character spchar_tabSymbol; // = "␉"
extern const Character spchar_return_; // = "\r"
extern const Character spchar_returnSymbol; // = "␍"
extern const Character spchar_space; // = " "
extern const Character spchar_spaceSymbol; // = "␠"
extern const Character spchar_spaceNobreak; // = " "
extern const Character spchar_spaceIdeographic; // = "　"
extern const Character spchar_spaceThin; // = "\u{2009}"
extern const Character spchar_bottomBracket; // = "⎵"
extern const Character spchar_underscore; // = "_"
extern const Character spchar_openBox; // = "␣"
extern const Character spchar_interpunct; // = "·"
extern const Character spchar_dot; // = "•"
extern const Character spchar_butterfly; // = "🦋"
extern const Character spchar_dodo; // = "🦤"



/*-- Keycodes ---------------------------------------------------*/
// Voir Carbon / HiToolbox / event.h...
//#include <Carbon/Carbon.h> // -> e.g. `kVK_Command`...
enum {
#if TARGET_OS_OSX == 1  // pour macOS
// Voir hitoolbox events.h...
// Touches modifier
    keycode_command = 0x37,
    keycode_shift = 0x38,
    keycode_capsLock = 0x39,
    keycode_option = 0x3A,
    keycode_control = 0x3B,
    keycode_rightCommand = 0x36,
    keycode_rightShift = 0x3C,
    keycode_rightOption = 0x3D,
    keycode_rightControl = 0x3E,
// Touche "importantes"
    keycode_return_ = 0x24,
    keycode_keypadEnter = 0x4C,
    keycode_tab = 0x30,
    keycode_space = 0x31,
    keycode_delete = 0x33,
    keycode_forwardDelete = 0x75,
    keycode_escape = 0x35,
// Touches de directions
    keycode_arrowLeft = 0x7B,
    keycode_arrowRight = 0x7C,
    keycode_arrowDown = 0x7D,
    keycode_arrowUp = 0x7E,
// Touches spéciales ANSI, ISO, JIS
    keycode_ANSI_Backslash = 0x2A,
    keycode_ANSI_Grave = 0x32,
    keycode_ISO_section = 0x0A,
    keycode_JIS_Yen = 0x5D,
    keycode_JIS_Underscore = 0x5E,
    keycode_JIS_kana = 0x68,
    keycode_JIS_Eisu = 0x66, // 英数
#else // Pour iOS.
// Touches modifier
    keycode_capsLock = 0x39,
    keycode_control = 0xE0,
    keycode_shift = 0xE1,
    keycode_option = 0xE2,
    keycode_command = 0xE3,
    keycode_rightControl = 0xE4,
    keycode_rightShift = 0xE5,
    keycode_rightOption = 0xE6,
    keycode_rightCommand = 0xE7,
// Touche "importantes"
    keycode_return_ = 0x28,
    keycode_keypadEnter = 0x58,
    keycode_tab = 0x2B,
    keycode_space = 0x2C,
    keycode_delete = 0x2A,
    keycode_forwardDelete = 0x4C,
    keycode_escape = 0x29,
// Touches de directions
    keycode_arrowLeft = 0x50,
    keycode_arrowRight = 0x4F,
    keycode_arrowDown = 0x51,
    keycode_arrowUp = 0x52,
// Touches spéciales ANSI, ISO, JIS
    keycode_ANSI_Backslash = 0x31,
    keycode_ANSI_Grave = 0x35,
    keycode_ISO_Backslash = 0x32,
    keycode_ISO_section = 0x64,
    keycode_JIS_Yen = 0x89,
    keycode_JIS_Underscore = 0x87,
    keycode_JIS_kana = 0x90,
    keycode_JIS_Eisu = 0x91, // 英数
#endif
// Dummy "empty" (touche "vide" ne faisant rien)
    keycode_empty = 0xFE,
    keycode_total_keycodes = 0xFF,
};

/*-- Modifiers --------------------------------------------------*/
// Voir AppKit / NSEvent.h
//#include <AppKit/AppKit.h> // -> e.g. `NSEventModifierFlagCapsLock`...
enum {
#if TARGET_OS_OSX == 1
    modifier_command =  1 << 20,
    modifier_shift =    1 << 17,
    modifier_capslock = 1 << 16,
    modifier_option =   1 << 19,
    modifier_control =  1 << 18,
#else
    modifier_command =   0x100000,
    modifier_shift =     0x020000,
    modifier_capslock =  0x010000,
    modifier_option =    0x080000,
    modifier_control =   0x040000,
#endif
    modifier_optionShift = modifier_shift|modifier_option,
};

/*-- "MyKeyCode" : Des "keycodes" indépendant du système. ----------*/
enum {
    // 0...11 -> ligne de 1.
    // 12...23 -> ligne de Q.
    // 24...34 -> ligne de A.
    // 35...44 -> ligne de Z.
    // 45...51 -> ANSI_Grave, ISO_Section, JIS_Underscore, JIS_Yen, ANSI_Backslash, ISO_Backslash, Space.
    mkc_space = 51,  // Dernier keycode "standard".
    
    // Keycodes spéciaux ayant une string associable
    mkc_delete = 52,
    mkc_return_ = 53,
    mkc_tab = 54,
    
    // Keycodes de contrôle
    mkc_modifier_first = 60,
    mkc_capsLock = 60,
    mkc_control = 61,
    mkc_shift = 62,
    mkc_option = 63,
    mkc_command = 64,
    mkc_rightControl = 65,
    mkc_rightShift = 66,
    mkc_rightOption = 67,
    mkc_rightCommand = 68,
    mkc_modifier_last = 68,
    // Autre Keycodes Spéciaux
    mkc_escape = 70,
    mkc_jis_eisu = 71,
    mkc_jis_kana = 72,
    
    // Clavier numerique
    // 74~79  -> ".", enter, "+", "-", "*", "/".
    // 80~89  -> 0 à 9
    mkc_numpad_first_dot = 74,
    mkc_numpad_enter = 75,
    mkc_numpad_0 = 80,
    mkc_numpad_9 = 89,
    
    // Pour les "autres" non définie (e.g. fn, quelconque...)
    mkc_empty = 99,
    mkc_total_mkcs = 100,
};

/// Mapping de mkc -> keycode.
extern const uint16_t MKC_keycode_of_mkc[];
/// Mapping de keycode -> mkc.
extern const uint16_t MKC_of_keycode[];

/// Les MyKeyCode sur la homerow, i.e. 24...34.
extern const uint16_t MKC_homerow_mkcs[];
/// Ordre de scan de preference, i.e HomeRow, upper row, lower row, etc.
extern const uint16_t MKC_prefered_order_mkcs[];


void test_print_mkcOfKeycode_(void);

#endif /* char_and_keycode_h */
