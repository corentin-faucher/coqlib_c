//
//  cellular.h
//  xc_coqlib_test
//
//  Created by Mathieu et Corentin on 3/5/24.
//
#ifndef cellular_h
#define cellular_h

#include "utils/util_base.h"
#include "nodes/node_drawable.h"
#include "coq_timer.h"

// Un noeud (drawable) avec automate cellulaire
typedef struct CelGrid CelGrid;

CelGrid* CelGrid_create(Node* parent, float x, float y, float height, uint32_t m, uint32_t n, bool mathieu);

#endif /* cellular_h */
