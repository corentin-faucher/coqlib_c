//
//  sliding_menu.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-01.
//

#include "sliding_menu.h"
#include "node_tree.h"
#include <math.h>

typedef struct _ScrollBar {
    Node        n;
    Fluid* nub;
#warning TODO faire vertical bar...?
    Drawable*   nubTop;
    Drawable*   nubMid;
    Drawable*   nubBot;
} ScrollBar;

typedef struct _SlidingMenu {
    Node        n;
    uint32_t    openPos;
    uint32_t    displayed_count;
    uint32_t    total_count;
    float       spacing;
    Fluid*      menu;
    ScrollBar*  scrollBar;
    Frame*      back;
    SmoothPos   vitY;
    float       vitYm1;
    Chrono      deltaT;
    Chrono      flingChrono;
} SlidingMenu;

ScrollBar* _ScrollBar_create(SlidingMenu* sm, float width) {
    ScrollBar* sb = _Node_createEmptyOfType(node_type_bare, sizeof(ScrollBar),
                                       0, &sm->n, 0);
    float parWidth = sm->n.w;
    float parHeight = sm->n.h;
    sb->n.x = 0.5f*parWidth - 0.5f*width;
    sb->n.w = width;
    sb->n.h = parHeight;
    Texture* backTex = Texture_sharedImageByName("coqlib_scroll_bar_back");
    Texture* frontTex = Texture_sharedImageByName("coqlib_scroll_bar_front");
    // Back of scrollBar
    // Top back
    Drawable* surf = _Drawable_create(&sb->n, 0, 0.5*parHeight - 0.5*width, 0, 0,
                     backTex, mesh_sprite);
    drawable_updateDimsWithDeltas(surf, width, width);
    // Mid back
    surf = _Drawable_create(&sb->n, 0, 0, flag_drawableDontRespectRatio, 0,
                     backTex, mesh_sprite);
    drawable_updateDimsWithDeltas(surf, width, parHeight - 2*width);
    drawable_setTile(surf, 1, 0);
    // Bottom back
    surf = _Drawable_create(&sb->n, 0, -0.5*parHeight + 0.5*width, 0, 0,
                            backTex, mesh_sprite);
    drawable_updateDimsWithDeltas(surf, width, width);
    drawable_setTile(surf, 2, 0);
    
    // Nub (sliding)
    sb->nub = Fluid_create(&sb->n, 0, 0.25f*parHeight, width, width*3, 30, 0, 0);
    sb->nubTop = _Drawable_create(&sb->nub->n, 0, width, 0, 0,
                                  frontTex, mesh_sprite);
    drawable_updateDimsWithDeltas(sb->nubTop, width, width);
    
    sb->nubMid = _Drawable_create(&sb->nub->n, 0, 0, flag_drawableDontRespectRatio, 0,
                                  frontTex, mesh_sprite);
    drawable_updateDimsWithDeltas(sb->nubMid, width, width);
    drawable_setTile(sb->nubMid, 1, 0);
    
    sb->nubBot = _Drawable_create(&sb->nub->n, 0, -width, 0, 0,
                                  frontTex, mesh_sprite);
    drawable_updateDimsWithDeltas(sb->nubBot, width, width);
    drawable_setTile(sb->nubBot, 2, 0);
    
    return sb;
}
// Retourne false si hidden, true si affiche
Bool _scrollbar_setNubHeightWithRelHeight(ScrollBar* sb, float newRelHeight) {
    if(newRelHeight >= 1.f || newRelHeight <= 0.f) {
        node_tree_hideAndTryToClose(&sb->n);
        return false;
    }
    sb->n.flags &= ~ flag_hidden;
    float w = sb->n.w;
    float htmp = sb->n.h * newRelHeight;
    float hmid = fmaxf(0.f, htmp - 2.f*w);
    sb->nub->n.h = hmid + 2.f*w;
    sb->nubTop->n.y =  0.5f*(hmid + w);
    sb->nubBot->n.y = -0.5f*(hmid + w);
    sb->nubMid->n.sy = hmid;

    return true;
}
void _scrollbar_setNubRelY(ScrollBar* sb, float newRelY) {
    float deltaY = 0.5f*(sb->n.h - sb->nub->n.h);
    fluid_setY(sb->nub, -newRelY * deltaY, false);
}

SlidingMenu* _SlidingMenu_last = NULL;
#define _sm_itemHeight (sm->n.h / sm->displayed_count)
/// La liberte en y du menu.
#define _sm_menuDeltaYMax \
0.5f * (sm->n.h / sm->displayed_count) * fmaxf(0.f, (float)sm->total_count - (float)sm->displayed_count)
void  _slidingmenu_checkItemsVisibility(SlidingMenu* sm, Bool openNode) {
    Node* pos = sm->menu->n.firstChild;
    // 0. Sortir s'il n'y a rien.
    if(!pos || !(sm->menu->n.flags & flag_show)) {
        chrono_stop(&sm->flingChrono);
        chrono_stop(&sm->deltaT);
        return;
    }
    // 1. Ajuster la visibilité des items
    float yActual = sm->menu->n.y;
    do {
        Bool toShow = fabsf(yActual + pos->y) < 0.5f * sm->n.h;
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
        pos = pos->littleBro;
    } while(pos);
}
void  _slidingmenu_setMenuYpos(SlidingMenu* sm, float yCandIn, Bool snap, Bool fix) {
    float deltaY = _sm_menuDeltaYMax;
    if(deltaY == 0.f) {
        fluid_setY(sm->menu, 0.f, true);
        return;
    }
    // Il faut "snapper" à une position.
    float yCand = snap ? roundf((yCandIn - deltaY) / _sm_itemHeight) * _sm_itemHeight + deltaY
        : yCandIn;
    fluid_setY(sm->menu, fmaxf(fminf(yCand, deltaY), -deltaY), fix);
    _scrollbar_setNubRelY(sm->scrollBar, sm->menu->n.y / deltaY);
}
void  _slidingmenu_checkFling(Node* nd) {
    SlidingMenu* sm = (SlidingMenu*)nd;
    if(!sm->flingChrono.isActive)
        return;
    // 1. Mise à jour automatique de la position en y.
    if(chrono_elapsedMS(&sm->flingChrono) > 100) {
        sp_set(&sm->vitY, 0.f, false); // Ralentissement.
        // OK, on arrête le "fling" après une seconde...
        if(chrono_elapsedMS(&sm->flingChrono) > 1000) {
            chrono_stop(&sm->flingChrono);
            chrono_stop(&sm->deltaT);
            _slidingmenu_setMenuYpos(sm, sm->menu->n.y, true, false);
            return;
        }
    }
    float elapsedSec = chrono_elapsedSec(&sm->deltaT);
    if(elapsedSec > 0.030f) {
        float deltaY = elapsedSec * sp_pos(&sm->vitY);
        _slidingmenu_setMenuYpos(sm, sm->menu->n.y + deltaY, false, false);
        chrono_start(&sm->deltaT);
    }
    // 2. Vérifier la visibilité des éléments.
    _slidingmenu_checkItemsVisibility(sm, true);
    if(!sm->deltaT.isActive)
        return;
    // 3. Callback
    timer_scheduled(NULL, 40, false, nd, _slidingmenu_checkFling);
}

void  _slidingmenu_open(Node* nd) {
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
    Node* pos = sm->menu->n.firstChild;
    if(!pos) return;
    float smallItemHeight = _sm_itemHeight / sm->spacing;
    do {
        float scale = smallItemHeight / pos->h;
        node_setScaleX(pos, scale, true);
        node_setScaleY(pos, scale, true);
        pos = pos->littleBro;
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
#warning todo ios
    // 6. Open "node" : fadeIn, relativePos...
    // ?
}
void _slidingmenu_close(Node* nd) {
#warning todo ios
}

SlidingMenu* SlidingMenu_create(Node* ref, int displayed_count, float spacing,
                                float x, float y, float width, float height,
                                flag_t flags) {
    // Init as Node.
    SlidingMenu* sm = _Node_createEmptyOfType(node_type_scrollable, sizeof(SlidingMenu),
                                       flags|flag_parentOfButton, ref, 0);
    node_tree_addRootFlag(&sm->n, flag_parentOfButton|flag_parentOfScrollable);
    sm->n.x = x;
    sm->n.y = y;
    sm->n.w = width;
    sm->n.h = height;
    sm->n.open = _slidingmenu_open;
    sm->n.close = _slidingmenu_close;
    // Init as Sliding
    sm->displayed_count = displayed_count;
    sm->spacing = spacing;
    sp_init(&sm->vitY, 0, 4);
    // Structure
    float scrollBarWidth = fmaxf(width, height) * 0.025f;
    Texture* frameTex = Texture_sharedImageByName("coqlib_sliding_menu_back");
    sm->back = Frame_createWithTex(&sm->n, 1, scrollBarWidth,
                         0.f, 0.f, frameTex, frametype_getSizesFromParent);
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
    return (sm->menu->n.y + deltaY) / sm->menu->n.h;
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
    while(sm->menu->n.firstChild)
        node_tree_throwToGarbage(sm->menu->n.firstChild);
    sm->total_count = 0;
}
/** (pour UIScrollView) OffsetRatio : Déroulement des UIScrollView par rapport au haut. */
void slidingmenu_setOffsetRatio(SlidingMenu* sm, float offsetRatio, Bool letGo) {
    float deltaY = _sm_menuDeltaYMax;
    if(deltaY == 0.f) {
        fluid_setY(sm->menu, 0.f, true);
        return;
    }
    float newy = sm->menu->n.h * offsetRatio - deltaY;
    if(letGo)
        _slidingmenu_setMenuYpos(sm, newy, true, false);
    else
        fluid_setY(sm->menu, newy, true);
    _scrollbar_setNubRelY(sm->scrollBar, sm->menu->n.y / deltaY);
    _slidingmenu_checkItemsVisibility(sm, true);
}

/*-- Scrollable (pour macOS et iPadOS avec trackpad/souris)     --*/
/*!! Pour aller vers le bas du menu -> scrollDeltaY < 0,        !!
  !! il faut envoyer le menu vers le haut -> menuDeltaY > 0 ... !!*/
void  slidingmenu_scroll(SlidingMenu* sm, Bool up) {
    float deltaY = _sm_itemHeight * (up ? -1.f : 1.f);
    _slidingmenu_setMenuYpos(sm, sm->menu->n.y + deltaY, true, false);
    _slidingmenu_checkItemsVisibility(sm, true);
}
void  slidingmenu_trackPadScrollBegan(SlidingMenu* sm) {
    chrono_stop(&sm->flingChrono);
    sm->vitYm1 = 0.f;
    sp_set(&sm->vitY, 0.f, true);
    chrono_start(&sm->deltaT);
}
void  slidingmenu_trackPadScroll(SlidingMenu* sm, float deltaY) {
    float menuDeltaY = -0.15f * deltaY;
    _slidingmenu_setMenuYpos(sm, sm->menu->n.y + menuDeltaY, false, false);
    _slidingmenu_checkItemsVisibility(sm, true);
    float elapsedSec = chrono_elapsedMS(&sm->deltaT);
    if(elapsedSec > 0) {
        sm->vitYm1 = sp_real(&sm->vitY);
        sp_set(&sm->vitY, menuDeltaY / elapsedSec, true);
    }
    chrono_start(&sm->deltaT);
}
void  slidingmenu_trackPadScrollEnded(SlidingMenu* sm) {
    sp_set(&sm->vitY, 0.5f*(sp_real(&sm->vitY) + sm->vitYm1), true);
    if(fabsf(sp_real(&sm->vitY)) < 6.f) {
        _slidingmenu_setMenuYpos(sm, sm->menu->n.y, true, false);
        return;
    }
    chrono_start(&sm->deltaT);
    chrono_start(&sm->flingChrono);
    
    _slidingmenu_checkFling(&sm->n);
}
