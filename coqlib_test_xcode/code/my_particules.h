//
//  my_particules.h
//  Piscine de particules qui bougent...
//
//  Created by Corentin Faucher on 2024-01-04.
//
#ifndef my_particules_h
#define my_particules_h

#include "coq_math.h"
#include "coq_nodes.h"

typedef struct _Particule {
    Vector2 pos;
    Vector2 vit;
    Vector2 acc;
    Vector2 pos_last; // Dernière position
    Vector2 vit_mean; // Dernière vitesse moyenne
} Particule;

typedef struct _ParticulesPool {
    uint32_t  count;
    float     deltaX;
    float     deltaY;
    float     r0;        // Rayon des particules
    float     brownian;  // Intensite mouvement brownien/temperature.
    float     repulsion; // Force de repulsion entre particules
    Particule particules[1];
} ParticulesPool;

ParticulesPool* particulespool_create(uint32_t count, float width, float height,
                                      float r0, float brownian, float repulsion);
void            particulespool_update(ParticulesPool* pp, float const deltaT);


typedef struct _PPNode {
    Node       n;
    ParticulesPool* pp;
    Timer*     timer;
} PPNode;

PPNode* PPNode_create(Node* ref);


#endif /* my_enums_h */
