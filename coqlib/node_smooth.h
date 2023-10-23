//
//  node_smooth.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-16.
//

#ifndef node_smooth_h
#define node_smooth_h

#include <stdio.h>
#include "node.h"
#include "smpos.h"

typedef struct _NodeSmooth {
    Node      nd;    // Peut être casté comme un noeud.
    union {
        SmoothPos dims[4];
        // Dans le meme ordre que dans Node (en commencant a scales)
        struct {
            SmoothPos sx;
            SmoothPos sy;
            SmoothPos x;
            SmoothPos y;
        };
    };
} NodeSmooth;

extern float NodeSmooth_defaultFadeInDelta;  // 2.2 par defaut.

NodeSmooth* NodeSmooth_create(Node* const refOpt, float x, float y, float w, float h,
                              float lambda, flag_t flags, uint8_t node_place);
void        _nodesmooth_init(NodeSmooth* ns, float lambda);

void    nodesmooth_setX(NodeSmooth* const ns, float x, Bool fix);
void    nodesmooth_setY(NodeSmooth* const ns, float y, Bool fix);
void    nodesmooth_setScaleX(NodeSmooth* const ns, float sx, Bool fix);
void    nodesmooth_setScaleY(NodeSmooth* const ns, float sy, Bool fix);
void    nodesmooth_setXRelToDef(NodeSmooth* const ns, float xShift, Bool fix);
void    nodesmooth_setYRelToDef(NodeSmooth* const ns, float yShift, Bool fix);

// Les fonctions de base d'ouverture, fermeture, reshape de Node pour sous-classes.
void    node_smooth_open_default(Node* const node);
void    node_smooth_close_fadeOut(Node* const node);
void    node_smooth_reshape_relatively(Node* const node);

#endif /* node_smooth_h */
