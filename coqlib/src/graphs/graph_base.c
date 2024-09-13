//
//  _graph_.c
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "graph_base.h"
#include "graph_texture.h"

//#define PIU_DEFAULT \
//{{{ 1.f, 0.f, 0.f, 0.f, \
//   0.f, 1.f, 0.f, 0.f, \
//   0.f, 0.f, 1.f, 0.f, \
//   0.f, 0.f, 0.f, 1.f }}, \
// {{ 1.f, 1.f, 1.f, 1.f }}, \
// {{ 0.f, 0.f, 1.f, 1.f }}, \
//   0u, 1.f, 0.f, 0.f }
const InstanceUniforms InstanceUnifoms_default = {
 {{ 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f, 
    0.f, 0.f, 1.f, 0.f, 
    0.f, 0.f, 0.f, 1.f }}, 
 {{ 1.f, 1.f, 1.f, 1.f }}, 
 {{ 0.f, 0.f, 1.f, 1.f }}, 
    0u, 1.f, 0.f, 0.f 
};

uint32_t piu_getTileI(const InstanceUniforms* piu) {
    return (uint32_t)roundf(piu->uvRect.o_x / piu->uvRect.w);
}

void   piusbuffer_setAllTo(IUsBuffer* const iusBuffer, InstanceUniforms const iu) {
    InstanceUniforms* end = &iusBuffer->ius[iusBuffer->max_count];
    for(InstanceUniforms* piu = iusBuffer->ius; piu < end; piu++) {
        *piu = iu;
    }
}
void   piusbuffer_setAllActiveTo(IUsBuffer* const iusBuffer, InstanceUniforms const iu) {
    InstanceUniforms* end = &iusBuffer->ius[iusBuffer->actual_count];
    for(InstanceUniforms* piu = iusBuffer->ius; piu < end; piu++) {
        *piu = iu;
    }
}

//Rectangle instancetile_toUVRectangle(InstanceTile const it, uint32_t m, uint32_t n) {
//    m = umaxu(m, 1); n = umaxu(n, 1);
//    float du0 = 1.f / (float)m;
//    float dv0 = 1.f / (float)n;
//    return (Rectangle) { 
//        (float)( it.i % m) * du0,
//        (float)((it.j + it.i / m) % n) * dv0,
//        (float)it.Di * du0,
//        (float)it.Dj * dv0
//    };
//}


const PixelBGRA pixelbgra_black = { 0xFF000000 };
const PixelBGRA pixelbgra_white = { 0xFFFFFFFF };
const PixelBGRA pixelbgra_red =   { 0xFFFF0000 };
const PixelBGRA pixelbgra_yellow = { 0xFFFFFF00 };

//const PerTextureUniforms ptu_default = PTU_DEFAULT;



