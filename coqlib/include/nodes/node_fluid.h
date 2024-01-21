//
//  fluid.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-16.
//

#ifndef COQ_NODE_FLUID_H
#define COQ_NODE_FLUID_H

#include "node_base.h"
#include "maths/math_flpos.h"

// Nombre de dimensions `fluid`, i.e. le scaling et les positions.
#define COQ_FLUID_DIMS_N 5

typedef struct _Fluid {
    Node      n;    // Peut être casté comme un noeud.
    union {
        FluidPos dims[COQ_FLUID_DIMS_N];
        // Dans le meme ordre que dans Node (en commencant a scales)
        struct {
            FluidPos sx;
            FluidPos sy;
            // FluidPos sz;  // (pourrait s'ajouter sz...
            //           un peu superflu pour l'instant, voir aussi scales dans Node)
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
// Pour initialiser les sub-structs.
void    fluid_init_(Fluid* f, float lambda);
// Getters
Vector3 fluid_pos(Fluid* s);
// Convenience setters
void    fluid_setXY(Fluid* f, Vector2 xy);
void    fluid_setX(Fluid* f, float x, bool fix);
void    fluid_setXrelToDef(Fluid* f, float x, bool fix);
void    fluid_setY(Fluid* f, float y, bool fix);
void    fluid_setYrelToDef(Fluid* f, float y, bool fix);
void    fluid_setScales(Fluid* f, Vector2 scales, bool fix);
void    fluid_setXYScales(Fluid* f, Box box);

// Internals
void    fluid_setRelatively_(Fluid* const f, const bool fix);
// Les fonctions de base d'ouverture, fermeture, reshape de Node pour sous-classes.
void    fluid_open_(Node* node);
void    fluid_close_(Node* node);
void    fluid_reshape_(Node* node);

#endif /* fluid_h */
