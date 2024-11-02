//  math_base.inl
//  Created by Corentin Faucher on 2024-10-23.
//

#pragma mark-- Vector2 -------------------------------------------------
static inline float vector2_norm(Vector2 const v) {
    return sqrtf(v.x*v.x + v.y*v.y);
}
static inline float vector2_norm2(Vector2 const v) {
    return v.x*v.x + v.y*v.y;
}
static inline float vector2_distance(Vector2 const v1, Vector2 const v2) {
    return sqrtf((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));
}
static inline Vector2 vector2_minus(Vector2 const v, Vector2 const toSubtract) {
    return (Vector2) {{ v.x - toSubtract.x, v.y - toSubtract.y }};
}
static inline Vector2 vector2_add(Vector2 const v, Vector2 const toAdd) {
    return (Vector2) {{ v.x + toAdd.x, v.y + toAdd.y }};
}
static inline Vector2 vector2_mean(Vector2 const a, Vector2 const b) {
    return (Vector2) {{ 0.5*(a.x + b.x), 0.5*(a.y + b.y) }};
}
/// Produit membre à membre avec scalaire (i.e. float).
static inline Vector2 vector2_times(Vector2 const v, float const f) {
    return (Vector2) {{ v.x*f, v.y*f }};
}
static inline Vector2 vector2_opposite(Vector2 const v) {
    return (Vector2) {{ -v.x, -v.y }};
}
/// Produit vectoriel avec vec k (i.e. Rotation de 90deg dans le sens horaire).
static inline Vector2 vector2_cross(Vector2 const v) {
    return (Vector2) {{ v.y, -v.x }};
}
/// Produit scalaire.
static inline float   vector2_dot(Vector2 const v1, Vector2 const v2) {
    return v1.x*v2.x + v1.y*v2.y;
}
/// Produit membre à membre.
static inline Vector2 vector2_product(Vector2 const v1, Vector2 const v2) {
    return (Vector2) {{ v1.x * v2.x, v1.y * v2.y }};
}
static inline Vector2 vector2_polarToCartesian(Vector2 const v) {
    return (Vector2) {{ v.radial*cos(v.theta), v.radial*sin(v.theta) }};
}
static inline Vector2 vector2_cartesianToPolar(Vector2 const v) {
    return (Vector2) { .theta = atan2f(v.y, v.x), .radial = sqrtf(v.x*v.x + v.y*v.y) };
}

#pragma mark - Vecteur 3D -------------------------
static inline Vector3 vector3_minus(Vector3 const v, Vector3 const toSubtract) {
    return (Vector3) {{ v.x - toSubtract.x, v.y - toSubtract.y, v.z - toSubtract.z }};
}
static inline Vector3 vector3_add(Vector3 const v, Vector3 const toAdd) {
    return (Vector3) {{ v.x + toAdd.x, v.y + toAdd.y, v.z + toAdd.z }};
}
static inline Vector3 vector3_cross(Vector3 const v1, Vector3 const v2) {
    return (Vector3) {{
        v1.y*v2.z - v1.z*v2.y,
        v1.z*v2.x - v1.x*v2.z,
        v1.x*v2.y - v1.y*v2.x,
    }};
}
static inline float   vector3_dot(Vector3 const v1, Vector3 const v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}
static inline Vector3 vector3_product(Vector3 const v1, Vector3 const v2) {
    return (Vector3) {{ v1.x*v2.x, v1.y*v2.y, v1.z*v2.z }};
}

#pragma mark-- Extensions de rand. -------------------------------
#include <stdlib.h> // (juste pour le checker de l'ide)
/// Nombre aléatoire (distr. lin.) dans l'interval [min, max] (inclusivement).
static inline float rand_floatIn(float const min, float const max) {
    return min + (max - min) * (float)((double)rand() / (double)RAND_MAX);
}
/// Nombre aléatoire (distr. lin.) dans l'interval [mean - delta, mean + delta] (inclusivement).
static inline float rand_floatAt(float const mean, float const delta) {
    return mean - delta + 2.f*delta*(float)((double)rand() / (double)RAND_MAX);
}
/// Nombre aléatoire positif (distr. lin. discrete)
/// dans l'interval [min, max] *inclusivement*, i.e. un dé.
static inline uint32_t rand_uint(uint32_t const min, uint32_t const max) {
    return min + rand() % (max - min + 1);
}
/// Nombre aléatoire entier (distr. lin. discrete)
/// dans l'interval [min, max] inclusivement.
static inline int32_t rand_int(int32_t const min, int32_t const max) {
    return min + rand() % (max - min + 1);
}
static inline bool  rand_bool(float const p) {
    return ((double)rand() / ((double)(RAND_MAX) + 1.0)) < p;
}
static inline Vector2 rand_vector2_inBox(Box const box) {
    return (Vector2) {{
        box.c_x - box.Dx + 2.f*box.Dx*(float)((double)rand() / (double)RAND_MAX),
        box.c_y - box.Dy + 2.f*box.Dy*(float)((double)rand() / (double)RAND_MAX)
    }};
}

#pragma mark-- Extensions de uint. -----------------------------
static inline uint32_t umaxu(uint32_t const a, uint32_t const b) {
    return a > b ? a : b;
}
static inline uint32_t uminu(uint32_t const a, uint32_t const b) {
    return a < b ? a : b;
}
static inline uint32_t uint_clamp(uint32_t const u, uint32_t const min, uint32_t const max) {
    return u < min ? min : (u > max ? max : u);
}
static inline void  uint_initConst(const uint32_t* const u, uint32_t const initValue) {
    *(uint32_t*)u = initValue;
}
static inline void  size_initConst(const size_t* const s, size_t const initValue) {
    *(size_t*)s = initValue;
}

#pragma mark - Extension de int
static inline int32_t imaxi(int32_t const a, int32_t const b) {
    return a > b ? a : b;
}

#pragma mark-- Extensions de float. ------------------------------
static inline float float_toNormalizedAngle(float const f) {
    return f - ceilf((f - M_PI) / (2.f * M_PI)) * 2.f * M_PI;
}
static inline float float_smoothOut(float const x, float const lambda) {
    return lambda * (x + 1.f / lambda) * expf(-lambda*x);
}
static inline void  float_initConst(float const*const f, float const initValue) {
    *(float*)f = initValue;
}
static inline float float_clamp(float const a, float const min, float max) {
    return a < min ? min : (a > max ? max : a);
}
