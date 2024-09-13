//
//  sliding_menu.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-01.
//

#ifndef coq_node_sliding_menu_h
#define coq_node_sliding_menu_h

#include "node_base.h"

typedef struct SlidingMenu SlidingMenu;
typedef struct SlidingMenuInit {
    uint32_t displayedCount;
    float    spacing;
    uint32_t relativeLeftRightFlag;
    bool     withoutScrollBar;
    Texture*  backFrameTexOpt;
    Rectangle backFrameUVrect;
} SlidingMenuInit;

SlidingMenu* SlidingMenu_create(Node* ref, SlidingMenuInit data,
                        float x, float y, float width, float height,
                        flag_t flags, uint8_t node_place);
/// Downcasting
SlidingMenu* node_asSlidingMenuOpt(Node* n);
/*-- Getters --*/
float slidingmenu_itemRelativeWidth(SlidingMenu* sm);
float slidingmenu_last_itemRelativeWidth(void);
/** (pour UIScrollView) Retourne menu.height / slidmenu.height. Typiquement > 1 (pas besoine de sliding menu si < 1) */
float slidingmenu_contentFactor(SlidingMenu* sm);
/** (pour UIScrollView) OffsetRatio : Déroulement des UIScrollView par rapport au haut. */
float slidingmenu_offsetRatio(SlidingMenu* sm);
/*-- Actions --*/
void  slidingmenu_addItem(SlidingMenu* sm, Node* toAdd);
void  slidingmenu_last_addItem(Node* toAdd);
void  slidingmenu_removeAll(SlidingMenu* sm);
/** (pour UIScrollView) OffsetRatio : Déroulement des UIScrollView par rapport au haut. */
void slidingmenu_setOffsetRatio(SlidingMenu* sm, float offsetRatio, bool letGo);

void  slidingmenu_scroll(SlidingMenu* sm, int deltaY);
void  slidingmenu_trackPadScrollBegan(SlidingMenu* sm);
void  slidingmenu_trackPadScroll(SlidingMenu* sm, float deltaY);
void  slidingmenu_trackPadScrollEnded(SlidingMenu* sm);

#endif /* sliding_menu_h */
