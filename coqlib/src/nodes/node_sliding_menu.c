//
//  sliding_menu.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-01.
//

#include "node_sliding_menu.h"

#include "node_tree.h"
#include "node_root.h"
#include "../utils/util_base.h"
//#include "coq_event.h"

typedef struct _ScrollBar {
    Node       n;
    Fluid*     nub;
    Frame*     nubFrame;
} ScrollBar;

typedef struct _SlidingMenu {
    union {
        Node    n;
        // Fluid aussi ?  Fluid   f;
    };
    uint32_t    openPos;
    uint32_t    displayed_count;
    uint32_t    total_count;
    float       spacing;
    Fluid*      menu;
//    Frame*      scrollBar;
    ScrollBar*  scrollBar;
    Frame*      back;
    FluidPos   vitY;
    float       vitYm1;
    Chrono      deltaT;
    Chrono      flingChrono;
    Timer*      timer; // (pour checker le fling)
} SlidingMenu;

ScrollBar* _ScrollBar_create(SlidingMenu* sm, float width) {
    float parWidth = sm->n.w;
    float parHeight = sm->n.h;
    ScrollBar* sb = coq_calloc(1, sizeof(ScrollBar));
    node_init_(&sb->n, &sm->n, 0.5f*parWidth - 0.5f*width, 0, width, parHeight, node_type_node, 0, 0);
    
    // Back of scrollBar
    Frame_createWithName(&sb->n, 1.f, 0.5*width, 0.f, parHeight, "coqlib_scroll_bar_back", frame_option_verticalBar);
    // Nub (sliding)
    sb->nub = Fluid_create(&sb->n, 0, 0.25f*parHeight, width, width*3, 30, 0, 0);
    sb->nubFrame = Frame_createWithName(&sb->nub->n, 1.f, 0.5f*width, 0.f, 0.25f*parHeight, "coqlib_scroll_bar_front", frame_option_verticalBar);
    
    return sb;
}
// Retourne false si hidden, true si affiche
bool _scrollbar_setNubHeightWithRelHeight(ScrollBar* sb, float newRelHeight) {
    if(newRelHeight >= 1.f || newRelHeight <= 0.f) {
        node_tree_hideAndTryToClose(&sb->n);
        return false;
    }
    sb->n.flags &= ~ flag_hidden;
    float nubHeight = sb->n.h * newRelHeight;
    sb->nub->n.h = nubHeight;
    sb->nubFrame->setDims(sb->nubFrame, (Vector2){{0.f, 0.5f*nubHeight}});
    return true;
}
void _scrollbar_setNubRelY(ScrollBar* sb, float newRelY) {
    float deltaY = 0.5f*(sb->n.h - sb->nub->n.h);
    fluid_setY(sb->nub, -newRelY * deltaY, false);
//    fl_set(&sb->nub->y, -newRelY * deltaY);
}

SlidingMenu* _SlidingMenu_last = NULL;
#define _sm_itemHeight (sm->n.h / sm->displayed_count)
/// La liberte en y du menu.
#define _sm_menuDeltaYMax \
0.5f * (sm->n.h / sm->displayed_count) * fmaxf(0.f, (float)sm->total_count - (float)sm->displayed_count)
void  _slidingmenu_checkItemsVisibility(SlidingMenu* sm, bool openNode) {
    Node* pos = sm->menu->n._firstChild;
    // 0. Sortir s'il n'y a rien.
    if(!pos || !(sm->menu->n.flags & flag_show)) {
        chrono_stop(&sm->flingChrono);
        chrono_stop(&sm->deltaT);
        return;
    }
    // 1. Ajuster la visibilité des items
    float yActual = fl_real(&sm->menu->y);
    do {
        bool toShow = fabsf(yActual + pos->y) < 0.5f * sm->n.h;
        if(toShow && (pos->flags & flag_hidden)) {
            pos->flags &= ~ flag_hidden;
            if(openNode)
                node_tree_openAndShow(pos);
        }
        if(!toShow && !(pos->flags & flag_hidden)) {
            pos->flags |= flag_hidden;
            if(openNode)
                node_tree_close(pos);
        }
        pos = pos->_littleBro;
    } while(pos);
}
void  _slidingmenu_setMenuYpos(SlidingMenu* sm, float yCandIn, bool snap, bool fix) {
    float deltaY = _sm_menuDeltaYMax;
    if(deltaY == 0.f) {
        fluid_setY(sm->menu, 0, true);
        return;
    }
    // Il faut "snapper" à une position.
    float yCand = snap ? roundf((yCandIn - deltaY) / _sm_itemHeight) * _sm_itemHeight + deltaY
        : yCandIn;
    yCand = fmaxf(fminf(yCand, deltaY), -deltaY);
    fluid_setY(sm->menu, yCand, fix);
    _scrollbar_setNubRelY(sm->scrollBar, yCand / deltaY);
}
void  _slidingmenu_checkFling(Node* nd) {
    SlidingMenu* sm = (SlidingMenu*)nd;
    if(!sm->flingChrono.isActive)
        return;
    // 1. Mise à jour automatique de la position en y.
    if(chrono_elapsedMS(&sm->flingChrono) > 100) {
        fl_set(&sm->vitY, 0.f); // Ralentissement.
        // OK, on arrête le "fling" après une seconde...
        if(chrono_elapsedMS(&sm->flingChrono) > 1000) {
            chrono_stop(&sm->flingChrono);
            chrono_stop(&sm->deltaT);
            _slidingmenu_setMenuYpos(sm, fl_real(&sm->menu->y), true, false);
            return;
        }
    }
    float elapsedSec = chrono_elapsedSec(&sm->deltaT);
    if(elapsedSec > 0.030f) {
        float deltaY = elapsedSec * fl_pos(&sm->vitY);
        _slidingmenu_setMenuYpos(sm, fl_real(&sm->menu->y) + deltaY, false, false);
        chrono_start(&sm->deltaT);
    }
    // 2. Vérifier la visibilité des éléments.
    _slidingmenu_checkItemsVisibility(sm, true);
    if(!sm->deltaT.isActive)
        return;
    // 3. Callback
    timer_scheduled(&sm->timer, 40, false, nd, _slidingmenu_checkFling);
}

void _slidingmenu_open(Node* nd) {
    SlidingMenu* sm = (SlidingMenu*)nd;
    if(!(sm->menu->n.flags & flag_hidden))
        sm->menu->n.flags |= flag_show;
    chrono_stop(&sm->flingChrono);
    chrono_stop(&sm->deltaT);
    // 1. Ajustement de la scroll bar
    float relHeight = (float)sm->displayed_count / fmaxf(1.f, (float)sm->total_count);
    if(_scrollbar_setNubHeightWithRelHeight(sm->scrollBar, relHeight))
        sm->back->n.flags &= ~ flag_hidden;
    else
        sm->back->n.flags |= flag_hidden;
    // 3. Normaliser les hauteurs pour avoir itemHeight
    Node* pos = sm->menu->n._firstChild;
    if(!pos) return;
    float smallItemHeight = _sm_itemHeight / sm->spacing;
    do {
        float scale = smallItemHeight / pos->h;
        Fluid *f = node_asFluidOpt(pos);
        if(f) fluid_setScales(f, (Vector2){{scale, scale}}, true);
        else  { pos->sx = scale; pos->sy = scale; }
        pos = pos->_littleBro;
    } while(pos);
    // 4. Aligner les éléments et placer au bon endroit.
    float menuWidth = sm->menu->n.w;
    node_tree_alignTheChildren(&sm->menu->n, node_align_vertically|node_align_fixPos,
                               1.f, sm->spacing);
    // (On remet la largeur à celle du sliding menu, pas la width max des éléments.)
    sm->menu->n.w = menuWidth;
    float deltaY = _sm_menuDeltaYMax;
    if(deltaY > 0.f) {
        _slidingmenu_setMenuYpos(sm, _sm_itemHeight * (float)sm->openPos - deltaY,
                                 true, true);
    } else {
        _slidingmenu_setMenuYpos(sm, 0.f, true, true);
    }
    _slidingmenu_checkItemsVisibility(sm, false);
    // 5. Signaler sa présence (pour iOS)
#ifdef __APPLE__
#if TARGET_OS_OSX != 1
    if(deltaY > 0.f) {
        Rectangle rect = node_windowRectangle(&sm->n, true);
        float contentFactor = slidingmenu_contentFactor(sm);
        float offSetRatio = slidingmenu_offsetRatio(sm);
        
        CoqEvent_addToWindowEvent((CoqEventWin) {
            .type = event_type_win_ios_scrollviewNeeded,
            .win_scrollViewInfo = { rect, contentFactor, offSetRatio },
        });
    }
#endif
#endif
    // 6. Open "node" : fadeIn, relativePos...
    // ?
}
void slidingmenu_reshape_(Node* n) {
#ifdef __APPLE__
#if TARGET_OS_OSX != 1
    SlidingMenu* sm = (SlidingMenu*)n;
    float deltaY = _sm_menuDeltaYMax;
    if(deltaY > 0.f) {
        Rectangle rect = node_windowRectangle(&sm->n, true);
        float contentFactor = slidingmenu_contentFactor(sm);
        float offSetRatio = slidingmenu_offsetRatio(sm);
        CoqEvent_addToWindowEvent((CoqEventWin) {
            .type = event_type_win_ios_scrollviewNeeded,
            .win_scrollViewInfo = { rect, contentFactor, offSetRatio },
        });
    }
#endif
#endif
}
void slidingmenu_close_(Node* nd) {
#ifdef __APPLE__
#if TARGET_OS_OSX != 1
    CoqEvent_addToWindowEvent((CoqEventWin) {
        .type = event_type_win_ios_scrollviewNotNeeded,
    });
#endif
#endif
}
void _slidingmenu_deinit(Node* n) {
    SlidingMenu* sm = (SlidingMenu*)n;
    timer_cancel(&sm->timer);
}

SlidingMenu* SlidingMenu_create(Node* ref, int displayed_count, float spacing,
                                float x, float y, float width, float height,
                                flag_t flags) {
    // Init as Node.
    SlidingMenu* sm = coq_calloc(1, sizeof(SlidingMenu));
    node_init_(&sm->n, ref, x, y, width, height, node_type_scrollable, flags|flag_parentOfButton, 0);
    node_tree_addRootFlags(&sm->n, flag_parentOfButton|flag_parentOfScrollable|flag_parentOfReshapable);
    sm->n.openOpt = _slidingmenu_open;
    sm->n.closeOpt = slidingmenu_close_;
    sm->n.reshapeOpt = slidingmenu_reshape_;
    sm->n.deinitOpt = _slidingmenu_deinit;
    // Init as Sliding
    sm->displayed_count = displayed_count;
    sm->spacing = spacing;
    fl_init(&sm->vitY, 0, 4, false);
    // Structure
    float scrollBarWidth = fmaxf(width, height) * 0.025f;
    sm->back = Frame_createWithName(&sm->n, 1, scrollBarWidth,
                         0.f, 0.f, "coqlib_sliding_menu_back", frame_option_getSizesFromParent);
    sm->menu = Fluid_create(&sm->n, -0.5f*scrollBarWidth, 0.f,
                                 width - scrollBarWidth, height, 20.f,
                                 flag_parentOfButton, 0);
    sm->scrollBar = _ScrollBar_create(sm, scrollBarWidth);
    _SlidingMenu_last = sm;
    
    return sm;
}
SlidingMenu* node_asSlidingMenuOpt(Node* n) {
    if(n->_type & node_type_flag_scrollable) return (SlidingMenu*)n;
    return NULL;
}

/*-- Getters --*/
float slidingmenu_itemRelativeWidth(SlidingMenu* sm) {
    return sm->menu->n.w / sm->n.h * ((float)sm->displayed_count) * sm->spacing;
}
float slidingmenu_last_itemRelativeWidth(void) {
    if(_SlidingMenu_last == NULL) {
        printerror("No sliding menu created."); return 1.f;
    }
    return slidingmenu_itemRelativeWidth(_SlidingMenu_last);
}
float slidingmenu_contentFactor(SlidingMenu* sm) {
    return sm->menu->n.h / sm->n.h;
}
float slidingmenu_offsetRatio(SlidingMenu* sm) {
    float deltaY = _sm_menuDeltaYMax;
    if(deltaY == 0.f) return 0.f;
    return (fl_real(&sm->menu->y) + deltaY) / sm->menu->n.h;
}
/*-- Actions --*/
void  slidingmenu_addItem(SlidingMenu* sm, Node* toAdd) {
    node_simpleMoveToParent(toAdd, &sm->menu->n, false);
    sm->total_count ++;
}
void  slidingmenu_last_addItem(Node* toAdd) {
    if(_SlidingMenu_last == NULL) {
        printerror("No sliding menu created."); return;
    }
    slidingmenu_addItem(_SlidingMenu_last, toAdd);
}
void  slidingmenu_removeAll(SlidingMenu* sm) {
    while(sm->menu->n._firstChild)
        node_tree_throwToGarbage(sm->menu->n._firstChild);
    sm->total_count = 0;
}
/** (pour UIScrollView) OffsetRatio : Déroulement des UIScrollView par rapport au haut. */
void slidingmenu_setOffsetRatio(SlidingMenu* sm, float offsetRatio, bool letGo) {
    float deltaY = _sm_menuDeltaYMax;
    if(deltaY == 0.f) {
        fluid_setY(sm->menu, 0, true);
//        fl_fix(&sm->menu->y, 0.f);
        return;
    }
    float newy = sm->menu->n.h * offsetRatio - deltaY;
    if(letGo)
        _slidingmenu_setMenuYpos(sm, newy, true, false);
    else
        fluid_setY(sm->menu, newy, false);
//        fl_fix(&sm->menu->y, newy);
    _scrollbar_setNubRelY(sm->scrollBar, fl_real(&sm->menu->y) / deltaY);
    _slidingmenu_checkItemsVisibility(sm, true);
}

/*-- Scrollable (pour macOS et iPadOS avec trackpad/souris)     --*/
/*!! Pour aller vers le bas du menu -> scrollDeltaY < 0,        !!
  !! il faut envoyer le menu vers le haut -> menuDeltaY > 0 ... !!*/
void  slidingmenu_scroll(SlidingMenu* sm, bool up) {
    float deltaY = _sm_itemHeight * (up ? -1.f : 1.f);
    _slidingmenu_setMenuYpos(sm, fl_real(&sm->menu->y) + deltaY, true, false);
    _slidingmenu_checkItemsVisibility(sm, true);
}
void  slidingmenu_trackPadScrollBegan(SlidingMenu* sm) {
    chrono_stop(&sm->flingChrono);
    sm->vitYm1 = 0.f;
    fl_fix(&sm->vitY, 0.f);
    chrono_start(&sm->deltaT);
}
void  slidingmenu_trackPadScroll(SlidingMenu* sm, float deltaY) {
    float menuDeltaY = -0.15f * deltaY;
    _slidingmenu_setMenuYpos(sm, fl_real(&sm->menu->y) + menuDeltaY, false, false);
    _slidingmenu_checkItemsVisibility(sm, true);
    float elapsedSec = chrono_elapsedMS(&sm->deltaT);
    if(elapsedSec > 0) {
        sm->vitYm1 = fl_real(&sm->vitY);
        fl_fix(&sm->vitY, menuDeltaY / elapsedSec);
    }
    chrono_start(&sm->deltaT);
}
void  slidingmenu_trackPadScrollEnded(SlidingMenu* sm) {
    fl_fix(&sm->vitY, 0.5f*(fl_real(&sm->vitY) + sm->vitYm1));
    if(fabsf(fl_real(&sm->vitY)) < 6.f) {
        _slidingmenu_setMenuYpos(sm, fl_real(&sm->menu->y), true, false);
        return;
    }
    chrono_start(&sm->deltaT);
    chrono_start(&sm->flingChrono);
    
    _slidingmenu_checkFling(&sm->n);
}
