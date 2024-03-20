//
//  colors.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//

#include "graphs/graph_colors.h"

Vector4 vector4_color_toGray(Vector4 v, float level, float alpha) {
    return (Vector4) {
        (1.f - alpha)*v.r + level*alpha,
        (1.f - alpha)*v.g + level*alpha,
        (1.f - alpha)*v.b + level*alpha,
        v.a,        
    };
}

const Vector4 color4_black = {{0, 0, 0, 1 }};
const Vector4 color4_black_back = {{0.1, 0.1, 0.05, 1 }};
const Vector4 color4_white = {{1, 1, 1, 1 }};
const Vector4 color4_white_beige = {{0.95, 0.92, 0.85, 1 }};
const Vector4 color4_gray_dark = {{0.40, 0.40, 0.40, 1 }};
const Vector4 color4_gray_dark2 = {{0.25, 0.25, 0.25, 0.7 }};
const Vector4 color4_gray = {{0.6, 0.6, 0.6, 1 }};
const Vector4 color4_gray_light = {{0.80, 0.80, 0.80, 1 }};
const Vector4 color4_gray2 = {{0.75, 0.75, 0.75, 0.9 }};
const Vector4 color4_gray3 = {{0.90, 0.90, 0.90, 0.70 }};
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
