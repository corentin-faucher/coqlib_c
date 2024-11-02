//
//  _graph_.c
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "graph_base.h"

#include "graph_texture.h"
#include "graph_mesh.h"
#include "../utils/util_base.h"
#include "../utils/util_file.h"

char*  FileManager_getPngPathOpt(const char* pngName, bool isCoqlib, bool isMini) {
    if(!pngName) { printerror("Texture without name."); return NULL; }
    const char* png_dir;
    if(isCoqlib) {
        png_dir = isMini ? "pngs_coqlib/minis" : "pngs_coqlib";
    } else {
        png_dir = isMini ? "pngs/minis" : "pngs";
    }
    // Pas d'erreur si mini.
    return FileManager_getResourcePathOpt(pngName, "png", png_dir);
}

//#define PIU_DEFAULT \
//{{{ 1.f, 0.f, 0.f, 0.f, \
//   0.f, 1.f, 0.f, 0.f, \
//   0.f, 0.f, 1.f, 0.f, \
//   0.f, 0.f, 0.f, 1.f }}, \
// {{ 1.f, 1.f, 1.f, 1.f }}, \
// {{ 0.f, 0.f, 1.f, 1.f }}, \
//   0u, 1.f, 0.f, 0.f }
const InstanceUniforms InstanceUnifoms_drawableDefaultIU = {
    .model = {{ 1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f, 
        0.f, 0.f, 1.f, 0.f, 
        0.f, 0.f, 0.f, 1.f }}, 
    .show = 1.f,
    .draw_color = {{ 1.f, 1.f, 1.f, 1.f }},
    .draw_uvRect = {{ 0.f, 0.f, 1.f, 1.f }}, 
};

//uint32_t piu_getTileI(const InstanceUniforms* piu) {
//    return (uint32_t)roundf(piu->uvRect.o_x / piu->uvRect.w);
//}

void   iusbuffer_setAllTo(IUsBuffer* const iusBuffer, InstanceUniforms const iuRef) {
    InstanceUniforms* end = &iusBuffer->ius[iusBuffer->max_count];
    for(InstanceUniforms* iu = iusBuffer->ius; iu < end; iu++) {
        *iu = iuRef;
    }
}
void   iusbuffer_setAllActiveTo(IUsBuffer* const iusBuffer, InstanceUniforms const iu) {
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

float coqfont_getRelY(coq_Font const*const cf) {
    return cf->deltaY / cf->solidHeight;
}
float coqfont_getRelHeight(coq_Font const*const cf) {
    return cf->glyphHeight / cf->solidHeight;
}

const PixelBGRA pixelbgra_black = { 0xFF000000 };
const PixelBGRA pixelbgra_white = { 0xFFFFFFFF };
const PixelBGRA pixelbgra_red =   { 0xFFFF0000 };
const PixelBGRA pixelbgra_yellow = { 0xFFFFFF00 };

//const PerTextureUniforms ptu_default = PTU_DEFAULT;



