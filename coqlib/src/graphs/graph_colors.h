//
//  colors.h
//  Liste de constantes pour les couleurs utiles.
//
//  Created by Corentin Faucher on 2023-10-30.
#ifndef COQ_GRAPH_COLORS_H
#define COQ_GRAPH_COLORS_H

#include "../maths/math_base.h"
#include "../graphs/graph_base.h"

extern const Vector4 color4_black;
extern const Vector4 color4_black_back;
extern const Vector4 color4_white;
extern const Vector4 color4_white_beige;
extern const Vector4 color4_gray_dark;
extern const Vector4 color4_gray_dark2;
extern const Vector4 color4_gray;
extern const Vector4 color4_gray_light;
extern const Vector4 color4_gray2;
extern const Vector4 color4_gray3;
extern const Vector4 color4_red;
extern const Vector4 color4_red_vermilion;
extern const Vector4 color4_red_coquelicot;
extern const Vector4 color4_red_orange2;
extern const Vector4 color4_red_coral;
extern const Vector4 color4_red_dark;
extern const Vector4 color4_orange;
extern const Vector4 color4_orange_amber;
extern const Vector4 color4_orange_bronze;
extern const Vector4 color4_orange_saffron;
extern const Vector4 color4_orange_saffron2;
extern const Vector4 color4_yellow_cadmium;
extern const Vector4 color4_yellow_amber;
extern const Vector4 color4_yellow_citrine;
extern const Vector4 color4_yellow_lemon;
extern const Vector4 color4_green_electric;
extern const Vector4 color4_green_electric2;
extern const Vector4 color4_green_fluo;
extern const Vector4 color4_green_ao;
extern const Vector4 color4_green_spring;
extern const Vector4 color4_green_avocado;
extern const Vector4 color4_green_dark_cyan;
extern const Vector4 color4_aqua;
extern const Vector4 color4_blue;
extern const Vector4 color4_blue_sky;
extern const Vector4 color4_blue_sky2;
extern const Vector4 color4_blue_pale;
extern const Vector4 color4_blue_azure;
extern const Vector4 color4_blue_strong;
extern const Vector4 color4_purple;
extern const Vector4 color4_purble_china_pink;
extern const Vector4 color4_purble_electric_indigo;
extern const Vector4 color4_purble_blue_violet;

/// Transforme la couleur vers le gris de niveau `level`.
/// alpha : ratio de `grisification`.
/// e.g. si level = 0.5, alpha = 0 -> pas de changement.
/// level = 0.5, alpha = 1 -> completement gris (0.5, 0.5, 0.5).
static inline Vector4   vector4_color_toGray(Vector4 const v, float const level, float const alpha) {
    return (Vector4) {{
        (1.f - alpha)*v.r + level*alpha,
        (1.f - alpha)*v.g + level*alpha,
        (1.f - alpha)*v.b + level*alpha,
        v.a,        
    }};
}
/// Conversion de vecteur 4 vers un pixel en BGRA uint8,
/// e.g. (1, 0.25, 0, 0.5) -> 0x7F003FFF
static inline PixelBGRA vector4_color_toPixelBGRA(Vector4 const v) {
    return (PixelBGRA) {
        .b = (uint8_t)(fminf(fmaxf(v.b, 0.f), 1.f)*255.f),
        .g = (uint8_t)(fminf(fmaxf(v.g, 0.f), 1.f)*255.f), 
        .r = (uint8_t)(fminf(fmaxf(v.r, 0.f), 1.f)*255.f), 
        .a = (uint8_t)(fminf(fmaxf(v.a, 0.f), 1.f)*255.f),
    };
}
#endif /* colors_h */
