//
//  coqevent.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-17.
//

#include "coqevent.h"
#include "maths.h"
#include "node_tree.h"
#include "button.h"

#define _Max_Event_count 64

static CoqEvent           _event_poll[_Max_Event_count];
static CoqEvent* const    _event_poll_end = &_event_poll[_Max_Event_count];
static CoqEvent*          _event_current =  &_event_poll[0];
static CoqEvent*          _event_todo =     &_event_poll[0];

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
                Button* const hovered = root_searchButtonOpt(root, _event_todo->pos, NULL);
                Button* const lastHovered = root->selectedButton;
                if(lastHovered == hovered) break;
                if(lastHovered) if(lastHovered->stopHovering)
                    lastHovered->stopHovering(lastHovered);
                root->selectedButton = hovered;
                if(hovered) if(hovered->startHovering)
                    hovered->startHovering(hovered);
                break;
            }
                
            case event_type_down: {
                root->lastTouchedPos = _event_todo->pos;
                Button* const lastSelected = root->selectedButton;
                if(lastSelected) if(lastSelected->stopHovering)
                    lastSelected->stopHovering(lastSelected);
                root->selectedButton = NULL;
                // 0. Trouver un bouton sélectionable
                Button* const toTouch = root_searchButtonOpt(root, _event_todo->pos, NULL);
                if(toTouch == NULL) break;
                // 1. Grab le noeud draggable (si on drag on ne select pas)
                if(toTouch->grab) {
                    root->selectedButton = toTouch;
                    Vector2 relpos = vector2_absPosToPosInReferentialOfNode(
                                                                            _event_todo->pos, &toTouch->n);
                    toTouch->grab(toTouch, relpos);
                    break;
                }
                // 2. Sinon activer le noeud sélectionné (non grabbable)
                if(toTouch->action)
                    toTouch->action(toTouch);
                break;
            }
                
            case event_type_drag: {
                Button* const grabbed = root->selectedButton;
                if(grabbed == NULL) break;
                if(grabbed->drag == NULL) {
                    printwarning("Grabbed node without drag function.");
                    break;
                }
                Vector2 relpos = vector2_absPosToPosInReferentialOfNode(
                                                                        _event_todo->pos, &grabbed->n);
                grabbed->drag(root->selectedButton, relpos);
                break;
            }
                
            case event_type_up: {
                Button* const grabbed = root->selectedButton;
                if(grabbed == NULL) break;
                if(grabbed->letGo) {
                    grabbed->letGo(root->selectedButton);
                } else {
                    printwarning("Grabbed node without letGo function.");
                }
                root->selectedButton = NULL;
                break;
            }
                
            case event_type_scroll: {
                SlidingMenu* sm = root_searchFirstSlidingMenuOpt(root);
                if(sm == NULL) break;
                slidingmenu_scroll(sm, _event_todo->deltas.y > 0);
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
                slidingmenu_trackPadScroll(sm, _event_todo->deltas.y);
                break;
            }
            case event_type_scrollTrackEnd: {
                SlidingMenu* sm = root_searchFirstSlidingMenuOpt(root);
                if(sm == NULL) break;
                slidingmenu_trackPadScrollEnded(sm);
                break;
            }
                
            case event_type_resize: {
                root_setFrame(root, _event_todo->margins, _event_todo->sizesPx, _event_todo->inTransition);
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
