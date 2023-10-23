//
//  maths.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "maths.h"
#include <simd/vector.h>

const Vector2 vector2_ones =  {1, 1};
const Vector2 vector2_zeros = {0, 0};

const Vector3 vector3_ones =  {1, 1, 1};
const Vector3 vector3_zeros = {0, 0, 0};

void vector3_print(Vector3 *v) {
    printf("[ %f, %f, %f ]\n", v->x, v->y, v->z);
}

const Matrix4 matrix4_identity = {
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.,
};

void matrix4_print(Matrix4 *m) {
    for(int j = 0; j < 4; j++) {
        if(j == 0) printf("[ "); else printf("  ");
        for(int i = 0; i< 4; i++)
            printf("%5.2f, ", m->m[i + j*4]);
        if(j == 3) printf("\b ]\n"); else printf("\n");
    }
}

void matrix4_initWithAndTranslate(Matrix4 *m, const Matrix4* const ref, Vector3 t) {
    m->v0 = ref->v0; m->v1 = ref->v1; m->v2 = ref->v2;
    m->v3 = (Vector4) {
        ref->v3.x + ref->v0.x * t.x + ref->v1.x * t.y + ref->v2.x * t.z,
        ref->v3.y + ref->v0.y * t.x + ref->v1.y * t.y + ref->v2.y * t.z,
        ref->v3.z + ref->v0.z * t.x + ref->v1.z * t.y + ref->v2.z * t.z,
        ref->v3.w,
    };
}
void matrix4_initWithRotateYAndTranslateYZ(Matrix4 *m, Matrix4 *ref,
                                           float thetaY, float ty, float tz) {
    float c = cosf(thetaY);
    float s = sinf(thetaY);
    Vector4 v2_rot = s*ref->v0 + c*ref->v2;
    m->v0 =          c*ref->v0 - s*ref->v2;
    m->v1 = ref->v1;
    m->v2 = v2_rot;
    m->v3 = (Vector4){
        ref->v3.x + ref->v1.x * ty + ref->v2.x * tz,
        ref->v3.y + ref->v1.y * ty + ref->v2.y * tz,
        ref->v3.z + ref->v1.z * ty + ref->v2.z * tz,
        ref->v3.w,
    };
}
void matrix4_initAsLookAt(Matrix4 *m, Vector3 eye, Vector3 center, Vector3 up) {
    Vector3 n = simd_normalize(eye - center);
    Vector3 u = simd_normalize(simd_cross(up, n));
    Vector3 v = simd_cross(n, u);
    *m = (Matrix4) {
        u.x, v.x, n.x, 0.f,
        u.y, v.y, n.y, 0.f,
        u.z, v.z, n.z, 0.f,
        -simd_dot(u, eye), -simd_dot(v, eye), -simd_dot(n, eye), 1.f,
    };
}
void matrix4_initAsPerspective(Matrix4 *m, float theta, float ratio,
                               float nearZ, float farZ) {
    float cotan = 1.f / tanf(theta / 2.f);
    *m = (Matrix4) {
        cotan / ratio, 0, 0, 0,
        0, cotan, 0, 0,
        0, 0, (farZ + nearZ) / (nearZ - farZ), -1.f,
        0, 0, (2 * farZ * nearZ) / (nearZ - farZ), 0.f,
    };
}
void matrix4_initAsPerspectiveDeltas(Matrix4 *m, float nearZ, float farZ, float middleZ,
                                     float deltaX, float deltaY) {
    *m = (Matrix4) {
        2.f * middleZ / deltaX, 0, 0, 0,
        0, 2.f * middleZ / deltaY, 0, 0,
        0, 0, (farZ + nearZ) / (nearZ - farZ), -1.f,
        0, 0, (2 * farZ * nearZ) / (nearZ - farZ), 0.f,
    };
}
void matrix4_scale(Matrix4 *m, float sx, float sy, float sz) {
    m->v0 *= sx;
    m->v1 *= sy;
    m->v2 *= sz;
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
    m->v1 = c*v1 + s*v2;
    m->v2 = c*v2 - s*v1;
}
void matrix4_rotateY(Matrix4 *m, float theta) {
    float c = cosf(theta);
    float s = sinf(theta);
    Vector4 v0 = m->v0;
    Vector4 v2 = m->v2;
    m->v0 = c*v0 - s*v2;
    m->v2 = s*v0 + c*v2;
}
void matrix4_rotateZ(Matrix4 *m, float theta) {
    float c = cosf(theta);
    float s = sinf(theta);
    Vector4 v0 = m->v0;
    Vector4 v1 = m->v1;
    m->v0 = c*v0 - s*v1;
    m->v1 = s*v0 + c*v1;
}
void matrix4_rotateYandTranslateYZ(Matrix4 *m, float thetaY, float ty, float tz) {
    float c = cosf(thetaY);
    float s = sinf(thetaY);
    Vector4 v0 = m->v0;
    Vector4 v2 = m->v2;
    m->v0 = c*v0 - s*v2; // Ou au long x,y,z ? et pas w ?
    m->v2 = s*v0 + c*v2;
    m->v3.x += m->v1.x * ty + m->v2.x * tz;
    m->v3.y += m->v1.y * ty + m->v2.y * tz;
    m->v3.z += m->v1.z * ty + m->v2.z * tz;
}

