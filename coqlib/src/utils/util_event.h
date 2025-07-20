//
//  coqevent.h
//  Gestion des events (souris, clavier, etc.)
//
//  Created by Corentin Faucher on 2023-11-17.
//
#ifndef COQ_EVENT_H
#define COQ_EVENT_H


#include "util_char_and_keycode.h"
#include "util_string.h"
#include "../maths/math_base.h"

typedef struct coq_Root Root;
typedef struct coq_Node Node;
typedef struct SlidingMenu SlidingMenu;

enum EventFlag {
    eventtype_null =                           0x000,
    // Event de la window vers la root.
    eventtype_touch_hovering =                 0x001, // Touch / mouse...
    eventtype_touch_down =                     0x002,
    eventtype_touch_drag =                     0x004,
    eventtype_touch_up =                       0x008,
    eventtype_scroll =                         0x010,
    eventtype_key_down =                       0x020,
    eventtype_key_up =                         0x040,
    eventtype_key_mod =                        0x080,
    eventtype_resize =                       0x00100,
    eventtype_systemChanged =                0x00200,
    eventtype_gamePad_down =                 0x00400,
    eventtype_gamePad_value =                0x00800, // Changement analogique de valeur... un peu comme touch_hovering/drag...
    eventtype_gamePad_up =                   0x01000,
    
    // Event de la root vers la window.
    eventtype_win_mac_resize =             0x00010000,
    eventtype_win_ios_keyboardNeeded =     0x00020000,
    eventtype_win_ios_keyboardNotNeeded =  0x00040000,
    eventtype_win_ios_scrollviewNeeded =   0x00080000,
    eventtype_win_ios_scrollviewNotNeeded =0x00100000,
    /// (`private`, désactivation temporaire de la scrollview lorsqu'un noeud est grabbé)
    eventtype_win_ios_scrollviewDisable_ = 0x00200000,
    eventtype_win_ios_fonts_install =      0x00400000,
    eventtype_win_ios_fonts_uninstall =    0x00800000,
    eventtype_win_cloudDrive_start =       0x01000000,
    eventtype_win_cloudDrive_stop =        0x02000000,
    eventtype_win_sendEmail =              0x04000000,
    eventtype_win_terminate =              0x08000000,
    
    // (privates flags)
    event_types_root_ =                    0x0000FFFF,
    event_types_win_ =                     0xFFFF0000,
};

enum {
    scrolltype_scroll,
    scrolltype_trackBegin,
    scrolltype_track,
    scrolltype_trackEnd,
    scrolltype_offSet,
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

/// Mouse/touch event de l'OS : hovering, down, up, drag.
typedef struct {
    Vector2       pos;       // Position en coord. (x,y) dans la view (pas en pixels).
    uint32_t      touchId;   // 0: click gauche / touche ordinaire, 1: click droit.
    bool          yInverted; // Axe des y inversé, i.e. vers le bas au lieu de vers le haut. e.g. iOS, glfw.
} TouchInfo;
// Touch event convertie de l'espace de la OS view vers l'espace de la structure Root.
// Genre de [0, 800] x [0, 500] -> [-1.6, 1.6] x [-1, 1]...
typedef struct {
    Node*const     n;      // Noeud touché (self)
    Vector2 const  posAbs; // Position absolue, i.e. au niveau de root.
    uint32_t const touchId; // Id de la touch, e.g. 0 clique gauche.
    Vector2 const  _posRelOpt; // Position dans le referential de n. Définie si posRelDefined.
    bool const     _posRelDefined;
} NodeTouch;
void    nodetouch_init(NodeTouch* nt, NodeTouch ref);
Vector2 nodetouch_evalPosRel(NodeTouch nt);

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
    /// Position de la fenêtre dans l'écran (par rapport au coin supérieur gauche).
    Vector2   originPt;
    /// Taille en pts, i.e. typiquement 2x les pixels (les coordonnées des `touch` events sont en pts).
    Vector2   framePt;
    /// Taille en pixels.
    Vector2   framePx;
    bool      fullScreen;
    bool      justMoving;
    bool      dontFix;
} ViewSizeInfo;

/// Notifications de changement divers dans l'OS.
typedef struct {
    bool      layoutDidChange;
    bool      languageRegionDidChange;
    bool      themeDidChange;
    bool      userNameDidChange;
    bool      cloudDriveDidChange;
    bool      ios_keyboardUp;
    bool      ios_keyboardDown;
} SystemChange;

/// Requête à l'OS d'envoie de courriel.
typedef struct {
    const char* recipient;
    const char* subject;
    const char* body;
} EmailInfo;
/// Requête pour vérifier un fichier dans le cloud drive.
typedef struct {
    const char* subFolderOpt;
    const char* extensionOpt;
} CloudDriveInfo;

/// Union des infos pour les différents types d'events de OS -> app.
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
        ViewSizeInfo    resize_info;
        // System change (layout, language, theme)
        SystemChange  system_change;
        // Gamepad
        GamePadInput  gamepadInput;
    };
    bool _todo;
} CoqEvent;

/// Union des infos pour les différents types d'events de app -> OS.
typedef struct CoqEventWin {
    uint32_t type;
    union {
        // Mac, demande de resize de la window, e.g. passer en fullscreen.
        ViewSizeInfo            resize_info;
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



/// Evenement de OS -> app, e.g. clic de la souris.
void   CoqEvent_addToRootEvent(CoqEvent event);
/// A mettre dans l'update de la boucle principale. (avec `Timer_check()` et autres dans la thread `event`.)
// MARK: La root principale de l'app à laquelle sont appliquée les events. 
extern Root* CoqEvent_root;
void   CoqEvent_processAllQueuedRootEvents(void);

/// Evenement de app -> OS, e.g. l'app veut passer en mode plein ecran.
void        CoqEvent_addToWindowEvent(CoqEventWin event);
/// A vérifier régulièrement, e.g. dans la NSView.
CoqEventWin CoqEvent_getNextTodoWindowEvent(void);

#endif /* coqevent_h */
