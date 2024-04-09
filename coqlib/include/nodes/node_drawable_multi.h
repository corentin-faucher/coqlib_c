//
//  node_drawable_multi.h
//  AnimalCounting
//
//  Created by Corentin Faucher on 2024-03-31.
//

#ifndef node_drawable_multi_h
#define node_drawable_multi_h

#include "node_drawable.h"

typedef struct DrawableMulti {
    union {
        Node     n;
        Drawable d;
    };
    uint32_t            _maxInstanceCount;
    uint32_t            currentInstanceCount;
    
//    UniformBuffer*      _piusBuffer;
    const void*          _piusBufferCptr;
//    PerInstanceUniforms pius[1];  (array de taille maxInstanceCount)
} DrawableMulti;

DrawableMulti* DrawableMulti_create(Node* const refOpt,
                          Texture* const tex, Mesh* const mesh, uint32_t maxInstanceCount,
                          float x, float y, float twoDy,
                          flag_t flags, uint8_t node_place);
DrawableMulti*  node_asDrawableMultiOpt(Node* nd);



#endif /* node_drawable_multi_h */
