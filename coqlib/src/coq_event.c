//
//  coqevent.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-17.
//

#include "coq_event.h"
#include "nodes/node_tree.h"
#include "nodes/node_root.h"
#include "nodes/node_sliding_menu.h"
#include "utils/util_base.h"

#define Max_Event_count_ 64

static CoqEvent           event_poll_[Max_Event_count_];
static CoqEvent* const    event_poll_end_ = &event_poll_[Max_Event_count_];
static CoqEvent*          event_current_ =  &event_poll_[0];
static CoqEvent*          event_todo_ =     &event_poll_[0];

bool CoqEvent_ignoreRepeatKeyDown = true;

void   CoqEvent_addToRootEvent(CoqEvent event) {
    if(!(event.type & event_types_root_)) {
        printerror("Not a `to root` event.");
        return;
    }
    if(event_current_->_todo) {
        printwarning("Event poll full.");
        return;
    }
    // Set to current event.
    *event_current_ = event;
    event_current_->_todo = true;
    // Placer current sur le next en liste.
    event_current_ ++;
    if(event_current_ >= event_poll_end_)
        event_current_ = event_poll_;
}

void  CoqEvent_processEvents(Root* root) {
    while (event_todo_->_todo) {
        switch (event_todo_->type & event_types_root_) {
            case event_type_touch_hovering: {
                Button* const hovered = root_searchActiveButtonOptWithPos(root, event_todo_->touch_pos, NULL);
                Button* const lastHovered = root->buttonSelectedOpt;
                if(lastHovered == hovered) break;
                if(lastHovered) if(lastHovered->stopHoveringOpt)
                    lastHovered->stopHoveringOpt(lastHovered);
                root->buttonSelectedOpt = hovered;
                if(hovered) if(hovered->startHoveringOpt)
                    hovered->startHoveringOpt(hovered);
                break;
            }
            case event_type_touch_down: {
                root->lastTouchedPos = event_todo_->touch_pos;
                Button* const lastSelected = root->buttonSelectedOpt;
                if(lastSelected) if(lastSelected->stopHoveringOpt)
                    lastSelected->stopHoveringOpt(lastSelected);
                root->buttonSelectedOpt = NULL;
                // 0. Trouver un bouton sélectionable
                Button* const toTouch = root_searchActiveButtonOptWithPos(root, event_todo_->touch_pos, NULL);
                if(toTouch == NULL) break;
                // 1. Grab le noeud draggable (si on drag on n'active pas)
                if(toTouch->grabOpt) {
                    // Désactiver un scrollview quand un noeud est grabbé
                    #if TARGET_OS_OSX != 1
                    CoqEvent_addToWindowEvent((CoqEventWin) {
                        .type = event_type_win_ios_scrollviewDisable_
                    });
                    #endif
                    root->buttonSelectedOpt = toTouch;
                    Vector2 relpos = vector2_absPosToPosInReferentialOfNode(
                                                                            event_todo_->touch_pos, &toTouch->n);
                    toTouch->grabOpt(toTouch, relpos);
                    break;
                }
                // 2. Sinon activer le noeud sélectionné (non grabbable)
                if(toTouch->action)
                    toTouch->action(toTouch);
                break;
            }
            case event_type_touch_drag: {
                Button* const grabbed = root->buttonSelectedOpt;
                if(grabbed == NULL) break;
                if(grabbed->dragOpt == NULL) {
                    printwarning("Grabbed node without drag function.");
                    break;
                }
                Vector2 relpos = vector2_absPosToPosInReferentialOfNode(
                                                                        event_todo_->touch_pos, &grabbed->n);
                grabbed->dragOpt(root->buttonSelectedOpt, relpos);
                break;
            }
            case event_type_touch_up: {
                Button* const grabbed = root->buttonSelectedOpt;
                if(grabbed == NULL) break;
                if(grabbed->letGoOpt) {
                    grabbed->letGoOpt(root->buttonSelectedOpt);
                } else {
                    printwarning("Grabbed node without letGo function.");
                }
                root->buttonSelectedOpt = NULL;
                break;
            }
            case event_type_scroll: {
                SlidingMenu* sm = root_searchFirstSlidingMenuOpt(root);
                if(sm == NULL) break;
                ScrollInfo info = event_todo_->scroll_info;
                switch(event_todo_->scroll_info.scrollType) {
                    case scroll_type_scroll:
                        slidingmenu_scroll(sm, info.scroll_deltas.y > 0); break;
                    case scroll_type_trackBegin:
                        slidingmenu_trackPadScrollBegan(sm); break;
                    case scroll_type_track:
                        slidingmenu_trackPadScroll(sm, info.scroll_deltas.y); break;
                    case scroll_type_trackEnd:
                        slidingmenu_trackPadScrollEnded(sm); break;
                    case scroll_type_offSet:
                        slidingmenu_setOffsetRatio(sm, info.offset_ratio, info.offset_letGo); break;
                    default: printerror("Bad scroll type %d.", event_todo_->scroll_info.scrollType);
                }
                break;
            }
            case event_type_key_down: {
                View* viewActive = root->viewActiveOpt;
                if(!viewActive) break;
                KeyboardInput key = event_todo_->key;
                
                // Touche spéciales de `navigation` (enter, escape, flèches)
                if((key.keycode == keycode_escape) && viewActive->escapeOpt) {
                    viewActive->escapeOpt(viewActive);
                    break;
                }
                if(((key.keycode == keycode_keypadEnter) ||
                    (key.keycode == keycode_return_)
                   ) && viewActive->enterOpt) {
                    viewActive->enterOpt(viewActive);
                    break;
                }
                if((key.keycode == keycode_arrowUp) || (key.keycode == keycode_arrowDown)) {
                    SlidingMenu* sm = root_searchFirstSlidingMenuOpt(root);
                    if(sm) {
                        slidingmenu_scroll(sm, key.keycode == keycode_arrowUp);
                        break;
                    }
                }
                
                // Touche quelconque
                if(!viewActive->keyDownOpt) break;
                viewActive->keyDownOpt(viewActive, key);
                break;
            }
            case event_type_key_up: {
                View* viewActive = root->viewActiveOpt;
                if(!viewActive) break;
                if(!viewActive->keyUpOpt) break;
                viewActive->keyUpOpt(viewActive, event_todo_->key);
                break;
            }
            case event_type_key_mod: {
                View* viewActive = root->viewActiveOpt;
                if(!viewActive) break;
                if(!viewActive->modifiersChangedToOpt) break;
                viewActive->modifiersChangedToOpt(viewActive, event_todo_->key.modifiers);
                break;
            }
            case event_type_resize: {
                root_viewResized(root, event_todo_->resize_info);
                break;
            }
            case event_type_systemChanged: {
                View* viewActive = root->viewActiveOpt;
                if(!viewActive) break;
                if(!viewActive->systemChangedOpt) break;
                viewActive->systemChangedOpt(viewActive, event_todo_->system_change);
//                if(root->systemDidChangedActionOpt) root->systemDidChangedActionOpt(root, event_todo_->system_change);
                break;
            }
            default: printwarning("Event %x not defined.", event_todo_->type & event_types_root_); break;
        }
        
        event_todo_->_todo = false;
        event_todo_ ++;
        if(event_todo_ >= event_poll_end_)
            event_todo_ = event_poll_;
    }
}

static CoqEventWin           event_win_arr_[10];
static CoqEventWin* const    event_win_arr_end_ = &event_win_arr_[10];
static CoqEventWin*          event_win_current_ = &event_win_arr_[0];
static CoqEventWin*          event_win_todo_ =    &event_win_arr_[0];

void  CoqEvent_addToWindowEvent(CoqEventWin event) {
    if(!(event.type & event_types_win_)) {
        printerror("Not a to window event.");
        return;
    }
    if(event_win_current_->_todo) {
        printwarning("Event win poll full.");
        return;
    }
    *event_win_current_ = event;
    event_win_current_->_todo = true;
    
    event_win_current_ ++;
    if(event_win_current_ >= event_win_arr_end_)
        event_win_current_ = event_win_arr_;
}

CoqEventWin CoqEvent_getNextTodoWindowEvent(void) {
    static const CoqEventWin event_win_null = { 0 };
    if(!(event_win_todo_->_todo))
        return event_win_null;
    CoqEventWin to_return = *event_win_todo_;
    // Libérer et aller sur next.
    event_win_todo_->_todo = false; 
    event_win_todo_++;
    if(event_win_todo_ >= event_win_arr_end_)
        event_win_todo_ = event_win_arr_;
    return to_return;
}
