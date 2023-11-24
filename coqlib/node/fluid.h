//
//  fluid.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-16.
//

#ifndef fluid_h
#define fluid_h

#include "_node.h"
#include "smpos.h"

#define _NODESMOOTH_DIMS_N 5

typedef struct _Fluid {
    Node      n;    // Peut être casté comme un noeud.
    union {
        SmoothPos dims[_NODESMOOTH_DIMS_N];
        // Dans le meme ordre que dans Node (en commencant a scales)
        struct {
            SmoothPos sx;
            SmoothPos sy;  // (pourrait s'ajouter sz... un peu superflu pour l'instant)
            SmoothPos x;
            SmoothPos y;
            SmoothPos z;
        };
    };
} Fluid;

extern float Fluid_defaultFadeInDelta;  // 2.2 par defaut.

Fluid*  Fluid_create(Node* const refOpt, float x, float y, float w, float h,
                              float lambda, flag_t flags, uint8_t node_place);
// Sub-struct init.
void    _fluid_init(Fluid* f, float lambda);
// Setters
void    fluid_setDefPos(Fluid* const f, Vector3 defPos);
void    fluid_setX(Fluid* f, float x, Bool fix);
void    fluid_setY(Fluid* f, float y, Bool fix);
void    fluid_setZ(Fluid* f, float z, Bool fix);
void    fluid_setScaleX(Fluid* f, float sx, Bool fix);
void    fluid_setScaleY(Fluid* f, float sy, Bool fix);
void    fluid_setXRelToDef(Fluid* f, float xShift, Bool fix);
void    fluid_setYRelToDef(Fluid* f, float yShift, Bool fix);
void    fluid_setZRelToDef(Fluid* f, float zShift, Bool fix);
// Getters
Vector3 fluid_pos(Fluid* s);
// Downcasting
Fluid* node_asFluidOpt(Node* n);


// Internals
void    _fluid_setRelatively(Fluid* const f, const Bool fix);
// Les fonctions de base d'ouverture, fermeture, reshape de Node pour sous-classes.
void    fluid_open(Node* node);
void    fluid_close(Node* node);
void    fluid_reshape(Node* node);

#endif /* fluid_h */
