//
//  smpos.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include "smpos.h"
#include <math.h>
#include "chronometers.h"

const static uint8_t _sm_type_static =     0;
const static uint8_t _sm_type_oscAmorti  = 1;
const static uint8_t _sm_type_amortiCrit = 2;
const static uint8_t _sm_type_surAmorti  = 3;

Vector2 _sp_getDeltaAndSlope(SmoothPos *sp) {
    if(sp->_type == _sm_type_static) return vector2_zeros;
    float elapsedSec = (float)((uint32_t)ChronoRender_elapsedMS() - sp->_time) * 0.001f;
    if(sp->_type == _sm_type_amortiCrit)
        return (Vector2) {
            expf(-sp->_lambda * elapsedSec) * (sp->_A + sp->_B * elapsedSec),
            expf(-sp->_lambda * elapsedSec)
            * (sp->_B*(1 - sp->_lambda*elapsedSec) - sp->_lambda*sp->_A)
        };
    if(sp->_type == _sm_type_surAmorti)
        return (Vector2) {
              sp->_A * expf(-sp->_lambda * elapsedSec)
            + sp->_B * expf(-sp->_beta   * elapsedSec),
             -sp->_lambda * sp->_A * expf(-sp->_lambda * elapsedSec)
            - sp->_beta   * sp->_B * expf(-sp->_beta   * elapsedSec)
        };
    // _sm_type_oscAmorti
    return (Vector2) {
        expf(-sp->_lambda * elapsedSec) *
         (sp->_A * cosf(sp->_beta * elapsedSec) + sp->_B * sinf(sp->_beta * elapsedSec)),
        expf(-sp->_lambda * elapsedSec)
        * ( cosf(sp->_beta * elapsedSec) * (sp->_beta*sp->_B   - sp->_lambda*sp->_A)
          - sinf(sp->_beta * elapsedSec) * (sp->_lambda*sp->_B + sp->_beta*sp->_A) )
    };
}

void  _sp_setABT(SmoothPos *sp, Vector2 ds) {
    switch(sp->_type) {
        case _sm_type_oscAmorti:
            sp->_A = ds.x;
            sp->_B = (ds.y + sp->_lambda * sp->_A) / sp->_beta;
            break;
        case _sm_type_amortiCrit:
            sp->_A = ds.x;
            sp->_B = ds.y + sp->_lambda * sp->_A;
            break;
        case _sm_type_surAmorti:
            sp->_A = (sp->_beta * ds.x + ds.y) / (sp->_beta - sp->_lambda);
            sp->_B = ds.x - sp->_A;
            break;
        default: // _sm_type_static:
            sp->_A = 0.f; sp->_B = 0.f;
            break;
    }
    sp->_time = (uint32_t)ChronoRender_elapsedMS();
}
void  _sp_setLambdaBetaType(SmoothPos *sp, float gamma, float k) {
    if(gamma == 0 && k == 0) {
        sp->_type = _sm_type_static;
        sp->_lambda = 0.f; sp->_beta = 0.f;
        return;
    }
    float discr = gamma * gamma - 4.f * k;
    if(discr > 0.001f) {
        sp->_type = _sm_type_surAmorti;
        sp->_lambda = gamma + sqrtf(discr) / 2.f;
        sp->_beta =   gamma - sqrtf(discr) / 2.f;
        return;
    }
    if(discr < -0.001f) {
        sp->_type = _sm_type_oscAmorti;
        sp->_lambda = gamma / 2.f;
        sp->_beta = sqrtf(-discr);
        return;
    }
    sp->_type = _sm_type_amortiCrit;
    sp->_lambda = gamma / 2.f;
    sp->_beta =   gamma / 2.f;
}
void  sp_updateToConstants(SmoothPos *sp, float gamma, float k) {
    // 1. Enregistrer delta et pente avant de modifier la courbe.
    Vector2 deltaSlope = _sp_getDeltaAndSlope(sp);
    // 2. Mise à jour des paramètres de la courbe
    _sp_setLambdaBetaType(sp, gamma, k);
    // 3. Réévaluer a/b pour nouveau lambda/beta et reset time.
    _sp_setABT(sp, deltaSlope);
}
void  sp_updateToLambda(SmoothPos *sp, float lambda) {
    sp_updateToConstants(sp, 2*lambda, lambda * lambda);
}

void  sp_init(SmoothPos *sp, float pos, float lambda) {
    *sp = (SmoothPos) {
        pos, pos,
        0, 0, 0, 0, _sm_type_static,
        (uint32_t)ChronoApp_elapsedMS(),
    };
    _sp_setLambdaBetaType(sp, 2*lambda, lambda * lambda);
}
void  sp_set(SmoothPos *sp, float pos, Bool fix) {
    if(fix) {
        sp->_A = 0.f; sp->_B = 0.f;
    } else {
        Vector2 deltaSlope = _sp_getDeltaAndSlope(sp);
        deltaSlope.x += sp->_pos - pos;
        _sp_setABT(sp, deltaSlope);
    }
    sp->_pos = pos;
}
// Les convenience du set...
void  sp_setRelToDef(SmoothPos *sp, float shift, Bool fix) {
    sp_set(sp, sp->def + shift, fix);
}
void  sp_move(SmoothPos *sp, float shift, Bool fix) {
    sp_set(sp, sp->_pos + shift, fix);
}
void  sp_fadeIn(SmoothPos *sp, float delta) {
    sp_set(sp, sp->def + delta, true);
    sp_set(sp, sp->def, false);
}
void  sp_fadeInFromDef(SmoothPos *sp, float delta) {
    sp_set(sp, sp->def, true);
    sp_set(sp, sp->def + delta, false);
}
void  sp_fadeOut(SmoothPos *sp, float delta) {
    sp_set(sp, sp->_pos - delta, false);
}

void  sp_array_init(SmoothPos *sp, const float *f, uint count, float lambda) {
    const float *end = &f[count];
    while(f < end) {
        sp_init(sp, *f, lambda);
        sp ++;
        f ++;
    }
}
void  sp_array_set(SmoothPos *sp, const float *f, uint count, Bool fix) {
    const float *end = &f[count];
    while(f < end) {
        sp_set(sp, *f, fix);
        sp ++;
        f ++;
    }
}
void  sp_array_writeTo(SmoothPos *sp, float *f, uint count) {
    float *end = &f[count];
    while(f < end) {
        *f = sp_pos(sp);
        sp ++;
        f ++;
    }
}
Vector2 sp_array_toVec2(SmoothPos *sp) {
    return (Vector2) {
        sp_pos(sp), sp_pos(&sp[1]),
    };
}
Vector3 sp_array_toVec3(SmoothPos *sp) {
    return (Vector3) {
        sp_pos(sp), sp_pos(&sp[1]), sp_pos(&sp[2]),
    };
}
Vector3 sp_array_toRealVec3(SmoothPos *sp) {
    return (Vector3) {
        sp[0]._pos, sp[1]._pos, sp[2]._pos,
    };
}

// Array de 4 smpos -> Vector4 (i.e. simd_float4, 4 aligned float)
Vector4 sp_array_toVec4(SmoothPos *sp) {
    return (Vector4) {
        sp_pos(&sp[0]), sp_pos(&sp[1]), sp_pos(&sp[2]), sp_pos(&sp[3]),
    };
}


float sp_pos(SmoothPos *sp) {
    if(sp->_type == _sm_type_static) return sp->_pos;
    float elapsedSec = (float)((uint32_t)ChronoRender_elapsedMS() - sp->_time) * 0.001f;
    if(sp->_type == _sm_type_amortiCrit)
        return (sp->_A + sp->_B * elapsedSec) * expf(-sp->_lambda * elapsedSec) + sp->_pos;
    if(sp->_type == _sm_type_surAmorti)
        return sp->_A * expf(-sp->_lambda * elapsedSec)
             + sp->_B * expf(-sp->_beta   * elapsedSec) + sp->_pos;;
    // _sm_type_oscAmorti
    return expf(-sp->_lambda * elapsedSec) *
        (sp->_A * cosf(sp->_beta * elapsedSec)
       + sp->_B * sinf(sp->_beta * elapsedSec)) + sp->_pos;
}
float sp_real(SmoothPos *sp) {
    return sp->_pos;
}

