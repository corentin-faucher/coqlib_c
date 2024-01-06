//
//  _math_flpos.h
//  Position (float) "smooth", i.e. avec transition pour se placer
//  à la nouvelle position (avec fonctions exp, sin, cos...)
//
//  Created by Corentin Faucher on 2023-10-14.
//

#ifndef _coq_math_flpos_h
#define _coq_math_flpos_h

#include "_math_.h"

// N'est pas caché, sinon ne pourrait pas être placé dans d'autres structs.
typedef struct _FluidPos {
    // Position vers où on tend (dernier set) ou "real" pos.
    float    _pos;
    // Position par défaut (mémoire, editable)
    float    def;
    // Paramètres de la courbe (exponentielle typiquement)
    float    _A;
    float    _B;
    float    _lambda;
    float    _beta;
    uint32_t _time;
    uint32_t _flags;
    // float* ref; Superflu ?? Si smoothpos est utilise en priorite.
} FluidPos;

void  fl_init(FluidPos *fl, float pos, float lambda, bool asAngle);
/// Mettre a jour avec nouveaux gamma et k.
/// gamma -> amortissement.
/// k -> constance du ressort.
/// Voir éq.Diff. de l'oscillateur avec amortissement : `x'' + gamma x' + k x = 0`.
void  fl_updateToConstants(FluidPos *fl, float gamma, float k);
/// Convenience de `fl_updateToConstants` ou `gamma = 2*lambda` et `k = lambda * lambda`.
/// i.e. amortissement critique (pas de rebond).
void  fl_updateToLambda(FluidPos *fl, float lambda);
/// Setter "smooth" de FluidPos. La position va tendre vers `pos`.
void  fl_set(FluidPos *fl, float pos);
/// Setter "hard" de FluidPos. La position est fixée directement à `pos`.
void  fl_fix(FluidPos* sp, float pos);
// Les convenience du set...
void  fl_setRelToDef(FluidPos *fl, float shift);
void  fl_move(FluidPos *fl, float shift);
void  fl_fadeIn(FluidPos *fl, float delta);
void  fl_fadeInFromDef(FluidPos *fl, float delta);
void  fl_fadeOut(FluidPos *fl, float delta);
// Getters
/// Position estimee
float   fl_pos(FluidPos *sp);
/// Vrai derniere position entree
float   fl_real(FluidPos *sp);
/// Si static -> n'est pas "fluid", reagit comme un float ordinaire.
bool    fl_isStatic(FluidPos *sp);

// Array de SmoothPos
void  fl_array_init(FluidPos *fl, const float *src_f_arr, size_t count, float lambda);
void  fl_array_set(FluidPos *fl, const float *f, size_t count);
void  fl_array_fix(FluidPos *fl, const float *f, size_t count);
// Ecrire les position actuelles vers un array de float.
void  fl_array_writeTo(FluidPos *fl, float *dst_f_arr, size_t count);
/// Array de 2 smpos -> Vector2.
Vector2 fl_array_toVec2(FluidPos *sp);
/// Array de 3 smpos -> Vector3.
Vector3 fl_array_toVec3(FluidPos *sp);
Vector3 fl_array_toRealVec3(FluidPos *sp);
/// Array de 4 smpos -> Vector4 (i.e. simd_float4, 4 aligned float)
Vector4 fl_array_toVec4(FluidPos *sp);

typedef struct _FluidPosWithDrift {
    FluidPos  fl;
    float     drift;
} FluidPosWithDrift;

void  fld_init(FluidPosWithDrift* fld, float pos, float lambda, bool asAngle);
void  fld_set(FluidPosWithDrift* fld, float pos, float drift);
float fld_pos(FluidPosWithDrift* spd);

#endif /* smpos_h */
