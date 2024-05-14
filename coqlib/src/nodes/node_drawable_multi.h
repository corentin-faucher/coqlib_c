//
//  node_drawable_multi.h
//  AnimalCounting
//
//  Created by Corentin Faucher on 2024-03-31.
//

#ifndef node_drawable_multi_h
#define node_drawable_multi_h

#include "node_drawable.h"
typedef struct DrawableMulti DrawableMulti;
typedef struct DrawableMulti {
    union {
        Node     n;
        Drawable d;
    };
    // Pointeur des données dans le MTLBuffer. *Éditable uniquement par `updateModels`.* (non thread-safe)
    PIUsBuffer           piusBuffer;
} DrawableMulti;

DrawableMulti* DrawableMulti_create(Node* const refOpt,
                          Texture* const tex, Mesh* const mesh, uint32_t maxInstanceCount,
                          float x, float y, float twoDy,
                          flag_t flags, uint8_t node_place);
DrawableMulti*  node_asDrawableMultiOpt(Node* nd);

Drawable*       drawablemulti_updateModelsDefault_(Node* const n);
void            drawablemulti_init_(DrawableMulti* dm, uint32_t maxInstanceCount);
void            drawablemulti_deinit_(Node* n);

#endif /* node_drawable_multi_h */
