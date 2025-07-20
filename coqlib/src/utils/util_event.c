//
//  coqevent.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-17.
//
#include "util_event.h"

#include "util_base.h"
#include "../nodes/node_tree.h"
#include "../nodes/node_root.h"
#include "../nodes/node_sliding_menu.h"
#include "../systems/system_base.h"


void    nodetouch_init(NodeTouch*const nt, NodeTouch const ref) {
    *(Node**)&nt->n = ref.n;
    *(Vector2*)&nt->posAbs = ref.posAbs;
    uint_initConst(&nt->touchId, ref.touchId);
    *(Vector2*)&nt->_posRelOpt = ref._posRelOpt;
    bool_initConst(&nt->_posRelDefined, ref._posRelDefined);
}
Vector2 nodetouch_evalPosRel(NodeTouch const nt) {
    if(nt._posRelDefined) return nt._posRelOpt;
    return vector2_referentialIn(nt.posAbs, node_referentialInParent(nt.n, NULL));
}

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
Root* CoqEvent_root = NULL;
void  CoqEvent_processAllQueuedRootEvents(void) {
    Root*const root = CoqEvent_root;
    if(!root) printerror("CoqEvent_root not defined.");
    while (event_todo_->_todo) {
        if(root) switch (event_todo_->type & event_types_root_) {
        case eventtype_touch_hovering: {
            if_let(View*, viewActive, root->viewActiveOpt)
            Vector2 const posAbs = root_absposFromViewPos(root, event_todo_->touch_info.pos,
                                                    event_todo_->touch_info.yInverted);
            viewActive->touchHovering((NodeTouch) {
                .n = &viewActive->n,
                .posAbs = posAbs,
                .touchId = event_todo_->touch_info.touchId,
            });
            if_let_end
        } break;
        case eventtype_touch_down: {
            if_let(View*, viewActive, root->viewActiveOpt)
            Vector2 const posAbs = root_absposFromViewPos(root, event_todo_->touch_info.pos,
                                                    event_todo_->touch_info.yInverted);
            viewActive->touchDown((NodeTouch) {
                .n = &viewActive->n,
                .posAbs = posAbs,
                .touchId = event_todo_->touch_info.touchId,
            });
            if_let_end
        } break;
        case eventtype_touch_drag: {
            if_let(View*, viewActive, root->viewActiveOpt)
            Vector2 const posAbs = root_absposFromViewPos(root, event_todo_->touch_info.pos,
                                                    event_todo_->touch_info.yInverted);
            viewActive->touchDrag((NodeTouch) {
                .n = &viewActive->n,
                .posAbs = posAbs,
                .touchId = event_todo_->touch_info.touchId,
            });
            if_let_end
        } break;
        case eventtype_touch_up: {
            if_let(View*, viewActive, root->viewActiveOpt)
            Vector2 const posAbs = root_absposFromViewPos(root, event_todo_->touch_info.pos,
                                                    event_todo_->touch_info.yInverted);
            viewActive->touchUp((NodeTouch) {
                .n = &viewActive->n,
                .posAbs = posAbs,
                .touchId = event_todo_->touch_info.touchId,
            });
            if_let_end
        } break;
        case eventtype_scroll: {
            SlidingMenu* sm = (SlidingMenu*)node_tree_searchFirstOfTypeInBranchOpt(&root->n, 
                                            node_type_scrollable, flag_parentOfScrollable);
            if(sm == NULL) break;
            ScrollInfo info = event_todo_->scroll_info;
            switch(event_todo_->scroll_info.scrollType) {
                case scrolltype_scroll:
                    slidingmenu_scroll(sm, info.scroll_deltas.y > 0 ? 1 : -1); break;
                case scrolltype_trackBegin:
                    slidingmenu_trackPadScrollBegan(sm); break;
                case scrolltype_track:
                    slidingmenu_trackPadScroll(sm, info.scroll_deltas.y); break;
                case scrolltype_trackEnd:
                    slidingmenu_trackPadScrollEnded(sm); break;
                case scrolltype_offSet:
                    slidingmenu_setOffsetRatio(sm, info.offset_ratio, info.offset_letGo); break;
                default: printerror("Bad scroll type %d.", event_todo_->scroll_info.scrollType);
            }
        } break;
        case eventtype_key_down: {
            View*const viewActive = root->viewActiveOpt;
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
                SlidingMenu* sm = (SlidingMenu*)node_tree_searchFirstOfTypeInBranchOpt(&root->n, 
                                            node_type_scrollable, flag_parentOfScrollable);
                if(sm) {
                    slidingmenu_scroll(sm, key.keycode == keycode_arrowUp ? 1 : -1);
                    break;
                }
            }
            // Touche quelconque
            if(viewActive->keyDownOpt)
            	viewActive->keyDownOpt(viewActive, key);
        } break;
        case eventtype_key_up: {
            View* viewActive = root->viewActiveOpt;
            if(viewActive && viewActive->keyUpOpt)
            	viewActive->keyUpOpt(viewActive, event_todo_->key); 
        } break;
        case eventtype_key_mod: {
            View*const viewActive = root->viewActiveOpt;
            if(viewActive && viewActive->modifiersChangedToOpt)
            	viewActive->modifiersChangedToOpt(viewActive, event_todo_->key.modifiers);
        } break;
        case eventtype_gamePad_up: {
            View*const viewActive = root->viewActiveOpt;
            if(viewActive && viewActive->gamePadUpOpt)
            	viewActive->gamePadUpOpt(viewActive, event_todo_->gamepadInput);
        } break;
        case eventtype_gamePad_down: {
            View*const viewActive = root->viewActiveOpt;
            if(viewActive && viewActive->gamePadDownOpt)
            	viewActive->gamePadDownOpt(viewActive, event_todo_->gamepadInput);
        } break;
        case eventtype_gamePad_value: {
            View*const viewActive = root->viewActiveOpt;
            if(viewActive && viewActive->gamePadValueOpt)
            	viewActive->gamePadValueOpt(viewActive, event_todo_->gamepadInput);
        } break;
        case eventtype_resize: {
            CoqSystem_setViewSize(event_todo_->resize_info);
            root_viewResized(root, event_todo_->resize_info);
            break;
        }
        case eventtype_systemChanged: {
            View*const viewActive = root->viewActiveOpt;
            if(viewActive && viewActive->systemChangedOpt)
            	viewActive->systemChangedOpt(viewActive, event_todo_->system_change);
        } break;
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
