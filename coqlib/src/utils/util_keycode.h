//
//  char_and_keycode.h
//  Constantes du clavier:
//  char importants, keycodes, mods...
//
//  Created by Corentin Faucher on 2023-12-08.
//
#ifndef COQ_UTIL_KEYCODES_H
#define COQ_UTIL_KEYCODES_H

#include <stdint.h>
#include <stdbool.h>

// MARK: -- Keycodes (dépend du système)--------------------------------
// Voir Carbon / HiToolbox / event.h...
//#include <Carbon/Carbon.h> // -> e.g. `kVK_Command`...
#if __APPLE__
// Pour les `TARGET_OS_OSX`, etc.
#include <TargetConditionals.h>
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
    // Input (virtual) de text (pour iOS < 13)
    keycode__text = 0xFD,
    // Dummy "empty" (touche "vide" ne faisant rien)
    keycode_empty = 0xFE,
    keycode_total_keycodes = 0xFF,
};
#else
// TODO: Linux keycodes...
enum {
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
    // Input (virtual) de text (pour iOS < 13)
    keycode__text = 0xFD,
    // Dummy "empty" (touche "vide" ne faisant rien)
    keycode_empty = 0xFE,
    keycode_total_keycodes = 0xFF,
};
#endif

/*-- Modifiers --------------------------------------------------*/
// Voir AppKit / NSEvent.h
//#include <AppKit/AppKit.h> // -> e.g. `NSEventModifierFlagCapsLock`...
enum {
    // Apple (iOS / macOS), todo pour autres systèmes.
    modifier_capslock =  0x010000,
    modifier_shift =     0x020000,
    modifier_control =   0x040000,
    modifier_option =    0x080000,
    modifier_command =   0x100000,

    modifiers_optionShift = modifier_shift|modifier_option,
};

// MARK: - "MyKeycode"
// des "keycodes" de touches du clavier indépendants du système.
enum {
    // 0 ...11 -> ligne de 1 à +
    // 12...23 -> ligne de Q à ]
    // 24...34 -> ligne de A à '
    // 35...44 -> ligne de Z à /
    // Touches usuelles de racourcis clavier...
    mkc_Q = 12,
    mkc_W = 13,
    mkc_E = 14,
    mkc_R = 15,
    mkc_A = 24,
    mkc_S = 25,
    mkc_D = 26,
    mkc_F = 27,
    mkc_J = 30,
    mkc_Z = 35,
    mkc_X = 36,
    mkc_C = 37,
    mkc_V = 38,
    // Internationals
    mkc_ansi_grave = 45,
    mkc_iso_section = 46,
    mkc_jis_underscore = 47,
    mkc_jis_yen = 48,
    mkc_ansi_backslash = 49,
    mkc_iso_backslash = 50,
    // Espace
    mkc_space = 51,  // Dernier keycode "standard".
    // Keycodes spéciaux ayant une string associable
    mkc_backspace = 52,
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
    // mkc_app = 73, ? le "menu" dans windows...

    // Clavier numerique
    // 74~79  -> ".", enter, "+", "-", "*", "/".
    // 80~89  -> 0 à 9
    mkc_numpad_first_dot = 74,
    mkc_numpad_enter = 75,
    mkc_numpad_0 = 80, // 1, 2, 3,...
    mkc_numpad_5 = 85,
    mkc_numpad_9 = 89,

    // Touches de directions
    mkc_arrowLeft =  90,
    mkc_arrowRight = 91,
    mkc_arrowDown =  92,
    mkc_arrowUp =    93,
    // Utile ? Ajouter genre 94 à 97 :
//    mkc_pageUp, mkc_pageDown, mkc_home, mkc_end, mkc_delete,

    // Text input (virtual)
    mkc__text = 98,
    // Pour les "autres" non définie (e.g. fn, quelconque...)
    mkc_empty = 99,
    mkc_total_mkcs = 100,
};
/// Pour les touche avec un affichage "non-localisé", e.g. "Enter", "Escape", "Ctrl",...
/// i.e. pas des lettres fonction du layout.
static inline bool mkc_isNotGlyphed(uint16_t mkc) {
    return ((mkc > mkc_space) && (mkc < mkc_numpad_first_dot)) ||
            (mkc > mkc_numpad_9) || (mkc == mkc_numpad_enter);
}

/// Mapping de mkc -> keycode.
extern const uint16_t MKC_keycode_of_mkc[];
/// Mapping de keycode -> mkc.
extern const uint16_t MKC_of_keycode[];

/// // MARK: - Keyboard input, une entrée clavier -> voir `util_event.h`.
typedef struct {
    uint32_t  modifiers;
    uint16_t  keycode;
    uint16_t  mkc;
    bool      isVirtual;
//    Character typed;
} KeyboardInput;
void KeyboardInput_initMapKeyboardInputOfMkc(void);
/// Mapping de mkc -> KeyboardInput (Virtuals). Fonction car a besoin d'être init au premier call...
KeyboardInput KeyboardInput_ofMkc(uint16_t mkc);

/// Les MyKeyCode sur la homerow, i.e. 24...34.
extern const uint16_t MKC_homerow_mkcs[];
/// Ordre de scan de preference, i.e HomeRow, upper row, lower row, etc.
extern const uint16_t MKC_prefered_order_mkcs[];


void test_print_mkcOfKeycode_(void);

// MARK: - GamePad buttons (style Nintendo)

enum {
    // Down/up input (comme keyboard keys)
    gamepad_A,
    gamepad_B,
    gamepad_X,
    gamepad_Y,
    gamepad_L,
    gamepad_R,
    gamepad_ZL,
    gamepad_ZR,
    gamepad_Plus,
    gamepad_Minus,
    // -> Value input ~analogiques
    gamepad_dpad,
    gamepad_JoystickLeft,
    gamepad_JoystickRight,
};

#endif /* char_and_keycode_h */
