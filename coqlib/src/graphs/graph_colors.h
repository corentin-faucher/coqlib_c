//
//  colors.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//

#ifndef COQ_GRAPH_COLORS_H
#define COQ_GRAPH_COLORS_H

#include "../maths/math_base.h"

/// Transforme la couleur vers le gris de niveau `level`.
/// alpha ration de grisification.
/// e.g. si level = 0.5, alpha = 0 -> pas de changement.
/// level = 0.5, alpha = 1 -> completement gris (0.5, 0.5, 0.5).
Vector4 vector4_color_toGray(Vector4 v, float level, float alpha);
// Superflu ?
//Vector4 vector4_color_toDark(float intensity);
//Vector4 vector4_color_toLight(float intensity);

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

#endif /* colors_h */
