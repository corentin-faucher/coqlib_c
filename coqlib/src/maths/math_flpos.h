//
//  _math_flpos.h
//  Position "smooth", i.e. avec transitions (typiquement osclillateur amortie (avec exp, sin, cos...)).
// Un FluidPos est édité par la `event thread` au temps `ChronosEvent.render_elapsedMS`
// et est lu par la `render thread` au tems `ChronosRender.render_elapsedMS`.
//
//  Created by Corentin Faucher on 2023-10-14.
//
#ifndef COQ_MATH_FLPOS_H
#define COQ_MATH_FLPOS_H

#include "math_base.h" // pour Vector3

// `Core` de FluidPos. 4 floats, i.e. `vecteur` simd de 128 bits.
typedef union {
    // Un vecteur simd de 128 bits est "atomic", du moins selon des expériences... ;)
    float __attribute__((vector_size(16))) v;
    struct {
        // Position vers où on tend (dernier set) ou "real" pos.
        float    pos;
        // Paramètres d'amplitude de la courbe (exponentielle typiquement)
        // ~ écart par rapport à `pos` target.
        float    A;
        float    B;
        uint32_t time; // Setting time (de `ChronoRender_elapsedMS`).
    };
} FluidPosCore_;

// Paramètres de FluidPos. 4 floats, i.e. `vecteur` simd de 128 bits.
typedef union {
    float __attribute__((vector_size(16))) v;
    struct {
        // Position par défaut (mémoire, editable)
        float    def;
        float    _lambda;
        float    _beta;
        uint32_t _flags;
    };
} FluidPosParams_;

// N'est pas caché, sinon ne pourrait pas être placé dans d'autres structs.
// De taile 256 bits / 8 floats / 2 `vecteurs`.
// Ecris par la event thread (aux 50ms)
// Lu par la rendering thread (aux 16ms)
typedef struct FluidPos {
    FluidPosCore_ _c;
    union {
        FluidPosParams_ _p;
        // Position par défaut (mémoire, editable)
        struct {
        float     def;
        float    _lambda;
        float    _beta;
        uint32_t _flags;
        };
    };
} FluidPos;

void  fl_init(FluidPos *fl, float pos, float lambda, bool asAngle);
void  fl_initGammaK(FluidPos *fl, float pos, float gamma, float k, bool asAngle);
/// Mettre a jour avec nouveaux gamma et k.
/// gamma -> amortissement.
/// k -> constance du ressort.
/// Voir éq.Diff. de l'oscillateur avec amortissement : `x'' + gamma x' + k x = 0`.
void  fl_updateToConstants(volatile FluidPos *fl, float gamma, float k);
/// Convenience de `fl_updateToConstants` ou `gamma = 2*lambda` et `k = lambda * lambda`.
/// i.e. amortissement critique (pas de rebond).
void  fl_updateToLambda(FluidPos *fl, float lambda);

// MARK: - Setters

/// Setter "smooth" de FluidPos. La position va tendre vers `pos`.
void  fl_set(FluidPos *fl, float pos);
/// Setter "hard" de FluidPos. La position est fixée directement à `pos`.
void  fl_fix(FluidPos* sp, float pos);
/// Changement de référentiel quelconques (avec positions et scales absolues).
__attribute__((deprecated("Sans doute obsolete...")))
void  fl_newReferential(FluidPos* fp, float pos, float destPos, float scale, float destScale);
void  fl_newReferentialAsDelta(FluidPos* fp, float scale, float destScale);
void  fl_referentialOut(FluidPos *const fp, float const refX, float const refScale);
void  fl_referentialOutAsDelta(FluidPos *const fp, float const refScale);

// Des setters de convenience (cas particuliers de fl_set / fl_fix)
void  fl_setRelToDef(FluidPos *fl, float shift);
void  fl_move(FluidPos *fl, float shift);
void  fl_fadeIn(FluidPos *fl, float delta);
void  fl_fadeInFromDef(FluidPos *fl, float delta);
void  fl_fadeOut(FluidPos *fl, float delta);

// MARK: - Getters
/// Position estimee
float fl_evalPos(volatile FluidPos const* fp);
/// Vrai derniere position entree
float fl_real(const FluidPos *sp);
/// Si static -> n'est pas "fluid", reagit comme un float ordinaire.
bool  fl_isStatic(const FluidPos *sp);

// Array de SmoothPos
void  fl_array_init(FluidPos *fl, const float *src_f_arr, size_t count, float lambda);
void  fl_array_set(FluidPos *fl, const float *f, size_t count);
void  fl_array_fix(FluidPos *fl, const float *f, size_t count);
// Ecrire les position actuelles vers un array de float.
void  fl_array_writeTo(const FluidPos *fl, float *dst_f_arr, size_t count);
/// Array de 2 smpos -> Vector2.
Vector2 fl_array_toVec2(volatile const FluidPos *sp);
/// Array de 3 smpos -> Vector3.
Vector3 fl_array_toVec3(volatile const FluidPos *sp);
Vector3 fl_array_toRealVec3(const FluidPos *sp);
/// Array de 4 smpos -> Vector4 (i.e. simd_float4, 4 aligned float)
Vector4 fl_array_toVec4(const FluidPos *sp);


// MARK: - Cas particulier settable avec pente (ou "drift").
typedef struct _FluidPosWithDrift {
    FluidPos  fp;
    float     drift;
} FluidPosWithDrift;

void  fld_init(FluidPosWithDrift* fld, float pos, float lambda, bool asAngle);
void  fld_set(FluidPosWithDrift* fld, float pos, float drift);
float fld_evalPos(const FluidPosWithDrift *fpd);

#endif /* smpos_h */
