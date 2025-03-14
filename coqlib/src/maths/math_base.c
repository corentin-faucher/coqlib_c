//
//  maths.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//
#include "math_base.h"
#include "../utils/util_base.h"

// MARK: - Vecteur 2D
Vector2 vector2_projOn(Vector2 const v, Vector2 const target) {
    float dot = v.x*target.x + v.y*target.y;
    float t2 = target.x*target.x + target.y*target.y;
    return (Vector2) {{
        dot * target.x / t2,
        dot * target.y / t2
    }};
}
Vector2 vector2_normalize(Vector2 v) {
    float n = hypotf(v.x, v.y);
    if(n < 0.0001)
        return rand_vector2_ofNorm(1.f);
    return (Vector2) {{ v.x / n, v.y / n }};
}

static char     tmp_vector_c_str_[10][30];
static uint32_t tmp_vector_index_ = 0;
const char* vector2_toString(Vector2 const v) {
    char* c_str = tmp_vector_c_str_[tmp_vector_index_];
    tmp_vector_index_ = (tmp_vector_index_ + 1) % 10;
    sprintf(c_str, "[ %5.2f, %5.2f ]", v.x, v.y);
    return c_str;
}

// MARK: - Vecteur 3D
//const Vector3 vector3_ones =  {{1, 1, 1}};
//const Vector3 vector3_zeros = {{0, 0, 0}};
Vector3 vector3_normalize(Vector3 const v) {
    float n = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    // ou hypotf(hypotf(v.x, v.y), v.z) ?
    return (Vector3) {{ v.x / n, v.y / n, v.z / n }};
}
const char* vector3_toString(Vector3 const v) {
    char* c_str = tmp_vector_c_str_[tmp_vector_index_];
    tmp_vector_index_ = (tmp_vector_index_ + 1) % 10;
    sprintf(c_str, "[ %5.2f, %5.2f, %5.2f ]", v.x, v.y, v.z);
    return c_str;
}

// MARK: - Vecteur 4

// MARK: - Matrices 4x4
const Matrix4 matrix4_identity = {{
    1., 0., 0., 0., // v0, colonne de x, i.e. ce que devient (1, 0, 0, 0).
    0., 1., 0., 0., // v1
    0., 0., 1., 0., // v2
    0., 0., 0., 1., // v3
}};
void matrix4_print(const Matrix4 *m) {
    for(int j = 0; j < 4; j++) {
        if(j == 0) printf("[ "); else printf("  ");
        for(int i = 0; i< 4; i++)
            printf("%5.2f, ", m->f_arr[i + j*4]);
        if(j == 3) printf("\b ]\n"); else printf("\n");
    }
}
void matrix4_initWithAndTranslate(Matrix4 *m, Matrix4 const*const ref, Vector3 const t) {
    m->v_arr[0] = ref->v_arr[0];
    m->v_arr[1] = ref->v_arr[1];
    m->v_arr[2] = ref->v_arr[2];
    m->v_arr[3] = ref->v_arr[3] + ((Vector4) {{
        ref->v0.x * t.x + ref->v1.x * t.y + ref->v2.x * t.z,
        ref->v0.y * t.x + ref->v1.y * t.y + ref->v2.y * t.z,
        ref->v0.z * t.x + ref->v1.z * t.y + ref->v2.z * t.z,
        0,}}).v;
}
void matrix4_initWithRotateYAndTranslateYZ(Matrix4 *m, Matrix4 const*const ref,
                                           float const thetaY, float const ty, float const tz) {
    float c = cosf(thetaY);
    float s = sinf(thetaY);
    m->v0.v = c*ref->v0.v - s*ref->v2.v;
    m->v1.v = ref->v1.v;
    m->v2.v = s*ref->v0.v + c*ref->v2.v;
    m->v3.v = ref->v3.v + ref->v1.v * ty + m->v2.v * tz;
}
void matrix4_initAsLookAt(Matrix4 *m, Vector3 const eye, Vector3 const center, Vector3 const up) {
    // Vecteur normalisé vers l'oeil de la caméra, i.e. `eye` par rapport à `center`.
    Vector3 const toEye = vector3_normalize(vector3_minus(eye, center));
    // Vecteur donnant la direction "droite" de la caméra.
    Vector3 const right = vector3_normalize(vector3_cross(up, toEye));
    // Up de la caméra... genre le up de départ normalisé pour être le vrai up de la caméra.
    Vector3 const camUp = vector3_cross(toEye, right);
    // Positionnement dans référentiel de eye...
    float const r_e = -vector3_dot(right, eye);
    float const u_e = -vector3_dot(camUp, eye);
    float const e_e = -vector3_dot(toEye, eye);
    *m = (Matrix4) {{
        right.x, camUp.x, toEye.x, 0.f,
        right.y, camUp.y, toEye.y, 0.f,
        right.z, camUp.z, toEye.z, 0.f,
        r_e,     u_e,     e_e,     1.f,
    }};
}
void matrix4_initAsPerspectiveAndLookAt(Matrix4* m,
        Vector3 const eye, Vector3 const center, Vector3 const up,
        float nearZ, float farZ, float const deltaX, float const deltaY) {
    // Direction vers eye.
    Vector3 const toEyeTmp = vector3_minus(eye, center);
    float const middleZ = vector3_norm(toEyeTmp);
    Vector3 const toEye = vector3_times(toEyeTmp, 1.f/middleZ);
    Vector3 const right = vector3_normalize(vector3_cross(up, toEye));
    if(nearZ >= middleZ) { printwarning("nearZ > midZ."); nearZ = middleZ - 0.01; }
    if(farZ <= middleZ) { printwarning("farZ < midZ."); farZ = middleZ + 0.01; }
    // Up pour la camera
    Vector3 const camUp = vector3_cross(toEye, right);
    // Projections de eye...
    float r_e = vector3_dot(right, eye), u_e = vector3_dot(camUp, eye), d_e = vector3_dot(toEye, eye);
    // Pour la projection... (voir matrix4_initAsPerspectiveDeltas)
    float sx = middleZ / deltaX;
    float sy = middleZ / deltaY;
    float sz = (farZ + nearZ)/(nearZ - farZ);
    float wz = (2*farZ*nearZ)/(nearZ - farZ);
    *m = (Matrix4) {{
        sx*right.x, sy*camUp.x, sz*toEye.x, -toEye.x,
        sx*right.y, sy*camUp.y, sz*toEye.y, -toEye.y,
        sx*right.z, sy*camUp.z, sz*toEye.z, -toEye.z,
        -sx*r_e,    -sy*u_e, -sz*d_e+wz,  d_e,   
    }};
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
    m->v0.v =  c*v0.v + s*v1.v;
    m->v1.v = -s*v0.v + c*v1.v;
}
void matrix4_rotateYandTranslateYZ(Matrix4 *m, float thetaY, float ty, float tz) {
    float c = cosf(thetaY);
    float s = sinf(thetaY);
    Vector4 v0 = m->v0;
    Vector4 v2 = m->v2;
    m->v0.v = c*v0.v - s*v2.v;
    m->v2.v = s*v0.v + c*v2.v;
    
    m->v3.v += m->v1.v * ty + m->v2.v * tz;
}

// MARK: - Générateurs de nombres aléatoires -------------------------
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
int64_t rand_float_toInt(float f) {
    int64_t i = (int64_t)floorf(f);
    return i + (int64_t)rand_bool(f - (float)i);
}
Vector2 rand_vector2_ofNorm(float norm) {
    float theta = 2.f * M_PI * (float)((double)rand() / (double)RAND_MAX);
    return (Vector2) {{ norm*cosf(theta), norm*sinf(theta) }};
}

void uintarr_initHashed(uint32_t *u_arr, size_t count, uint32_t seed) {
    if(!count) return;
    uint32_t *const end = &u_arr[count];
    seed *= 1217;
    uint32_t hash = 0xc4fad76B ^ seed;  // Valeur quelconque du début.
    for(uint32_t *u = u_arr; u < end; u++) {
        *u = hash;
        // Change pas grand chose en fait... ? Faut juste un truc random.
        hash = (((hash << 4) ^ hash ^ (hash >> 2)) * 971) ^ 317;
//        hash = ((hash << 4) ^ *p ^ hash ^ (hash >> 2)) * 53;
//        hash = *p ^ (hash << 6) ^ (hash >> 3) ^ hash;
    }
}

// MARK: - Fonctions utiles sur les unsigneds ---------------------
static const uint32_t uint_pow10s_[] = {
    1,       10,       100,
    1000,    10000,    100000,
    1000000, 10000000, 100000000,
    1000000000,
};
static const uint32_t uint_pow10m1s_[] = {
    9,       99,       999,
    9999,    99999,    999999,
    9999999, 99999999, 999999999,
    4294967295
};
/// Nombre de chiffres moins un dans les puissance de 2, i.e.
/// {1, 2, 4, 8 } -> 0, {16, 32, 64} -> 1, {128, 256, 512} -> 2, ...
static const uint32_t uint_pow2numberOfDigit_[] = {
    0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
    6, 6, 6, 6, 7, 7, 7, 8, 8, 8,
    9, 9, 9
};
uint32_t  uint_highestDecimal(uint32_t u) {
    uint32_t highestDecimal = uint_pow2numberOfDigit_[u ? 31 - __builtin_clz(u) : 0];
    return highestDecimal + ((u > uint_pow10m1s_[highestDecimal]) ? 1 : 0);
}
uint32_t  uint_digitAt(uint32_t u, uint32_t decimal) {
    if(decimal > 9) { printerror("Bad decimal %d.", decimal); return 0; }
    return (u / uint_pow10s_[decimal]) % 10;
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
void  uintarr_print(const uint32_t* u_arr, uint32_t u_arr_count) {
    const uint32_t* p = u_arr;
    const uint32_t* const end = &u_arr[u_arr_count];
    printf("[ ");
    while(p < end) {
        printf("%5u, ", *p);
        p++;
    }
    printf("]\n");    
}

// MARK: - Fonctions utiles sur les floats ---------------------
/// Retourne une angle dans l'interval [-pi, pi].
//float float_toNormalizedAngle(float f) {
//    return f - ceilf((f - M_PI) / (2.f * M_PI)) * 2.f * M_PI;
//}
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

