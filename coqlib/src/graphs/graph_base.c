//
//  _graph_.c
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "graph_base.h"
#include "graph_texture.h"

const PerInstanceUniforms piu_default = PIU_DEFAULT;

uint32_t piu_getTileI(const PerInstanceUniforms* piu) {
    return (uint32_t)roundf(piu->uvRect.o_x / piu->uvRect.w);
}

Rectangle instancetile_toUVRectangle(InstanceTile const it, uint32_t m, uint32_t n) {
    m = umaxu(m, 1); n = umaxu(n, 1);
    float du0 = 1.f / (float)m;
    float dv0 = 1.f / (float)n;
    return (Rectangle) { 
        (float)( it.i % m) * du0,
        (float)((it.j + it.i / m) % n) * dv0,
        (float)it.Di * du0,
        (float)it.Dj * dv0
    };
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


