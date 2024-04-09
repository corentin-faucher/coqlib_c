//
//  maths.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "math_base.h"

#include "../utils/utils_base.h"

const Vector2 vector2_ones =  {{1, 1}};
const Vector2 vector2_zeros = {{0, 0}};

float   vector2_norm(Vector2 v) {
    return sqrtf(v.x*v.x + v.y*v.y);
}
float   vector2_norm2(Vector2 v) {
    return v.x*v.x + v.y*v.y;
}
float   vector2_dot(Vector2 v1, Vector2 v2) {
    return v1.x*v2.x + v1.y*v2.y;
}
Vector2 vector2_projOn(Vector2 v, Vector2 target) {
    float dot = v.x*target.x + v.y*target.y;
    float t2 = target.x*target.x + target.y*target.y;
    return (Vector2) {{
        dot * target.x / t2,
        dot * target.y / t2
    }};
}
float   vector2_distance(Vector2 v1, Vector2 v2) {
    return sqrtf((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));
}
Vector2 vector2_normalize(Vector2 v) {
    float n = sqrtf(v.x*v.x + v.y*v.y);
    return (Vector2) {{ v.x / n, v.y / n }};
}
Vector2 vector2_minus(Vector2 v, Vector2 toSubtract) {
    return (Vector2) {{ v.x - toSubtract.x, v.y - toSubtract.y }};
}
Vector2 vector2_add(Vector2 v, Vector2 toAdd) {
    return (Vector2) {{ v.x + toAdd.x, v.y + toAdd.y }};
}
Vector2 vector2_mean(Vector2 a, Vector2 b) {
    return (Vector2) {{ 0.5*(a.x + b.x), 0.5*(a.y + b.y) }};
}
Vector2 vector2_times(Vector2 v, float f) {
    return (Vector2) {{ v.x*f, v.y*f }};
}
Vector2 vector2_opposite(Vector2 v) {
    return (Vector2) {{ -v.x, -v.y }};
}
/// Produit vectoriel avec vec k (i.e. Rotation de 90deg dans le sens horaire).
Vector2 vector2_cross(Vector2 v) {
    return (Vector2) {{ v.y, -v.x }};
}

Vector3 vector3_normalize(Vector3 v) {
    float n = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    return (Vector3) {{ v.x / n, v.y / n, v.z / n }};
}
Vector3 vector3_minus(Vector3 v, Vector3 toSubtract) {
    return (Vector3) {{ v.x - toSubtract.x, v.y - toSubtract.y, v.z - toSubtract.z }};
}
Vector3 vector3_add(Vector3 v, Vector3 toAdd) {
    return (Vector3) {{ v.x + toAdd.x, v.y + toAdd.y, v.z + toAdd.z }};
}
Vector3 vector3_cross(Vector3 v1, Vector3 v2) {
    return (Vector3) {{
        v1.y*v2.z - v1.z*v2.y,
        v1.z*v2.x - v1.x*v2.z,
        v1.x*v2.y - v1.y*v2.x,
    }};
}
float    vector3_dot(Vector3 v1, Vector3 v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}


const Vector3 vector3_ones =  {{1, 1, 1}};
const Vector3 vector3_zeros = {{0, 0, 0}};

void vector3_print(Vector3 *v) {
    printf("[ %f, %f, %f ]\n", v->x, v->y, v->z);
}

const Matrix4 matrix4_identity = {{
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.,
}};

void matrix4_print(Matrix4 *m) {
    for(int j = 0; j < 4; j++) {
        if(j == 0) printf("[ "); else printf("  ");
        for(int i = 0; i< 4; i++)
            printf("%5.2f, ", m->f_arr[i + j*4]);
        if(j == 3) printf("\b ]\n"); else printf("\n");
    }
}

void matrix4_initWithAndTranslate(Matrix4 *m, const Matrix4* const ref, Vector3 t) {
    m->v0 = ref->v0; m->v1 = ref->v1; m->v2 = ref->v2;
    m->v3 = (Vector4) {{
        ref->v3.x + ref->v0.x * t.x + ref->v1.x * t.y + ref->v2.x * t.z,
        ref->v3.y + ref->v0.y * t.x + ref->v1.y * t.y + ref->v2.y * t.z,
        ref->v3.z + ref->v0.z * t.x + ref->v1.z * t.y + ref->v2.z * t.z,
        ref->v3.w,
    }};
}


void matrix4_initWithRotateYAndTranslateYZ(Matrix4 *m, Matrix4 *ref,
                                           float thetaY, float ty, float tz) {
    float c = cosf(thetaY);
    float s = sinf(thetaY);
    m->v0.v = c*ref->v0.v - s*ref->v2.v;
    m->v1 =   ref->v1;
    m->v2.v = s*ref->v0.v + c*ref->v2.v;
    m->v3 = (Vector4){{
        ref->v3.x + ref->v1.x * ty + m->v2.x * tz,
        ref->v3.y + ref->v1.y * ty + m->v2.y * tz,
        ref->v3.z + ref->v1.z * ty + m->v2.z * tz,
        ref->v3.w,
    }};
}



void matrix4_initAsLookAt(Matrix4 *m, Vector3 eye, Vector3 center, Vector3 up) {
    
    Vector3 n = vector3_normalize(vector3_minus(eye, center));
    Vector3 u = vector3_normalize(vector3_cross(up, n));
    Vector3 v = vector3_cross(n, u);
    *m = (Matrix4) {{
        u.x, v.x, n.x, 0.f,
        u.y, v.y, n.y, 0.f,
        u.z, v.z, n.z, 0.f,
        -vector3_dot(u, eye), -vector3_dot(v, eye), -vector3_dot(n, eye), 1.f,
    }};
}
void matrix4_initAsPerspective(Matrix4 *m, float theta, float ratio,
                               float nearZ, float farZ) {
    float cotan = 1.f / tanf(theta / 2.f);
    *m = (Matrix4) {{
        cotan / ratio, 0, 0, 0,
        0, cotan, 0, 0,
        0, 0, (farZ + nearZ) / (nearZ - farZ), -1.f,
        0, 0, (2 * farZ * nearZ) / (nearZ - farZ), 0.f,
    }};
}
void matrix4_initAsPerspectiveDeltas(Matrix4 *m, float nearZ, float farZ, float middleZ,
                                     float deltaX, float deltaY) {
    *m = (Matrix4) {{
        2.f * middleZ / deltaX, 0, 0, 0,
        0, 2.f * middleZ / deltaY, 0, 0,
        0, 0, (farZ + nearZ) / (nearZ - farZ), -1.f,
        0, 0, (2 * farZ * nearZ) / (nearZ - farZ), 0.f,
    }};
}
void matrix4_scale(Matrix4 *m, float sx, float sy, float sz) {
    m->v0.v *= sx;
    m->v1.v *= sy;
    m->v2.v *= sz;
    // Revient au meme ?
//    m->v0.x *= s.x; m->v0.y *= s.x; m->v0.z *= s.x; m->v0.w *= s.x;
//    m->v1.x *= s.y; m->v1.y *= s.y; m->v1.z *= s.y; m->v1.w *= s.y;
//    m->v2.x *= s.z; m->v2.y *= s.z; m->v2.z *= s.z; m->v2.w *= s.z;
}
void matrix4_translate(Matrix4 *m, Vector3 t) {
    m->v3.x += m->v0.x * t.x + m->v1.x * t.y + m->v2.x * t.z;
    m->v3.y += m->v0.y * t.x + m->v1.y * t.y + m->v2.y * t.z;
    m->v3.z += m->v0.z * t.x + m->v1.z * t.y + m->v2.z * t.z;
}
void matrix4_rotateX(Matrix4 *m, float theta) {
    float c = cosf(theta);
    float s = sinf(theta);
    Vector4 v1 = m->v1;
    Vector4 v2 = m->v2;
    m->v1.v = c*v1.v + s*v2.v;
    m->v2.v = c*v2.v - s*v1.v;
}
void matrix4_rotateY(Matrix4 *m, float theta) {
    float c = cosf(theta);
    float s = sinf(theta);
    Vector4 v0 = m->v0;
    Vector4 v2 = m->v2;
    m->v0.v = c*v0.v - s*v2.v;
    m->v2.v = s*v0.v + c*v2.v;
}
void matrix4_rotateZ(Matrix4 *m, float theta) {
    float c = cosf(theta);
    float s = sinf(theta);
    Vector4 v0 = m->v0;
    Vector4 v1 = m->v1;
    m->v0.v = c*v0.v - s*v1.v;
    m->v1.v = s*v0.v + c*v1.v;
}
void matrix4_rotateYandTranslateYZ(Matrix4 *m, float thetaY, float ty, float tz) {
    float c = cosf(thetaY);
    float s = sinf(thetaY);
    Vector4 v0 = m->v0;
    Vector4 v2 = m->v2;
    m->v0.v = c*v0.v - s*v2.v; // Ou au long x,y,z ? et pas w ?
    m->v2.v = s*v0.v + c*v2.v;
    m->v3.x += m->v1.x * ty + m->v2.x * tz;
    m->v3.y += m->v1.y * ty + m->v2.y * tz;
    m->v3.z += m->v1.z * ty + m->v2.z * tz;
}

/// Nombre aléatoire (distr. lin.) dans l'interval [min, max] (inclusivement).
float rand_floatIn(float min, float max) {
    return min + (max - min) * (float)((double)rand() / (double)RAND_MAX);
}
/// Nombre aléatoire (distr. lin.) dans l'interval [mean - delta, mean + delta] (inclusivement).
float rand_floatAt(float mean, float delta) {
    return mean - delta + 2.f*delta*(float)((double)rand() / (double)RAND_MAX);
}
/// Nombre aléatoire positif (distr. lin. discrete)
/// dans l'interval [min, max] *inclusivement*, i.e. un dé.
uint32_t rand_uint(uint32_t min, uint32_t max) {
    return min + rand() % (max - min + 1);
}
/// Nombre aléatoire entier (distr. lin. discrete)
/// dans l'interval [min, max] inclusivement.
int32_t rand_int(int32_t min, int32_t max) {
    return min + rand() % (max - min + 1);
}
void  rand_uintarr_shuffle(uint32_t* const u_arr, uint32_t count) {
    uint32_t* p = u_arr;
    uint32_t* const end = &u_arr[count];
    uint32_t* other;
    while(p < end) {
        other = &u_arr[rand() % count];
        uint32_t tmp = *p;
        *p = *other;
        *other = tmp;
        p++;
    }
}
bool  rand_bool(float p) {
    return ((double)rand() / ((double)(RAND_MAX) + 1.0)) < p;
}
int64_t rand_float_toInt(float f) {
    int64_t i = (int64_t)floorf(f);
    return i + (int64_t)rand_bool(f - (float)i);
}

Vector2 rand_vector2_ofNorm(float norm) {
    float theta = 2.f * M_PI * (float)((double)rand() / (double)RAND_MAX);
    return (Vector2) {{ norm*cosf(theta), norm*sinf(theta) }};
}
Vector2 rand_vector2_inBox(Box box) {
    return (Vector2) {{
        box.c_x - box.Dx + 2.f*box.Dx*(float)((double)rand() / (double)RAND_MAX),
        box.c_y - box.Dy + 2.f*box.Dy*(float)((double)rand() / (double)RAND_MAX)
    }};
}

/*-- Extensions de uint. -------------------------------*/
static const uint32_t _pow10s[] = {
    1,       10,       100,
    1000,    10000,    100000,
    1000000, 10000000, 100000000,
    1000000000,
};
static const uint32_t _pow10m1s[] = {
    9,       99,       999,
    9999,    99999,    999999,
    9999999, 99999999, 999999999,
    4294967295
};
/// Nombre de chiffres moins un dans les puissance de 2, i.e.
/// {1, 2, 4, 8 } -> 0, {16, 32, 64} -> 1, {128, 256, 512} -> 2, ...
static const uint32_t _pow2numberOfDigit[] = {
    0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
    6, 6, 6, 6, 7, 7, 7, 8, 8, 8,
    9, 9, 9
};
uint32_t  uint_highestDecimal(uint32_t u) {
    uint32_t highestDecimal = _pow2numberOfDigit[u ? 31 - __builtin_clz(u) : 0];
    return highestDecimal + ((u > _pow10m1s[highestDecimal]) ? 1 : 0);
}
uint32_t  uint_digitAt(uint32_t u, uint32_t decimal) {
    if(decimal > 9) { printerror("Bad decimal %d.", decimal); return 0; }
    return (u / _pow10s[decimal]) % 10;
}
uint32_t  umaxu(uint32_t a, uint32_t b) {
    return a > b ? a : b;
}
uint32_t  uminu(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

void  uintarr_linspace(uint32_t* const u_arr, uint32_t const u_arr_count,
                       uint32_t first, uint32_t const delta) {
    uint32_t* p = u_arr;
    uint32_t* const end = &u_arr[u_arr_count];
    while(p < end) {
        *p = first;
        first += delta;
        p++;
    }
}
void  uintarr_print(uint32_t* u_arr, uint32_t u_arr_count) {
    uint32_t* p = u_arr;
    uint32_t* const end = &u_arr[u_arr_count];
    printf("[ ");
    while(p < end) {
        printf("%5u, ", *p);
        p++;
    }
    printf("]\n");
}

/// Retourne une angle dans l'interval [-pi, pi].
float float_toNormalizedAngle(float f) {
    return f - ceilf((f - M_PI) / (2.f * M_PI)) * 2.f * M_PI;
}
/** Retourne la plus grosse "subdivision" pour le nombre présent en base 10.
 * Le premier chiffre peut être : 1, 2 ou 5. Utile pour les axes de graphiques.
 * e.g.: 792 -> 500, 192 -> 100, 385 -> 200. */
float float_toRoundedSubDiv(float f) {
    float pow10 = powf(10.f, floorf(log10f(f)));
    float mantissa = f / pow10;
    if(mantissa < 2.f)
        return pow10;
    if(mantissa < 5.f)
        return pow10 * 2.f;
    return pow10 * 5;
}
/**-- Fonction "coupé", "en S", i.e.    __/
 *                                     /        */
float float_truncated(float f, float delta) {
    if(f > 0.f) {
        return fmaxf(0.f, f - delta);
    } else {
        return fminf(0.f, f + delta);
    }
}

float float_smoothOut(float x, float lambda) {
    return lambda * (x + 1.f / lambda) * expf(-lambda*x);
}
