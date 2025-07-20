//
//  my_particules.h
//  Piscine de particules qui bougent...
//
//  Created by Corentin Faucher on 2024-01-04.
//
#ifndef my_particules_h
#define my_particules_h

#include "nodes/node_base.h"

typedef struct ParticulesPool ParticulesPool;
ParticulesPool* ParticulesPool_create(uint32_t count, float width, float height,
                                      float r0, float brownian, float repulsion);
void            particulespool_update(ParticulesPool* pp);


typedef struct PPNode {
    Node n;
    
    ParticulesPool* pp;
    int64_t         last_time;
    Timer           timer;
} PPNode;

PPNode* PPNode_create(Node* ref);


#endif /* my_enums_h */
