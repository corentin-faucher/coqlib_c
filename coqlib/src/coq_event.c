//
//  coqevent.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-17.
//

#include "coq_event.h"
#include "_nodes/_node_tree.h"

#define _Max_Event_count 64

static CoqEvent           _event_poll[_Max_Event_count];
static CoqEvent* const    _event_poll_end = &_event_poll[_Max_Event_count];
static CoqEvent*          _event_current =  &_event_poll[0];
static CoqEvent*          _event_todo =     &_event_poll[0];

bool CoqEvent_ignoreRepeatKeyDown = true;

void   CoqEvent_addEvent(CoqEvent event) {
    if(_event_current->flags & _event_flag_done) {
        _event_current->flags = 0;
    }
    if(_event_current->flags & _event_flag_toDo) {
        printwarning("Event poll full.");
        return;
    }
    event.flags &= ~ (_event_flag_toDo|_event_flag_done);
    *_event_current = event;
    
    _event_current->flags |= _event_flag_toDo;
    _event_current ++;
    if(_event_current >= _event_poll_end)
        _event_current = _event_poll;
}

void  CoqEvent_processEvents(Root* root) {
    while ((_event_todo->flags & _event_flag_toDo) && !(_event_todo->flags & _event_flag_done)) {
        switch (_event_todo->flags & _event_types) {
                
            case event_type_hovering: {
                Button* const hovered = root_searchButtonOpt(root, _event_todo->touch_pos, NULL);
                Button* const lastHovered = root->buttonSelectedOpt;
                if(lastHovered == hovered) break;
                if(lastHovered) if(lastHovered->stopHoveringOpt)
                    lastHovered->stopHoveringOpt(lastHovered);
                root->buttonSelectedOpt = hovered;
                if(hovered) if(hovered->startHoveringOpt)
                    hovered->startHoveringOpt(hovered);
                break;
            }
                
            case event_type_down: {
                root->lastTouchedPos = _event_todo->touch_pos;
                Button* const lastSelected = root->buttonSelectedOpt;
                if(lastSelected) if(lastSelected->stopHoveringOpt)
                    lastSelected->stopHoveringOpt(lastSelected);
                root->buttonSelectedOpt = NULL;
                // 0. Trouver un bouton sélectionable
                Button* const toTouch = root_searchButtonOpt(root, _event_todo->touch_pos, NULL);
                if(toTouch == NULL) break;
                // 1. Grab le noeud draggable (si on drag on ne select pas)
                if(toTouch->grabOpt) {
                    root->buttonSelectedOpt = toTouch;
                    Vector2 relpos = vector2_absPosToPosInReferentialOfNode(
                                                                            _event_todo->touch_pos, &toTouch->n);
                    toTouch->grabOpt(toTouch, relpos);
                    break;
                }
                // 2. Sinon activer le noeud sélectionné (non grabbable)
                if(toTouch->action)
                    toTouch->action(toTouch);
                break;
            }
                
            case event_type_drag: {
                Button* const grabbed = root->buttonSelectedOpt;
                if(grabbed == NULL) break;
                if(grabbed->dragOpt == NULL) {
                    printwarning("Grabbed node without drag function.");
                    break;
                }
                Vector2 relpos = vector2_absPosToPosInReferentialOfNode(
                                     _event_todo->touch_pos, &grabbed->n);
                grabbed->dragOpt(root->buttonSelectedOpt, relpos);
                break;
            }
                
            case event_type_up: {
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
                slidingmenu_scroll(sm, _event_todo->scroll_deltas.y > 0);
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
                slidingmenu_trackPadScroll(sm, _event_todo->scroll_deltas.y);
                break;
            }
            case event_type_scrollTrackEnd: {
                SlidingMenu* sm = root_searchFirstSlidingMenuOpt(root);
                if(sm == NULL) break;
                slidingmenu_trackPadScrollEnded(sm);
                break;
            }
                
            case event_type_resize: {
                root_setFrame(root, _event_todo->resize_margins, _event_todo->resize_sizesPx, _event_todo->resize_inTransition);
                break;
            }
                
            case event_type_keyDown: {
                View* viewActive = root->viewActiveOpt;
                if(!viewActive) break;
                if((_event_todo->key.keycode == keycode_escape) && viewActive->escapeOpt) {
                    viewActive->escapeOpt(viewActive);
                    break;
                }
                if(((_event_todo->key.keycode == keycode_keypadEnter) ||
                    (_event_todo->key.keycode == keycode_return_)
                   ) && viewActive->enterOpt) {
                    viewActive->enterOpt(viewActive);
                    break;
                }
                if(!viewActive->keyDownOpt) break;
                viewActive->keyDownOpt(viewActive, _event_todo->key);
                break;
            }
            case event_type_keyUp: {
                View* viewActive = root->viewActiveOpt;
                if(!viewActive) break;
                if(!viewActive->keyUpOpt) break;
                viewActive->keyUpOpt(viewActive, _event_todo->key);
                break;
            }
            case event_type_keyMod: {
                View* viewActive = root->viewActiveOpt;
                if(!viewActive) break;
                if(!viewActive->modifiersChangedToOpt) break;
                viewActive->modifiersChangedToOpt(viewActive, _event_todo->key.modifiers);
                break;
            }
                
            default: printwarning("Event %d not defined.", _event_todo->flags & _event_types); break;
        }
        
        _event_todo->flags |= _event_flag_done;
        _event_todo ++;
        if(_event_todo >= _event_poll_end)
            _event_todo = _event_poll;
    }
}
