//
//  coqevent.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-17.
//

#include "coq_event.h"
#include "nodes/node_tree.h"
#include "nodes/node_root.h"

#define Max_Event_count_ 64

static CoqEvent           event_poll_[Max_Event_count_];
static CoqEvent* const    event_poll_end_ = &event_poll_[Max_Event_count_];
static CoqEvent*          event_current_ =  &event_poll_[0];
static CoqEvent*          event_todo_ =     &event_poll_[0];

bool CoqEvent_ignoreRepeatKeyDown = true;

void   CoqEvent_addEvent(CoqEvent event) {
    if(!(event.flags & event_types_root_)) {
        printerror("Not a to root event.");
        return;
    }
    if(event_current_->flags & event_flag_done_) {
        event_current_->flags = 0;
    }
    if(event_current_->flags & event_flag_toDo_) {
        printwarning("Event poll full.");
        return;
    }
    event.flags &= ~ (event_flag_toDo_|event_flag_done_);
    *event_current_ = event;
    
    event_current_->flags |= event_flag_toDo_;
    event_current_ ++;
    if(event_current_ >= event_poll_end_)
        event_current_ = event_poll_;
}

void  CoqEvent_processEvents(Root* root) {
    while ((event_todo_->flags & event_flag_toDo_) && !(event_todo_->flags & event_flag_done_)) {
        switch (event_todo_->flags & event_types_root_) {
            case event_type_touch_hovering: {
                Button* const hovered = root_searchButtonOpt(root, event_todo_->touch_pos, NULL);
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
                Button* const toTouch = root_searchButtonOpt(root, event_todo_->touch_pos, NULL);
                if(toTouch == NULL) break;
                // 1. Grab le noeud draggable (si on drag on ne select pas)
                if(toTouch->grabOpt) {
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
                slidingmenu_scroll(sm, event_todo_->scroll_deltas.y > 0);
                break;
            }
            case event_type_scrollTrackBegin: {
                SlidingMenu* sm = root_searchFirstSlidingMenuOpt(root);
                if(sm == NULL) break;
                slidingmenu_trackPadScrollBegan(sm);
                break;
            }
            case event_type_scrollTrack: {
                SlidingMenu* sm = root_searchFirstSlidingMenuOpt(root);
                if(sm == NULL) break;
                slidingmenu_trackPadScroll(sm, event_todo_->scroll_deltas.y);
                break;
            }
            case event_type_scrollTrackEnd: {
                SlidingMenu* sm = root_searchFirstSlidingMenuOpt(root);
                if(sm == NULL) break;
                slidingmenu_trackPadScrollEnded(sm);
                break;
            }
            case event_type_resize: {
                root_viewResized(root, event_todo_->resize_info);
                break;
            }
            case event_type_system_change: {
                if(root->systemDidChangedActionOpt) root->systemDidChangedActionOpt(root, event_todo_->system_change);
                break;
            }
            case event_type_key_down: {
                View* viewActive = root->viewActiveOpt;
                if(!viewActive) break;
                if((event_todo_->key.keycode == keycode_escape) && viewActive->escapeOpt) {
                    viewActive->escapeOpt(viewActive);
                    break;
                }
                if(((event_todo_->key.keycode == keycode_keypadEnter) ||
                    (event_todo_->key.keycode == keycode_return_)
                   ) && viewActive->enterOpt) {
                    viewActive->enterOpt(viewActive);
                    break;
                }
                if(!viewActive->keyDownOpt) break;
                viewActive->keyDownOpt(viewActive, event_todo_->key);
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
                
            default: printwarning("Event %d not defined.", event_todo_->flags & event_types_root_); break;
        }
        
        event_todo_->flags |= event_flag_done_;
        event_todo_ ++;
        if(event_todo_ >= event_poll_end_)
            event_todo_ = event_poll_;
    }
}

static CoqEvent           event_win_arr_[10];
static CoqEvent* const    event_win_arr_end_ = &event_win_arr_[10];
static CoqEvent*          event_win_current_ = &event_win_arr_[0];
static CoqEvent*          event_win_todo_ =    &event_win_arr_[0];

void  CoqEvent_addWindowEvent(CoqEvent event) {
    if(!(event.flags & event_types_win_)) {
        printerror("Not a to window event.");
        return;
    }
    if(event_win_current_->flags & event_flag_done_) {
        event_current_->flags = 0;
    }
    if(event_win_current_->flags & event_flag_toDo_) {
        printwarning("Event win poll full.");
        return;
    }
    event.flags &= ~ (event_flag_toDo_|event_flag_done_);
    *event_win_current_ = event;
    
    event_win_current_->flags |= event_flag_toDo_;
    event_win_current_ ++;
    if(event_win_current_ >= event_win_arr_end_)
        event_win_current_ = event_win_arr_;
}

CoqEvent* CoqEvent_getWindowEventOpt(void) {
    if(!(event_win_todo_->flags & event_flag_toDo_))
        return NULL;
    if(event_win_todo_->flags & event_flag_done_)
        return NULL;
    CoqEvent* todo = event_win_todo_;
    event_win_todo_->flags |= event_flag_done_;
    event_win_todo_++;
    if(event_win_todo_ >= event_win_arr_end_)
        event_win_todo_ = event_win_arr_;
    return todo;
}
