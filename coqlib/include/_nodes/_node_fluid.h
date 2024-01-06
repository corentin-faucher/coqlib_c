//
//  fluid.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-16.
//

#ifndef _coq_node_fluid_h
#define _coq_node_fluid_h

#include "_node.h"
#include "_math_flpos.h"

#define _NODESMOOTH_DIMS_N 5

typedef struct _Fluid {
    Node      n;    // Peut être casté comme un noeud.
    union {
        FluidPos dims[_NODESMOOTH_DIMS_N];
        // Dans le meme ordre que dans Node (en commencant a scales)
        struct {
            FluidPos sx;
            FluidPos sy;  // (pourrait s'ajouter sz... un peu superflu pour l'instant)
            FluidPos x;
            FluidPos y;
            FluidPos z;
        };
    };
} Fluid;

extern float Fluid_defaultFadeInDelta;  // 2.2 par defaut.

Fluid*  Fluid_create(Node* const refOpt, float x, float y, float w, float h,
                              float lambda, flag_t flags, uint8_t node_place);
// Downcasting
Fluid* node_asFluidOpt(Node* n);
// Sub-struct init.
void    _fluid_init(Fluid* f, float lambda);
// Getters
Vector3 fluid_pos(Fluid* s);
// Setters
void    fluid_setXY(Fluid* f, Vector2 xy);
void    fluid_setX(Fluid* f, float x, bool fix);
void    fluid_setXrelToDef(Fluid* f, float x, bool fix);
void    fluid_setY(Fluid* f, float y, bool fix);
void    fluid_setYrelToDef(Fluid* f, float y, bool fix);
void    fluid_setScales(Fluid* f, Vector2 scales, bool fix);

// Internals
void    _fluid_setRelatively(Fluid* const f, const bool fix);
// Les fonctions de base d'ouverture, fermeture, reshape de Node pour sous-classes.
void    fluid_open(Node* node);
void    fluid_close(Node* node);
void    fluid_reshape(Node* node);

#endif /* fluid_h */
