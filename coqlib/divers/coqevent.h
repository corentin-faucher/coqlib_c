//
//  coqevent.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-17.
//

#ifndef coqevent_h
#define coqevent_h

#include "root.h"

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
        Vector2 pos;     // mouse/touch : hovering, down, drag.
        Vector2 deltas;  // scroll...
        struct {         // keyboard events
            char     typed[8];
            uint32_t mod;
        };
        struct {         // View resize
            Margins  margins;
            Vector2  sizesPx;      // (en pixels)
            Bool     inTransition;
        };
    };
} CoqEvent;

void   CoqEvent_addEvent(CoqEvent event);
void   CoqEvent_processEvents(Root* root);

#endif /* coqevent_h */
