//
//  node_selectable.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#ifndef node_selectable_h
#define node_selectable_h

#include <stdio.h>
#include "node_smooth.h"

typedef struct _Selectable NodeSel;

// Boutons, slider, objet deplacable...
typedef struct _Selectable {
    union {
        Node       nd;      // Peut être casté comme un noeud ou smooth.
        NodeSmooth ns;
    };
    /*-- Activation! --*/
    void (*action)(NodeSel*);
    /*-- Deplacement --*/
    void (*grab)(NodeSel*);
    void (*drag)(NodeSel*);
    void (*letGo)(NodeSel*);
    /*--Survole --*/
    void (*startHovering)(NodeSel*);
    void (*stopHovering)(NodeSel*);
//    void (*showPop)
} NodeSel;

// Selectable quelconque. methodes non definies.
NodeSel* NodeSel_create(Node* const refOpt, float x, float y, float h,
                        float lambda, flag_t flags, uint8_t node_place);


#endif /* node_selectable_h */
