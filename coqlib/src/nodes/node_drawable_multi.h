//
//  node_drawable_multi.h
//  AnimalCounting
//
//  Created by Corentin Faucher on 2024-03-31.
//

#ifndef node_drawable_multi_h
#define node_drawable_multi_h

#include "node_drawable.h"
#include "../graphs/graph_iusbuffer.h"
// MARK: - DrawableMulti Un noeud affichable avec multi-instance, e.g. des particules.
typedef struct DrawableMulti {
    union {
        Node     n;
        Drawable d;
    };
    IUsBuffer*   iusBuffer;
} DrawableMulti;

static inline DrawableMulti* node_asDrawableMultiOpt(Node* nOpt) {
    return (nOpt && (nOpt->_type & node_type_drawMulti)) ? (DrawableMulti*)nOpt : NULL;
}

/// Ne fait que setter le IUsBuffer et le deinit. 
/// -> le `renderer_updateInstanceUniforms` doit
/// être implémenté en fonction du DrawableMulti.
void           drawablemulti_init(DrawableMulti* dm, uint32_t maxInstanceCount, InstanceUniforms const* defaultIUOpt);
void           drawablemulti_deinit(Node* n);

#endif /* node_drawable_multi_h */
