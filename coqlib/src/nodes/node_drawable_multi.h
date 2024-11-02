//
//  node_drawable_multi.h
//  AnimalCounting
//
//  Created by Corentin Faucher on 2024-03-31.
//

#ifndef node_drawable_multi_h
#define node_drawable_multi_h

#include "node_drawable.h"
#pragma mark - DrawableMulti Un noeud avec multi-instance à dessiner, e.g. des particules.
typedef struct DrawableMulti {
    union {
        Node     n;
        Drawable d;
    };
    // Pointeur des données dans le MTLBuffer. *Éditable uniquement par `updateModels`.* (non thread-safe)
    IUsBuffer           iusBuffer;
} DrawableMulti;

DrawableMulti* node_asDrawableMultiOpt(Node* nd);

/// Ne fait que setter le IUsBuffer et le deinit. Les Instance uniforms et le `renderer_updateInstanceUniforms` doivent
/// être implémenté en fonction du DrawableMulti.
void           drawablemulti_init(DrawableMulti* dm, uint32_t maxInstanceCount);
void           drawablemulti_deinit(Node* n);

#endif /* node_drawable_multi_h */
