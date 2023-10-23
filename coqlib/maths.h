//
//  maths.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef maths_h
#define maths_h

#include <stdio.h>
#include <simd/simd.h>

#define true 1
#define false 0
typedef int32_t Bool;

// Si on veut l'allignement, utiliser struct __attribute__((aligned(16))) Vector { union {...};};
// Mais je crois qu'on n'en a pas vraiment besoin...

// En fait, pour l'instant on va garder les simd.
// (C'est pas mal la meme chose que mes union de struct...)
// Mais les struct "maison" reviennent pas mal au meme avec les optimisations...

typedef union {
    float v[2];
    struct {float x, y;};
    struct {float w, h;};
} Vector2;
//typedef simd_float2 Vector2;
extern const Vector2 vector2_ones;
extern const Vector2 vector2_zeros;

// Structure "Rectangle" pour les zone de NSView, view SDL, ...
typedef union {
    float v[4];
    float x, y, w, h;
    struct {
        Vector2 origin;
        Vector2 size;
    };
} Rectangle;

//typedef union {
//    float v[3];
//    struct { float x, y, z; };
//    struct { float r, g, b; };
//} Vector3;
typedef simd_float3 Vector3;
extern const Vector3 vector3_ones;
extern const Vector3 vector3_zeros;

void vector3_print(Vector3* v);

//typedef union _Vector4 {
//    float v[4];
//    struct { float x, y, z, w; };
//    struct { float r, g, b, a; };
//};
typedef simd_float4 Vector4;

typedef union {
    float m[16];  // En premier pour init des valeurs.
    struct { Vector4 v0, v1, v2, v3; };
} Matrix4;
// simd_float4x4 n'est pas pratique pour les init...
//typedef simd_float4x4 Matrix4;

// Matrice avec des 1 sur la diagonale.
extern const Matrix4 matrix4_identity;


/*-- Fonction de manipulation des matrices... --*/
// Pour l'instant toute les opérations sont des produits
// s'ajoutant à DROITE de la matrice de modèle (donc "prior").
// Car on commence pas les noeuds racines pour l'affichage...
// (pas encore eu besoin d'opération "post" ou à gauche.)

void matrix4_print(Matrix4 *m);

void matrix4_initWithAndTranslate(Matrix4 *m, const Matrix4* ref, Vector3 t);
void matrix4_initWithRotateYAndTranslateYZ(Matrix4 *m, Matrix4 *ref,
                                           float thetaY, float ty, float tz);
void matrix4_initAsLookAt(Matrix4 *m, Vector3 eye, Vector3 center, Vector3 up);
void matrix4_initAsPerspective(Matrix4 *m, float theta, float ratio,
                               float nearZ, float farZ);
void matrix4_initAsPerspectiveDeltas(Matrix4 *m, float nearZ, float farZ, float middleZ,
                                     float deltaX, float deltaY);

void matrix4_scale(Matrix4 *m, float sx, float sy, float sz);
void matrix4_translate(Matrix4 *m, Vector3 t);
void matrix4_rotateX(Matrix4 *m, float theta);
void matrix4_rotateY(Matrix4 *m, float theta);
void matrix4_rotateZ(Matrix4 *m, float theta);
void matrix4_rotateYandTranslateYZ(Matrix4 *m, float thetaY, float ty, float tz);

#endif /* maths_h */
