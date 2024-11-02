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

#pragma mark - Scroll bar du sliding menu

typedef struct _ScrollBar {
    Node       n;
    Fluid*     nub;
    Frame*     nubFrame;
} ScrollBar;

ScrollBar* _ScrollBar_create(Node* ref, float width) {
    float parWidth =  ref->w;
    float parHeight = ref->h;
    ScrollBar* sb = coq_callocTyped(ScrollBar);
    node_init(&sb->n, ref, 0.5f*parWidth - 0.5f*width, 0, width, parHeight, 0, 0);
    
    // Back of scrollBar
    Frame_create(&sb->n, 1.f, 0.5*width, 0.f, parHeight,
        // texOpt ? texOpt :
        Texture_sharedImageByName("coqlib_scroll_bar_back"), 
        frame_option_verticalBar);
    // Nub (sliding)
    sb->nub = Fluid_create(&sb->n, 0, 0.25f*parHeight, width, width*3, 30, 0, 0);
    sb->nubFrame = Frame_create(&sb->nub->n, 1.f, 0.5f*width, 0.f, 0.25f*parHeight, Texture_sharedImageByName("coqlib_scroll_bar_front"), frame_option_verticalBar);
    
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



#pragma mark - Sliding Menu

typedef struct SlidingMenu {
    Node        n;
    
    uint32_t       openIndex;
    uint32_t const displayedCount;
    uint32_t    totalItemCount;
    float       deltaYMax;
    float const itemHeight;
    float const spacing;
    uint32_t const itemRelativeFlag;
    Fluid*      menu;
    ScrollBar*  scrollBarOpt;
    Frame*      back;
    FluidPos    vitY;
    float       vitYm1;
    Chrono      deltaT;
    Chrono      flingChrono;
    Timer       timer; // (pour checker le fling)
} SlidingMenu;


SlidingMenu* _SlidingMenu_last = NULL;

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
    float const deltaY = sm->deltaYMax;
    if(deltaY == 0.f) {
        fluid_setY(sm->menu, 0, true);
        return;
    }
    // Il faut "snapper" à une position.
    float yCand = snap ? roundf((yCandIn - deltaY) / sm->itemHeight) * sm->itemHeight + deltaY
        : yCandIn;
    yCand = fmaxf(fminf(yCand, deltaY), -deltaY);
    fluid_setY(sm->menu, yCand, fix);
    if(sm->scrollBarOpt)
        _scrollbar_setNubRelY(sm->scrollBarOpt, yCand / deltaY);
}
void  slidingmenu_checkFling_callBack_(void* smIn) {
    SlidingMenu* sm = (SlidingMenu*)smIn;
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
        float deltaY = elapsedSec * fl_evalPos(sm->vitY);
        _slidingmenu_setMenuYpos(sm, fl_real(&sm->menu->y) + deltaY, false, false);
        chrono_start(&sm->deltaT);
    }
    // 2. Vérifier la visibilité des éléments.
    _slidingmenu_checkItemsVisibility(sm, true);
    if(!sm->deltaT.isActive)
        return;
    // 3. Callback
    timer_scheduled(&sm->timer, 40, false, sm, slidingmenu_checkFling_callBack_);
}

void _slidingmenu_open(Node* nd) {
    SlidingMenu* sm = (SlidingMenu*)nd;
    if(!(sm->menu->n.flags & flag_hidden))
        sm->menu->n.flags |= flag_show;
    chrono_stop(&sm->flingChrono);
    chrono_stop(&sm->deltaT);
    // 1. Ajustement de la scroll bar
    float relHeight = (float)sm->displayedCount / fmaxf(1.f, (float)sm->totalItemCount);
    if(sm->scrollBarOpt) {
        if(_scrollbar_setNubHeightWithRelHeight(sm->scrollBarOpt, relHeight))
            sm->back->n.flags &= ~ flag_hidden;
        else
            sm->back->n.flags |= flag_hidden;
    }
    // 3. Normaliser les hauteurs pour avoir itemHeight
    Node* pos = sm->menu->n._firstChild;
    if(!pos) return;
    float smallItemHeight = sm->itemHeight / sm->spacing;
    do {
        float scale = smallItemHeight / pos->h;
        Fluid *f = node_asFluidOpt(pos);
        if(f) fluid_setScales(f, (Vector2){{scale, scale}}, true);
        else  { pos->sx = scale; pos->sy = scale; }
        pos = pos->_littleBro;
    } while(pos);
    // 4. Aligner les éléments et placer au bon endroit.
    float menuWidth = sm->menu->n.w;
    uint32_t alignFlags = node_align_vertically|node_align_fixPos;
    if(sm->itemRelativeFlag & relative_justifiedLeft)
        alignFlags |= node_align_leftTop;
    else if(sm->itemRelativeFlag & relative_justifiedRight)
        alignFlags |= node_align_rightBottom; 
    node_tree_alignTheChildren(&sm->menu->n, alignFlags,
                               1.f, sm->spacing);
                               
    // (On remet la largeur à celle du sliding menu, pas la width max des éléments.)
    sm->menu->n.w = menuWidth;
    float deltaY = sm->deltaYMax;
    if(deltaY > 0.f) {
        _slidingmenu_setMenuYpos(sm, sm->itemHeight * (float)sm->openIndex - deltaY,
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

SlidingMenu* SlidingMenu_create(Node* ref, SlidingMenuInit sid,
                        float x, float y, float width, float height,
                        flag_t flags, const uint8_t node_place) {
    // Init as Node.
    SlidingMenu* const sm = coq_callocTyped(SlidingMenu);
    node_init(&sm->n, ref, x, y, width, height, 
               flags|flag_parentOfButton, node_place);
    // Init as sliding menu
    sm->n._type |= node_type_flag_scrollable;
    node_tree_addRootFlags(&sm->n, flag_parentOfButton|flag_parentOfScrollable|flag_parentOfReshapable);
    sm->n.openOpt = _slidingmenu_open;
    sm->n.closeOpt = slidingmenu_close_;
    sm->n.reshapeOpt = slidingmenu_reshape_;
    sm->n.deinitOpt = _slidingmenu_deinit;
    // Init as Sliding
    uint_initConst(&sm->displayedCount, sid.displayedCount);
    uint_initConst(&sm->itemRelativeFlag, sid.relativeLeftRightFlag);
    float_initConst(&sm->spacing, sid.spacing);
    float_initConst(&sm->itemHeight, height / sid.displayedCount);
    fl_init(&sm->vitY, 0, 4, false);
    // Structure
    float scrollBarWidth = fmaxf(width, height) * 0.025f;
    
    sm->back = Frame_create(&sm->n, 1, scrollBarWidth,
                         0.f, 0.f, 
                         sid.backFrameTexOpt ? sid.backFrameTexOpt :
                         Texture_sharedImageByName("coqlib_sliding_menu_back"), 
                         frame_option_getSizesFromParent);
    if(sid.backFrameTexOpt) {
        drawable_setUVRect(&sm->back->d, sid.backFrameUVrect);
    }
    float menuX = sid.withoutScrollBar ? 0.f : -0.5f*scrollBarWidth;
    sm->menu = Fluid_create(&sm->n, menuX, 0.f,
                            width - (sid.withoutScrollBar ? 0 : scrollBarWidth), height, 20.f,
                            flag_parentOfButton, 0);
    if(!sid.withoutScrollBar)
        sm->scrollBarOpt = _ScrollBar_create(&sm->n, scrollBarWidth);
    _SlidingMenu_last = sm;
    
    return sm;
}
SlidingMenu* node_asSlidingMenuOpt(Node* n) {
    if(n->_type & node_type_flag_scrollable) return (SlidingMenu*)n;
    return NULL;
}

/*-- Getters --*/
float slidingmenu_itemRelativeWidth(SlidingMenu* sm) {
    return sm->menu->n.w / sm->n.h * ((float)sm->displayedCount) * sm->spacing;
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
    float deltaY = sm->deltaYMax;
    if(deltaY == 0.f) return 0.f;
    return (fl_real(&sm->menu->y) + deltaY) / sm->menu->n.h;
}
/*-- Actions --*/
void  slidingmenu_addItem(SlidingMenu* sm, Node* toAdd) {
    node_simpleMoveToParent(toAdd, &sm->menu->n, false);
    sm->totalItemCount ++;
    sm->deltaYMax = 0.5f * (sm->n.h / sm->displayedCount) * fmaxf(0.f, (float)sm->totalItemCount - (float)sm->displayedCount);
}
void  slidingmenu_last_addItem(Node* toAdd) {
    if(_SlidingMenu_last == NULL) {
        printerror("No sliding menu created."); return;
    }
    slidingmenu_addItem(_SlidingMenu_last, toAdd);
}
void  slidingmenu_removeAll(SlidingMenu* sm) {
    while(sm->menu->n._firstChild)
        node_throwToGarbage_(sm->menu->n._firstChild);
    sm->totalItemCount = 0;
    sm->deltaYMax = 0;
}
/** (pour UIScrollView) OffsetRatio : Déroulement des UIScrollView par rapport au haut. */
void slidingmenu_setOffsetRatio(SlidingMenu* sm, float offsetRatio, bool letGo) {
    float const deltaY = sm->deltaYMax;
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
    _slidingmenu_checkItemsVisibility(sm, true);
    if(sm->scrollBarOpt)
        _scrollbar_setNubRelY(sm->scrollBarOpt, fl_real(&sm->menu->y) / deltaY);
}

/*-- Scrollable (pour macOS et iPadOS avec trackpad/souris)     --*/
/*!! Pour aller vers le bas du menu -> scrollDeltaY < 0,        !!
  !! il faut envoyer le menu vers le haut -> menuDeltaY > 0 ... !!*/
void  slidingmenu_scroll(SlidingMenu* sm, int deltaYInt) {
    float deltaY = -sm->itemHeight * (float)deltaYInt;
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
    
    slidingmenu_checkFling_callBack_(sm);
}
