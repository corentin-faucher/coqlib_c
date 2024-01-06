//
//  coqevent.h
//  Gestion des events (souris, clavier, etc.)
//
//  Created by Corentin Faucher on 2023-11-17.
//
#ifndef _coq_event_h
#define _coq_event_h

#include "_nodes/_node_root.h"

typedef enum _EventFlag {
    _event_flag_toDo =            0x0001,
    _event_flag_done =            0x0002,
    
    event_type_hovering =         0x0010,
    event_type_down =             0x0020,
    event_type_drag =             0x0040,
    event_type_up =               0x0080,
    event_type_scroll =           0x0100,
    event_type_scrollTrackBegin = 0x0200,
    event_type_scrollTrack =      0x0400,
    event_type_scrollTrackEnd =   0x0800,
    event_type_keyDown =          0x1000,
    event_type_keyUp =            0x2000,
    event_type_keyMod =           0x4000,
    event_type_resize =           0x8000,
    
    _event_types =                0xFFF0,
} EventFlag;

typedef struct _CoqEvent {
    uint32_t flags;
    union {
        Vector2 touch_pos;       // mouse/touch : hovering, down, drag.
        Vector2 scroll_deltas;   // scroll/swipe
        KeyboardInput key; // keyboard events
        struct {                 // View resize
            Margins  resize_margins;
            Vector2  resize_sizesPx;      // (en pixels)
            bool     resize_inTransition;
        };
    };
} CoqEvent;

extern bool CoqEvent_ignoreRepeatKeyDown; // true

void   CoqEvent_addEvent(CoqEvent event);
void   CoqEvent_processEvents(Root* root);

#endif /* coqevent_h */
