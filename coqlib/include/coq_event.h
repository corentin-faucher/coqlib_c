//
//  coqevent.h
//  Gestion des events (souris, clavier, etc.)
//
//  Created by Corentin Faucher on 2023-11-17.
//
#ifndef COQ_EVENT_H
#define COQ_EVENT_H

#include "maths/math_base.h"
#include "utils/utils_char_and_keycode.h"

typedef struct _Root Root;

typedef enum _EventFlag {
    event_flag_toDo_ =             0x0001,
    event_flag_done_ =             0x0002,
    // Event de la window vers la root.
    event_type_touch_hovering =    0x0010,
    event_type_touch_down =        0x0020,
    event_type_touch_drag =        0x0040,
    event_type_touch_up =          0x0080,
    event_type_scroll =            0x0100,
    event_type_scrollTrackBegin =  0x0200,
    event_type_scrollTrack =       0x0400,
    event_type_scrollTrackEnd =    0x0800,
    event_type_key_down =          0x1000,
    event_type_key_up =            0x2000,
    event_type_key_mod =           0x4000,
    event_type_resize =          0x008000,
    event_type_system_change =   0x010000,
    
    // Event de la root vers la window.
    event_type_win_full_screen = 0x100000,
    event_type_win_windowed =    0x200000,
    
    event_types_root_ =          0x0FFFF0,
    event_types_win_ =           0xF00000,
} EventFlag;

/// Structure pour un keyboard event.
typedef struct {
    uint32_t  modifiers;
    uint16_t  keycode;
    uint16_t  mkc;
    bool      isVirtual;
    Character typed;
} KeyboardInput;

/// Structure pour un event de resize de la window.
typedef struct {
    /// Marge "inactive" en pts.
    Margins   margins;
    /// Taille en pts, i.e. typiquement 2x les pixels (les coordonnées des `touch` events sont en pts).
    Rectangle framePt;
    /// Taille en pixels.
    Vector2   sizesPix;
    bool      inTransition;
    bool      fullScreen;
    bool      justMoving;
} ResizeInfo;

typedef struct {
    bool      layoutDidChanged;
    bool      languageDidChanged;
    bool      themeDidChanged;
} SystemChange;

/// Union des infos pour les différents types d'events.
typedef struct {
    uint32_t flags;
    union {
        // mouse/touch : hovering, down, drag.
        Vector2       touch_pos;
        // scroll/swipe
        Vector2       scroll_deltas;
        // keyboard events
        KeyboardInput key;
        // View resize
        ResizeInfo    resize_info;
        // System change (layout, language, theme)
        SystemChange  system_change;
    };
} CoqEvent;

extern bool CoqEvent_ignoreRepeatKeyDown; // true

/// Evenement de window -> root, e.g. clic de la souris.
void   CoqEvent_addEvent(CoqEvent event);
/// A mettre dans l'update de la boucle principale.
void   CoqEvent_processEvents(Root* root);

/// Evenement de root -> window, e.g. l'app veut passer en mode plein ecran.
void      CoqEvent_addWindowEvent(CoqEvent event);
CoqEvent* CoqEvent_getWindowEventOpt(void);

#endif /* coqevent_h */
