//
//  sliding_menu.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-01.
//

#ifndef sliding_menu_h
#define sliding_menu_h

#include <stdio.h>
#include "_node.h"
#include "button.h"
#include "drawable.h"


typedef struct _SlidingMenu SlidingMenu;

SlidingMenu* SlidingMenu_create(Node* ref, int displayed_count, float spacing,
                                float x, float y, float width, float height,
                                flag_t flags);
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
void slidingmenu_setOffsetRatio(SlidingMenu* sm, float offsetRatio, Bool letGo);

void  slidingmenu_scroll(SlidingMenu* sm, Bool up);
void  slidingmenu_trackPadScrollBegan(SlidingMenu* sm);
void  slidingmenu_trackPadScroll(SlidingMenu* sm, float deltaY);
void  slidingmenu_trackPadScrollEnded(SlidingMenu* sm);

#endif /* sliding_menu_h */
