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
// Position et vitesse utilisées pour interpoller la position (affichage, actif à tour de rôle)
// Seulement pour lecture...
    Vector2 pos0, pos1; 
    Vector2 vit0, vit1;
// Pos, vitesse et acc pour calculs
    Vector2 _posi;
    Vector2 _posf;
    Vector2 _vit;
    Vector2 _acc;
} Particule;

/// Estimer la position actuelle (interpolation entre `particulespool_update` pour l'affichage)
//Vector2 particule_evalPos(Particule* part, float dT, bool zeroOne);

typedef struct ParticulesPool {
    uint32_t  count;
    float     deltaX;
    float     deltaY;
    float     r0;        // Rayon des particules
    float     brownian;  // Intensite mouvement brownien/temperature.
    float     repulsion; // Force de repulsion entre particules
    // Temps de calcul des positions. Si t1 > t0 => Mode `1` actif, i.e. pos1 et vit1 sont `live`, pos0 et vit0 sont libre pour édition.
    int64_t   t0, t1;    
    int64_t   last_time; 
    Particule particules[1];
} ParticulesPool;

ParticulesPool* ParticulesPool_create(uint32_t count, float width, float height,
                                      float r0, float brownian, float repulsion);
void            particulespool_update(ParticulesPool* pp);


typedef struct PPNode {
    Node n;
    
    ParticulesPool* pp;
//    DrawableMulti*  dm;
    int64_t         last_time;
    Timer*          timer;
} PPNode;

PPNode* PPNode_create(Node* ref);


#endif /* my_enums_h */
