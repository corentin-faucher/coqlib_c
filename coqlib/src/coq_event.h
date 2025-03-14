//
//  coqevent.h
//  Gestion des events (souris, clavier, etc.)
//
//  Created by Corentin Faucher on 2023-11-17.
//
#ifndef COQ_EVENT_H
#define COQ_EVENT_H

#include "maths/math_base.h"
#include "utils/util_char_and_keycode.h"
#include "utils/util_string.h"

typedef struct coq_Root Root;
typedef struct SlidingMenu SlidingMenu;

enum EventFlag {
    event_type_null =                           0x000,
    // Event de la window vers la root.
    event_type_touch_hovering =                 0x001, // Touch / mouse...
    event_type_touch_down =                     0x002,
    event_type_touch_drag =                     0x004,
    event_type_touch_up =                       0x008,
    event_type_scroll =                         0x010,
    event_type_key_down =                       0x020,
    event_type_key_up =                         0x040,
    event_type_key_mod =                        0x080,
    event_type_resize =                       0x00100,
    event_type_systemChanged =                0x00200,
    event_type_gamePad_down =                 0x00400,
    event_type_gamePad_value =                0x00800, // Changement analogique de valeur... un peu comme touch_hovering/drag...
    event_type_gamePad_up =                   0x01000,
    
    // Event de la root vers la window.
    event_type_win_mac_resize =             0x00010000,
    event_type_win_ios_keyboardNeeded =     0x00020000,
    event_type_win_ios_keyboardNotNeeded =  0x00040000,
    event_type_win_ios_scrollviewNeeded =   0x00080000,
    event_type_win_ios_scrollviewNotNeeded =0x00100000,
    /// (`private`, désactivation temporaire de la scrollview lorsqu'un noeud est grabbé)
    event_type_win_ios_scrollviewDisable_ = 0x00200000,
    event_type_win_ios_fonts_install =      0x00400000,
    event_type_win_ios_fonts_uninstall =    0x00800000,
    event_type_win_cloudDrive_start =       0x01000000,
    event_type_win_cloudDrive_stop =        0x02000000,
    event_type_win_sendEmail =              0x04000000,
    
    // (privates flags)
    event_types_root_ =                     0x0000FFFF,
    event_types_win_ =                      0xFFFF0000,
};

enum {
    scroll_type_scroll,
    scroll_type_trackBegin,
    scroll_type_track,
    scroll_type_trackEnd,
    scroll_type_offSet,
};
typedef struct {
    uint16_t scrollType;
    union {
        Vector2  scroll_deltas;
        struct {
            float offset_ratio;
            bool  offset_letGo;
        };
    };
} ScrollInfo;

/// Mouse/touch : hovering, down, drag.
// TODO: touch_ superflu...
typedef struct {
        Vector2       touch_pos; // Position en coord. (x,y) dans la view (pas en pixels).
        uint32_t      touch_id;  // 0: click gauche / touche ordinaire, 1: click droit.
        bool          touch_yInverted; // Axe des y inversé.
} TouchInfo;

typedef struct {
    Rectangle  rect;
    float      contentFactor;
    float      offSetRatio;
} RequireScrollViewInfo;

/// Structure pour un keyboard event. (voir `util_char_and_keycode.h`)
//typedef struct {
//    uint32_t  modifiers;
//    uint16_t  keycode;
//    uint16_t  mkc;
//    bool      isVirtual;
//    Character typed;
//} KeyboardInput;

/// Structure pour un gamePad event
typedef struct GamePadInput {
    uint16_t buttonId;
    float    buttonValue;
    Vector2  vector;
} GamePadInput;

/// Structure pour un event de resize de la window.
typedef struct {
    /// Marge "inactive" en pts.
    Margins   margins;
    /// Taille en pts, i.e. typiquement 2x les pixels (les coordonnées des `touch` events sont en pts).
    Rectangle framePt;
    /// Taille en pixels.
    Vector2   sizesPix;
    bool      fullScreen;
    bool      justMoving;
    bool      dontFix;
} ResizeInfo;

typedef struct {
    bool      layoutDidChange;
    bool      languageRegionDidChange;
    bool      themeDidChange;
    bool      userNameDidChange;
    bool      cloudDriveDidChange;
    bool      ios_keyboardUp;
    bool      ios_keyboardDown;
} SystemChange;

typedef struct {
    const char* recipient;
    const char* subject;
    const char* body;
} EmailInfo;
typedef struct {
    const char* subFolderOpt;
    const char* extensionOpt;
} CloudDriveInfo;

/// Union des infos pour les différents types d'events.
typedef struct CoqEvent {
    uint32_t type;
    union {
        // mouse/touch : hovering, down, drag.
        TouchInfo     touch_info;
        // scroll/swipe
        ScrollInfo    scroll_info;
        // keyboard events
        KeyboardInput key;
        // View resize
        ResizeInfo    resize_info;
        // System change (layout, language, theme)
        SystemChange  system_change;
        // Gamepad
        GamePadInput  gamepadInput;
    };
    bool _todo;
} CoqEvent;

typedef struct CoqEventWin {
    uint32_t type;
    union {
        // Mac, demande de resize de la window.
        ResizeInfo            resize_info;
        // iOS, besoin d'une dummy scroll-view
        RequireScrollViewInfo win_scrollViewInfo;
        // iOS, install/uninstall de Fonts
        SharedStringsArray    win_font_list;
        EmailInfo             email_info;
        CloudDriveInfo        cloudDrive_info;
    };
    bool _todo;
} CoqEventWin;

extern bool CoqEvent_ignoreRepeatKeyDown; // true

/// Evenement de window -> root, e.g. clic de la souris.
void   CoqEvent_addToRootEvent(CoqEvent event);
/// A mettre dans l'update de la boucle principale.
void   CoqEvent_processEvents(Root* root);

/// Evenement de root -> window, e.g. l'app veut passer en mode plein ecran.
void        CoqEvent_addToWindowEvent(CoqEventWin event);
CoqEventWin CoqEvent_getNextTodoWindowEvent(void);

#endif /* coqevent_h */
