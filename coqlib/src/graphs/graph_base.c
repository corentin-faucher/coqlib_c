//
//  _graph_.c
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "graph_base.h"

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
const InstanceUniforms InstanceUniforms_default = {
    .model = {{ 1.f, 0.f, 0.f, 0.f,
                0.f, 1.f, 0.f, 0.f,
                0.f, 0.f, 1.f, 0.f,
                0.f, 0.f, 0.f, 1.f }},
    .color = {{ 1.f, 1.f, 1.f, 1.f }},
    .uvRect = {{ 0.f, 0.f, 1.f, 1.f }},
};
const Rectangle InstanceUniforms_defaultUVRect = {{ 0.f, 0.f, 1.f, 1.f }};

//uint32_t piu_getTileI(const InstanceUniforms* piu) {
//    return (uint32_t)roundf(piu->uvRect.o_x / piu->uvRect.w);
//}



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

// MARK: - Couleurs en pixels BGRA
const PixelBGRA pixelbgra_black =  { 0xFF000000 };
const PixelBGRA pixelbgra_white =  { 0xFFFFFFFF };
const PixelBGRA pixelbgra_red =    { 0xFFFF0000 };
const PixelBGRA pixelbgra_yellow = { 0xFFFFFF00 };

// MARK: - Couleurs en Vector4
const Vector4 color4_black = {{0, 0, 0, 1 }};
const Vector4 color4_black_back = {{0.1, 0.1, 0.05, 1 }};
const Vector4 color4_white = {{1, 1, 1, 1 }};
const Vector4 color4_white_beige = {{0.95, 0.92, 0.85, 1 }};
const Vector4 color4_gray_25 = {{0.25, 0.25, 0.25, 1 }};
const Vector4 color4_gray_40 = {{0.40, 0.40, 0.40, 1 }};
const Vector4 color4_gray_60 = {{0.60, 0.60, 0.60, 1 }};
const Vector4 color4_gray_80 = {{0.80, 0.80, 0.80, 1 }};
const Vector4 color4_red = {{1, 0, 0, 1 }};
const Vector4 color4_red_vermilion = {{ 1, 0.3, 0.1, 1 }};
const Vector4 color4_red_coquelicot = {{ 1, 0.2, 0, 1 }};
const Vector4 color4_red_orange2 = {{ 1, 0.4, 0.4, 1 }};
const Vector4 color4_red_coral = {{ 1, 0.5, 0.3, 1 }};
const Vector4 color4_red_dark = {{ 0.2, 0.1, 0.1, 1 }};
const Vector4 color4_orange = {{ 1, 0.6, 0, 1 }};
const Vector4 color4_orange_amber = {{ 1, 0.5, 0, 1 }};
const Vector4 color4_orange_bronze = {{ 0.8, 0.5, 0.2, 1 }};
const Vector4 color4_orange_saffron = {{ 1.0, 0.6, 0.2, 1 }};
const Vector4 color4_orange_saffron2 = {{ 1.0, 0.7, 0.4, 1 }};
const Vector4 color4_yellow_cadmium = {{ 1, 1, 0, 1 }};
const Vector4 color4_yellow_amber = {{ 1, 0.75, 0, 1 }};
const Vector4 color4_yellow_citrine = {{ 0.90, 0.82, 0.04, 1 }};
const Vector4 color4_yellow_lemon = {{ 1, 0.95, 0.05, 1 }};
const Vector4 color4_green_electric = {{ 0, 1, 0, 1 }};
const Vector4 color4_green_electric2 = {{ 0.25, 1, 0.25, 1 }};
const Vector4 color4_green_fluo = {{ 0.5, 1, 0.5, 1 }};
const Vector4 color4_green_ao = {{ 0.0, 0.55, 0.0, 1 }};
const Vector4 color4_green_spring = {{ 0.2, 1, 0.5, 1 }};
const Vector4 color4_green_avocado = {{ 0.34, 0.51, 0.01, 1 }};
const Vector4 color4_green_dark_cyan = {{ 0.0, 0.55, 0.55, 1 }};
const Vector4 color4_aqua = {{ 0, 1, 1, 1 }};
const Vector4 color4_blue = {{ 0, 0.25, 1, 1 }};
const Vector4 color4_blue_sky = {{ 0.40, 0.70, 1, 1 }};
const Vector4 color4_blue_sky2 = {{ 0.55, 0.77, 1, 1 }};
const Vector4 color4_blue_pale = {{ 0.8, 0.9, 1, 1 }};
const Vector4 color4_blue_azure = {{ 0.00, 0.50, 1, 1 }};
const Vector4 color4_blue_strong = {{ 0, 0, 1, 1 }};
const Vector4 color4_purple = {{ 0.8, 0, 0.8, 1 }};
const Vector4 color4_purble_china_pink = {{ 0.87, 0.44, 0.63, 1 }};
const Vector4 color4_purble_electric_indigo = {{ 0.44, 0.00, 1, 1 }};
const Vector4 color4_purble_blue_violet = {{ 0.54, 0.17, 0.89, 1 }};


//const PerTextureUniforms ptu_default = PTU_DEFAULT;
