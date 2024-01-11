//
//  smpos.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include "_math/_math_flpos.h"
#include "_math/_math_chrono.h"

#define _sm_type_static     0x0000
#define _sm_type_oscAmorti  0x0001
#define _sm_type_amortiCrit 0x0002
#define _sm_type_surAmorti  0x0003
#define _sm_types           0x0003
#define _sm_flag_angle      0x0004

float   _sp_getElapsedSec(FluidPos* sp) {
    return (float)((uint32_t)ChronoRender_elapsedMS() - sp->_time) * 0.001f;
}
float   _sp_getDelta(FluidPos* sp, float elapsedSec) {
    if((sp->_flags & _sm_types) == _sm_type_static) return 0.f;
    if((sp->_flags & _sm_types) == _sm_type_amortiCrit)
        return expf(-sp->_lambda * elapsedSec) * (sp->_A + sp->_B * elapsedSec);
    if((sp->_flags & _sm_types) == _sm_type_surAmorti)
        return sp->_A * expf(-sp->_lambda * elapsedSec)
            + sp->_B * expf(-sp->_beta   * elapsedSec);
    // _sm_type_oscAmorti
    return expf(-sp->_lambda * elapsedSec) *
         (sp->_A * cosf(sp->_beta * elapsedSec) + sp->_B * sinf(sp->_beta * elapsedSec));
}
Vector2 _sp_getDeltaAndSlope(FluidPos* fl, float elapsedSec) {
    if((fl->_flags & _sm_types) == _sm_type_static) return vector2_zeros;
    if((fl->_flags & _sm_types) == _sm_type_amortiCrit)
        return (Vector2) {{
            expf(-fl->_lambda * elapsedSec) * (fl->_A + fl->_B * elapsedSec),
            expf(-fl->_lambda * elapsedSec)
            * (fl->_B*(1 - fl->_lambda*elapsedSec) - fl->_lambda*fl->_A),
        }};
    if((fl->_flags & _sm_types) == _sm_type_surAmorti)
        return (Vector2) {{
              fl->_A * expf(-fl->_lambda * elapsedSec)
            + fl->_B * expf(-fl->_beta   * elapsedSec),
             -fl->_lambda * fl->_A * expf(-fl->_lambda * elapsedSec)
            - fl->_beta   * fl->_B * expf(-fl->_beta   * elapsedSec),
        }};
    // _sm_type_oscAmorti
    return (Vector2) {{
        expf(-fl->_lambda * elapsedSec) *
         (fl->_A * cosf(fl->_beta * elapsedSec) + fl->_B * sinf(fl->_beta * elapsedSec)),
        expf(-fl->_lambda * elapsedSec)
        * ( cosf(fl->_beta * elapsedSec) * (fl->_beta*fl->_B   - fl->_lambda*fl->_A)
          - sinf(fl->_beta * elapsedSec) * (fl->_lambda*fl->_B + fl->_beta*fl->_A) ),
    }};
}

void  _sp_setABT(FluidPos* fl, Vector2 ds) {
    switch(fl->_flags & _sm_types) {
        case _sm_type_oscAmorti:
            fl->_A = ds.x;
            fl->_B = (ds.y + fl->_lambda * fl->_A) / fl->_beta;
            break;
        case _sm_type_amortiCrit:
            fl->_A = ds.x;
            fl->_B = ds.y + fl->_lambda * fl->_A;
            break;
        case _sm_type_surAmorti:
            fl->_A = (fl->_beta * ds.x + ds.y) / (fl->_beta - fl->_lambda);
            fl->_B = ds.x - fl->_A;
            break;
        default: // _sm_type_static:
            fl->_A = 0.f; fl->_B = 0.f;
            break;
    }
    fl->_time = (uint32_t)ChronoRender_elapsedMS();
}
void  _sp_setLambdaBetaType(FluidPos *sp, float gamma, float k) {
    sp->_flags &= ~ _sm_types;
    if(gamma == 0 && k == 0) {
        sp->_flags |= _sm_type_static;
        sp->_lambda = 0.f; sp->_beta = 0.f;
        return;
    }
    float discr = gamma * gamma - 4.f * k;
    if(discr > 0.001f) {
        sp->_flags |= _sm_type_surAmorti;
        sp->_lambda = gamma + sqrtf(discr) / 2.f;
        sp->_beta =   gamma - sqrtf(discr) / 2.f;
        return;
    }
    if(discr < -0.001f) {
        sp->_flags |= _sm_type_oscAmorti;
        sp->_lambda = gamma / 2.f;
        sp->_beta = sqrtf(-discr);
        return;
    }
    sp->_flags |= _sm_type_amortiCrit;
    sp->_lambda = gamma / 2.f;
    sp->_beta =   gamma / 2.f;
}
void  fl_updateToConstants(FluidPos *sp, float gamma, float k) {
    // 1. Enregistrer delta et pente avant de modifier la courbe.
    float elapsedSec = _sp_getElapsedSec(sp);
    Vector2 deltaSlope = _sp_getDeltaAndSlope(sp, elapsedSec);
    // 2. Mise à jour des paramètres de la courbe
    _sp_setLambdaBetaType(sp, gamma, k);
    // 3. Réévaluer a/b pour nouveau lambda/beta et reset time.
    _sp_setABT(sp, deltaSlope);
}
void  fl_updateToLambda(FluidPos *fl, float lambda) {
    fl_updateToConstants(fl, 2*lambda, lambda * lambda);
}

void  fl_init(FluidPos* sp, float pos, float lambda, bool asAngle) {
    if(asAngle)
        pos = float_toNormalizedAngle(pos);
    memset(sp, 0, sizeof(FluidPos));
    sp->_pos = pos;
    sp->def =  pos;
    sp->_time = (uint32_t)ChronoApp_elapsedMS();
    sp->_flags = _sm_type_static | (asAngle ? _sm_flag_angle : 0);
    
    _sp_setLambdaBetaType(sp, 2*lambda, lambda * lambda);
}
void  fl_set(FluidPos* sp, float pos) {
    float elapsedSec = _sp_getElapsedSec(sp);
    Vector2 deltaSlope = _sp_getDeltaAndSlope(sp, elapsedSec);
    deltaSlope.x += sp->_pos - pos;
    if(sp->_flags & _sm_flag_angle)
        deltaSlope.x = float_toNormalizedAngle(deltaSlope.x);
    _sp_setABT(sp, deltaSlope);
    if(sp->_flags & _sm_flag_angle)
        sp->_pos = float_toNormalizedAngle(pos);
    else
        sp->_pos = pos;
}
void  fl_fix(FluidPos* sp, float pos) {
    sp->_A = 0.f; sp->_B = 0.f;
    if(sp->_flags & _sm_flag_angle)
        sp->_pos = float_toNormalizedAngle(pos);
    else
        sp->_pos = pos;
}

// Les convenience du set...
void  fl_setRelToDef(FluidPos *sp, float shift) {
    fl_set(sp, sp->def + shift);
}
void  fl_move(FluidPos *sp, float shift) {
    fl_set(sp, sp->_pos + shift);
}
void  fl_fadeIn(FluidPos *sp, float delta) {
    fl_fix(sp, sp->def + delta);
    fl_set(sp, sp->def);
}
void  fl_fadeInFromDef(FluidPos *sp, float delta) {
    fl_fix(sp, sp->def);
    fl_set(sp, sp->def + delta);
}
void  fl_fadeOut(FluidPos *sp, float delta) {
    fl_set(sp, sp->_pos - delta);
}

void  fl_array_init(FluidPos *sp, const float *f, size_t count, float lambda) {
    const float *end = &f[count];
    while(f < end) {
        fl_init(sp, *f, lambda, false);
        sp ++;
        f ++;
    }
}
void  fl_array_set(FluidPos *sp, const float *f, size_t count) {
    const float *end = &f[count];
    while(f < end) {
        fl_set(sp, *f);
        sp ++;
        f ++;
    }
}
void  fl_array_fix(FluidPos *sp, const float *f, size_t count) {
    const float *end = &f[count];
    while(f < end) {
        fl_fix(sp, *f);
        sp ++;
        f ++;
    }
}
void  fl_array_writeTo(FluidPos *sp, float *f, size_t count) {
    float *end = &f[count];
    while(f < end) {
        *f = fl_pos(sp);
        sp ++;
        f ++;
    }
}
Vector2 fl_array_toVec2(FluidPos *sp) {
    return (Vector2) {{
        fl_pos(sp), fl_pos(&sp[1]),
    }};
}
Vector3 fl_array_toVec3(FluidPos *sp) {
    return (Vector3) {{
        fl_pos(sp), fl_pos(&sp[1]), fl_pos(&sp[2]),
    }};
}
Vector3 fl_array_toRealVec3(FluidPos *sp) {
    return (Vector3) {{
        sp[0]._pos, sp[1]._pos, sp[2]._pos,
    }};
}

// Array de 4 smpos -> Vector4 (i.e. simd_float4, 4 aligned float)
Vector4 fl_array_toVec4(FluidPos *sp) {
    return (Vector4) {{
        fl_pos(&sp[0]), fl_pos(&sp[1]), fl_pos(&sp[2]), fl_pos(&sp[3]),
    }};
}

float fl_pos(FluidPos *sp) {
    float elapsedSec = _sp_getElapsedSec(sp);
    return _sp_getDelta(sp, elapsedSec) + sp->_pos;
}
float   fl_real(FluidPos *sp) {
    return sp->_pos;
}
bool    fl_isStatic(FluidPos *fl) {
    return (fl->_flags & _sm_types) == _sm_type_static;
}

void  fld_init(FluidPosWithDrift* fld, float pos, float lambda, bool asAngle) {
    fl_init(&fld->fl, pos, lambda, asAngle);
    fld->drift = 0.f;
}
void  fld_set(FluidPosWithDrift* spd, float pos, float drift) {
    float elapsedSec = _sp_getElapsedSec(&spd->fl);
    Vector2 deltaSlope = _sp_getDeltaAndSlope(&spd->fl, elapsedSec);
    deltaSlope.x += spd->fl._pos + elapsedSec * spd->drift - pos;
    if(spd->fl._flags & _sm_flag_angle)
        deltaSlope.x = float_toNormalizedAngle(deltaSlope.x);
    deltaSlope.y += spd->drift - drift;
    _sp_setABT(&spd->fl, deltaSlope);
    if(spd->fl._flags & _sm_flag_angle)
        spd->fl._pos = float_toNormalizedAngle(pos);
    else
        spd->fl._pos = pos;
    spd->drift = drift;
}
float fld_pos(FluidPosWithDrift* spd) {
    float elapsedSec = _sp_getElapsedSec(&spd->fl);
    return _sp_getDelta(&spd->fl, elapsedSec)
        + spd->fl._pos + elapsedSec * spd->drift;
}
