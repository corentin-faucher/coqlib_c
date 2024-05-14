//
//  _graph_.c
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "graph_base.h"

const PerInstanceUniforms piu_default = PIU_DEFAULT;

uint32_t piu_getTileI(const PerInstanceUniforms* piu) {
    return (uint32_t)roundf(piu->u0 / piu->Du);
}

//const PerTextureUniforms ptu_default = PTU_DEFAULT;

PerFrameUniforms pfu_default = {
    {{
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    }},
    0.f, 0.f, 0.f, 0.f
};


