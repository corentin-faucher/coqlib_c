//
//  graph_base.inl
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2025-03-13.
//

/// Convertion d'un float en gris (et alpha = f)
static inline PixelBGRA float_toPixelBGRA(float const f) {
    return (PixelBGRA) {
        .b = f < 0.f ? 0 : (f > 1.f ? 255 : (uint8_t)(f*255.f)),
        .g = f < 0.f ? 0 : (f > 1.f ? 255 : (uint8_t)(f*255.f)),
        .r = f < 0.f ? 0 : (f > 1.f ? 255 : (uint8_t)(f*255.f)),
        .a = f < 0.f ? 0 : (f > 1.f ? 255 : (uint8_t)(f*255.f)),
    };
}
/// Conversion de Vector4 vers un pixel en BGRA uint8,
/// e.g. (1, 0.25, 0, 0.5) -> 0x7F003FFF
static inline PixelBGRA vector4_color_toPixelBGRA(Vector4 const v) {
    return (PixelBGRA) {
        .b = v.b < 0.f ? 0 : (v.b > 1.f ? 255 : (uint8_t)(v.b*255.f)),
        .g = v.g < 0.f ? 0 : (v.g > 1.f ? 255 : (uint8_t)(v.g*255.f)),
        .r = v.r < 0.f ? 0 : (v.r > 1.f ? 255 : (uint8_t)(v.r*255.f)),
        .a = v.a < 0.f ? 0 : (v.a > 1.f ? 255 : (uint8_t)(v.a*255.f)),
    };
}

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


