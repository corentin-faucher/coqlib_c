//
//  maths.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "math_base.h"

#include "../utils/util_base.h"

#pragma mark - Vecteur 2D -------------------------
const Vector2 vector2_ones =  {{1, 1}};
const Vector2 vector2_zeros = {{0, 0}};

Vector2 vector2_projOn(Vector2 const v, Vector2 const target) {
    float dot = v.x*target.x + v.y*target.y;
    float t2 = target.x*target.x + target.y*target.y;
    return (Vector2) {{
        dot * target.x / t2,
        dot * target.y / t2
    }};
}
Vector2 vector2_normalize(Vector2 v) {
    float n = sqrtf(v.x*v.x + v.y*v.y);
    if(n < 0.0001)
        return rand_vector2_ofNorm(1.f);
    return (Vector2) {{ v.x / n, v.y / n }};
}

static char  tmp_vector_c_str_[30];
const char* vector2_toString(Vector2 const v) {
    sprintf(tmp_vector_c_str_, "[ %5.2f, %5.2f ]", v.x, v.y);
    return tmp_vector_c_str_;
}

#pragma mark - Vecteur 3D -------------------------
const Vector3 vector3_ones =  {{1, 1, 1}};
const Vector3 vector3_zeros = {{0, 0, 0}};
Vector3 vector3_normalize(Vector3 const v) {
    float n = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    return (Vector3) {{ v.x / n, v.y / n, v.z / n }};
}
const char* vector3_toString(Vector3 const v) {
    sprintf(tmp_vector_c_str_, "[ %5.2f, %5.2f, %5.2f ]", v.x, v.y, v.z);
    return tmp_vector_c_str_;
}

#pragma mark - Vecteur 4 -------------------------
const Vector4 vector4_ones = {{1, 1, 1, 1 }};
const Vector4 vector4_zeros = {{0, 0, 0, 0 }};

#pragma mark - Matrices 4x4 -------------------------
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
void matrix4_initWithAndTranslate(Matrix4 *m, const Matrix4* const ref, Vector3 t) {
    m->v0 = ref->v0; m->v1 = ref->v1; m->v2 = ref->v2;
    m->v3 = (Vector4) {{
        ref->v3.x + ref->v0.x * t.x + ref->v1.x * t.y + ref->v2.x * t.z,
        ref->v3.y + ref->v0.y * t.x + ref->v1.y * t.y + ref->v2.y * t.z,
        ref->v3.z + ref->v0.z * t.x + ref->v1.z * t.y + ref->v2.z * t.z,
        ref->v3.w,
    }};
}
void matrix4_initWithRotateYAndTranslateYZ(Matrix4 *m, const Matrix4* const ref,
                                           float thetaY, float ty, float tz) {
    float c = cosf(thetaY);
    float s = sinf(thetaY);
    m->v0.v = c*ref->v0.v - s*ref->v2.v;
    m->v1.v = ref->v1.v;
    m->v2.v = s*ref->v0.v + c*ref->v2.v;
    m->v3.v = ref->v3.v + ref->v1.v * ty + m->v2.v * tz;
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
    // Bonne ref pour la théorie : http://www.songho.ca/opengl/gl_projectionmatrix.html
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
    // Ici, on utilise l'astuce du w = -1 dans v2 pour faire un 1/z...
    // z sont négatif car on regarde vers les z négatifs...
    // Bonne ref pour la théorie : http://www.songho.ca/opengl/gl_projectionmatrix.html
    *m = (Matrix4) {{
        middleZ / deltaX, 0.0,     0.0,           0.0,
        0.0, middleZ/deltaY,       0.0,           0.0,
        0.0, 0.0, (farZ + nearZ)/(nearZ - farZ), -1.0,
        0.0, 0.0, (2*farZ*nearZ)/(nearZ - farZ),  0.0,
    }};
}
void matrix4_initAsPerspectiveAndLookAt(Matrix4* m,
        Vector3 eye, Vector3 center, Vector3 up,
        float nearZ, float farZ, float middleZ, float deltaX, float deltaY) {
    // Direction vers eye.
    Vector3 direc = vector3_normalize(vector3_minus(eye, center)); // n->dir
    Vector3 right = vector3_normalize(vector3_cross(up, direc)); // u -> right
    // Up pour la camera
    up = vector3_cross(direc, right); // v-> up
    float r_e = vector3_dot(right, eye), u_e = vector3_dot(up, eye), d_e = vector3_dot(direc, eye);
    // Pour la projection... (voir matrix4_initAsPerspectiveDeltas)
    float sx = middleZ / deltaX;
    float sy = middleZ / deltaY;
    float sz = (farZ + nearZ)/(nearZ - farZ);
    float wz = (2*farZ*nearZ)/(nearZ - farZ);
    *m = (Matrix4) {{
        sx*right.x, sy*up.x, sz*direc.x, -direc.x,
        sx*right.y, sy*up.y, sz*direc.y, -direc.y,
        sx*right.z, sy*up.z, sz*direc.z, -direc.z,
        -sx*r_e,    -sy*u_e, -sz*d_e+wz,  d_e,   
    }};
}
Matrix4 matrix4_product(const Matrix4* a, const Matrix4* b) {
    Matrix4 c;
    c.v0.v = a->v0.v * b->v0.x + a->v1.v * b->v0.y + a->v2.v * b->v0.z + a->v3.v * b->v0.w;
    c.v1.v = a->v0.v * b->v1.x + a->v1.v * b->v1.y + a->v2.v * b->v1.z + a->v3.v * b->v1.w;
    c.v2.v = a->v0.v * b->v2.x + a->v1.v * b->v2.y + a->v2.v * b->v2.z + a->v3.v * b->v2.w;
    c.v3.v = a->v0.v * b->v3.x + a->v1.v * b->v3.y + a->v2.v * b->v3.z + a->v3.v * b->v3.w;
    return c;
}

void matrix4_scale(Matrix4 *m, float sx, float sy, float sz) {
    m->v0.v *= sx;
    m->v1.v *= sy;
    m->v2.v *= sz;
}
void matrix4_translate(Matrix4 *m, Vector3 t) {
    m->v3.v += m->v0.v * t.x + m->v1.v * t.y + m->v2.v * t.z;
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
//    m->v3.x += m->v1.x * ty + m->v2.x * tz;
//    m->v3.y += m->v1.y * ty + m->v2.y * tz;
//    m->v3.z += m->v1.z * ty + m->v2.z * tz;
}


#pragma mark - Générateurs de nombres aléatoires -------------------------
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

#pragma mark -- Extension de bool --
const char* bool_toString(bool b) {
    return b ? "true" : "false";
}

#pragma mark - Fonctions utiles sur les unsigneds ---------------------
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

#pragma mark - Fonctions utiles sur les floats ---------------------

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
/**-- Fonction "coupé", "en S", i.e.    __/
 *                                     /        */
float float_truncated(float f, float delta) {
    if(f > 0.f) {
        return fmaxf(0.f, f - delta);
    } else {
        return fminf(0.f, f + delta);
    }
}
float float_appearing(float x, float x0, float deltaX) {
    if(x < x0) return 0.f;
    if(x > x0 + deltaX) return 1.f;
    return (x - x0) / deltaX;
}

