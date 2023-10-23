//
//  smpos.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#ifndef smpos_h
#define smpos_h

#include <stdio.h>
#include "maths.h"

typedef struct _SmoothPos {
    // Position par défaut (mémoire, editable)
    float def;
    // Position vers où on tend (dernier set)
    float _pos;
    // Paramètres de la courbe (exponentielle typiquement)
    float _A;
    float _B;
    float _lambda;
    float _beta;
    uint8_t  _type;
    uint32_t _time;
    // float* ref; Superflu ?? Si smoothpos est utilise en priorite.
} SmoothPos;

void  sp_init(SmoothPos *sp, float pos, float lambda);
void  sp_array_init(SmoothPos *sp, float *f, uint count, float lambda);
/// Mettre a jour avec nouveaux gamma et k.
/// gamma -> amortissement.
/// k -> force du ressort.
void  sp_updateToConstants(SmoothPos *sp, float gamma, float k);
/// Convenience de sp_updateToConstants ou gamma = 2*lambda et k = lambda * lambda.
/// i.e. amortissement critique (pas de rebond).
void  sp_updateToLambda(SmoothPos *sp, float lambda);
void  sp_set(SmoothPos *sp, float pos, Bool fix);
// Les convenience du set...
void  sp_setRelToDef(SmoothPos *sp, float shift, Bool fix);
void  sp_move(SmoothPos *sp, float shift, Bool fix);
void  sp_fadeIn(SmoothPos *sp, float delta);
void  sp_fadeInFromDef(SmoothPos *sp, float delta);
void  sp_fadeOut(SmoothPos *sp, float delta);

// Getters
// Position estimee
float   sp_pos(SmoothPos *sp);
// Vrai derniere position entree
float   sp_real(SmoothPos *sp);
// Ecrire les position actuelles vers un array de float.
void    sp_array_writeTo(SmoothPos *sp, float *f, uint count);
// Array de 3 smpos -> Vector3.
Vector3 sp_array_toVec3(SmoothPos *sp);
Vector3 sp_array_toRealVec3(SmoothPos *sp);



#endif /* smpos_h */
