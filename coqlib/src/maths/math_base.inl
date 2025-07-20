//  math_base.inl
//  Created by Corentin Faucher on 2024-10-23.
//

// MARK: - Vector2
static inline float vector2_norm(Vector2 const v) {
    return hypotf(v.x, v.y); // sqrtf(v.x*v.x + v.y*v.y);
}
static inline float vector2_norm2(Vector2 const v) {
    return v.x*v.x + v.y*v.y;
}
static inline float vector2_distance(Vector2 const v1, Vector2 const v2) {
    return hypotf(v1.x - v2.x, v1.y - v2.y);
    // sqrtf((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));
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
    return (Vector2) { 
        .theta = atan2f(v.y, v.x), 
        .radial = hypotf(v.x, v.y) 
    };
}


static inline RectangleInt rectangle_roundedToRectangleInt(Rectangle const r) {
    return (RectangleInt) {{ 
        (int32_t)roundf(r.o_x),       (int32_t)roundf(r.o_y),
        (uint32_t)fabsf(roundf(r.w)), (uint32_t)fabsf(roundf(r.h)) 
    }};
}

static inline bool vector2_isInBox(Vector2 const pos, Box const hitBox) {
    return fabsf(hitBox.c_x - pos.x) <= hitBox.Dx &&
           fabsf(hitBox.c_y - pos.y) <= hitBox.Dy;
}
/// On place la position `xy` dans le référentiel `referential`.
static inline Vector2 vector2_referentialIn(Vector2 const xy, Box const referential) {
    return (Vector2){{
        (xy.x - referential.c_x) / referential.Dx, 
        (xy.y - referential.c_y) / referential.Dy,
    }};
}
/// Resort du référentiel de la boîte.
static inline Vector2 vector2_referentialOut(Vector2 const xy, Box const referential) {
    return (Vector2) {{
        xy.x*referential.Dx + referential.c_x,
        xy.y*referential.Dy + referential.c_y,
    }};
}
/// On place `b` dans le référentiel `referential`.
static inline Box box_referentialIn(Box b, Box referential) {
    return (Box) {
        .center = {{
            (b.c_x - referential.c_x) / referential.Dx, 
            (b.c_y - referential.c_y) / referential.Dy,
        }},
        .deltas = {{ 
            b.Dx / referential.Dx,
            b.Dy / referential.Dy,
        }},
    };
}
/// Resort `b` du `referential` (inverse de boxIn).
static inline Box box_referentialOut(Box b, Box referential) {
    return (Box) {
        .center = {{
            b.c_x * referential.Dx + referential.c_x, 
            b.c_y * referential.Dy + referential.c_y,
        }},
        .deltas = {{ 
            b.Dx * referential.Dx,
            b.Dy * referential.Dy,
        }},
    };
}

// MARK: - Vecteur 3D
static inline float   vector3_norm(Vector3 v) {
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}
static inline Vector3 vector3_minus(Vector3 const v, Vector3 const toSubtract) {
    return (Vector3) {{ v.x - toSubtract.x, v.y - toSubtract.y, v.z - toSubtract.z }};
}
static inline Vector3 vector3_add(Vector3 const v, Vector3 const toAdd) {
    return (Vector3) {{ v.x + toAdd.x, v.y + toAdd.y, v.z + toAdd.z }};
}
static inline Vector3 vector3_times(Vector3 const v, float const f) {
    return (Vector3) {{ v.x * f, v.y * f, v.z * f }};
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
static inline Vector3 vector3_alphaBlend(Vector3 const v0, Vector3 const v1, float const alpha) {
    return (Vector3) {{ 
        (1.f - alpha)*v0.x + alpha*v1.x,
        (1.f - alpha)*v0.y + alpha*v1.y,
        (1.f - alpha)*v0.z + alpha*v1.z,
    }};
}
static inline Vector3 vector3_max(Vector3 const v1, Vector3 const v2) {
    return (Vector3) {{ 
        v1.x > v2.x ? v1.x : v2.x,
        v1.y > v2.y ? v1.y : v2.y,
        v1.z > v2.z ? v1.z : v2.z,
    }};
}
static inline Vector3 vector3_values(float const f) {
    return (Vector3) {{ f, f, f, }};
}

// MARK: - Vector4
static inline Vector4 vector4_max(Vector4 const v1, Vector4 const v2) {
    return (Vector4) {{ 
        v1.x > v2.x ? v1.x : v2.x,
        v1.y > v2.y ? v1.y : v2.y,
        v1.z > v2.z ? v1.z : v2.z,
        v1.w > v2.w ? v1.w : v2.w,
    }};
}
static inline Vector4 vector4_min(Vector4 const v1, Vector4 const v2) {
    return (Vector4) {{ 
        v1.x < v2.x ? v1.x : v2.x,
        v1.y < v2.y ? v1.y : v2.y,
        v1.z < v2.z ? v1.z : v2.z,
        v1.w < v2.w ? v1.w : v2.w,
    }};
}
static inline Vector4 vector4_cat(Vector3 const v, float const e3) {
    return (Vector4) {{
        v.x, v.y, v.z, e3,
    }};
}

// MARK: - Matrices 4x4
static inline void matrix4_initAsPerspective(Matrix4 *const m, float const theta, float const ratio,
                               float const nearZ, float const farZ) {
    // Bonne ref pour la théorie : http://www.songho.ca/opengl/gl_projectionmatrix.html
    *m = ({
        float cotan = 1.f / tanf(theta / 2.f);
        (Matrix4) {{
            cotan / ratio, 0, 0, 0,
            0,         cotan, 0, 0,
            0, 0, (farZ + nearZ) / (nearZ - farZ),    -1.f,
            0, 0, (2 * farZ * nearZ) / (nearZ - farZ), 0.f,
        }};
    });
}
static inline void matrix4_initAsPerspectiveDeltas(Matrix4 *const m, float const nearZ, float const farZ, float const middleZ,
                                     float const deltaX, float const deltaY) {
    // Ici, on utilise l'astuce du w = -1 dans v2 pour faire un 1/z...
    // z sont négatif car on regarde vers les z négatifs...
    // Bonne ref pour la théorie : http://www.songho.ca/opengl/gl_projectionmatrix.html
    // On peut customizer cette projection, par exemple, si on veut, on n'est pas obligé de tronquer en z, 
    // i.e. enlever les composantes de la 3e ligne (ne pas oublier que les "lignes" sont les "colonnes" lorsqu'on écrit la matrice...)
    *m = (Matrix4) {{
        middleZ / deltaX, 0.0,     0.0,           0.0, // première "colonne"
        0.0, middleZ/deltaY,       0.0,           0.0, // deuxième "colonne"...
        0.0, 0.0, (farZ + nearZ)/(nearZ - farZ), -1.0,
        0.0, 0.0, (2*farZ*nearZ)/(nearZ - farZ),  0.0,
    }};
}
static inline void matrix4_scale(Matrix4 *const m, Vector3 const s){
    m->v0.v *= s.x; m->v1.v *= s.y; m->v2.v *= s.z;
}
static inline void matrix4_translate(Matrix4 *const m, Vector3 const t) {
    m->v3.v += m->v0.v * t.x + m->v1.v * t.y + m->v2.v * t.z;
}
static inline Matrix4 matrix4_product(const Matrix4*const a, const Matrix4*const b) {
    return (Matrix4) { .v_arr = {
        a->v0.v * b->v0.x + a->v1.v * b->v0.y + a->v2.v * b->v0.z + a->v3.v * b->v0.w,
        a->v0.v * b->v1.x + a->v1.v * b->v1.y + a->v2.v * b->v1.z + a->v3.v * b->v1.w,
        a->v0.v * b->v2.x + a->v1.v * b->v2.y + a->v2.v * b->v2.z + a->v3.v * b->v2.w,
        a->v0.v * b->v3.x + a->v1.v * b->v3.y + a->v2.v * b->v3.z + a->v3.v * b->v3.w,
    }};
}

// MARK: - Extensions de rand.
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

// MARK: - Extension de bool
static inline char const* bool_toString(bool const b) {
    return b ? "true" : "false";
}
static inline void bool_initConst(const bool*const b, bool const initValue) {
    *(bool*)b = initValue;
}

// MARK: - Extensions de uint.
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
static inline void  voidptr_initConst(const void**const ptr, void*const init_ptr) {
    *(void**)ptr = init_ptr;
}

// MARK: - Extension de int
static inline int32_t imini(int32_t const a, int32_t const b) {
    return a > b ? b : a;
}
static inline int32_t imaxi(int32_t const a, int32_t const b) {
    return a > b ? a : b;
}
static inline int32_t int_clamp(int32_t const u, int32_t const min, int32_t const max) {
    return u < min ? min : (u > max ? max : u);
}

// MARK: - Extensions de float.
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
static inline float float_truncated(float const f, float const delta) {
    return (f > 0.f) ? fmaxf(0.f, f - delta) : fminf(0.f, f + delta);
}
static inline float float_appearing(float const x, float const x0, float const deltaX) {
    return (x < x0) ? 0.f : ((x > x0 + deltaX) ? 1.f : (x - x0)/deltaX );
}
static inline float float_alphaBlend(float const f0, float const f1, float const alpha) {
    return (1.f - alpha) * f0 + alpha * f1;
}
