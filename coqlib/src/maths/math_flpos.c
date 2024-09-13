//
//  smpos.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include <string.h>

#include "math_flpos.h"
#include "math_chrono.h"

#define _sm_type_static     0x0000
#define _sm_type_oscAmorti  0x0001
#define _sm_type_amortiCrit 0x0002
#define _sm_type_surAmorti  0x0003
#define _sm_types           0x0003
#define _sm_flag_angle      0x0004

float   fl_getElapsedSec_(const FluidPos* sp) {
    return (float)((uint32_t)ChronoRender_elapsedMS() - sp->_time) * 0.001f;
}
float   fl_getDelta_(const FluidPos* fl, float elapsedSec) {
    if((fl->_flags & _sm_types) == _sm_type_static) return 0.f;
    if((fl->_flags & _sm_types) == _sm_type_amortiCrit)
        return expf(-fl->_lambda * elapsedSec) * (fl->_A + fl->_B * elapsedSec);
    if((fl->_flags & _sm_types) == _sm_type_surAmorti)
        return fl->_A * expf(-fl->_lambda * elapsedSec)
            + fl->_B * expf(-fl->_beta   * elapsedSec);
    // _sm_type_oscAmorti
    return expf(-fl->_lambda * elapsedSec) *
         (fl->_A * cosf(fl->_beta * elapsedSec) + fl->_B * sinf(fl->_beta * elapsedSec));
}
Vector2 fl_getDeltaAndSlope_(const FluidPos* fl, float elapsedSec) {
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

void  fl_setABT_(FluidPos* fl, Vector2 ds) {
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
void  fl_setLambdaBetaType_(FluidPos *sp, float gamma, float k) {
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
    float elapsedSec = fl_getElapsedSec_(sp);
    Vector2 deltaSlope = fl_getDeltaAndSlope_(sp, elapsedSec);
    // 2. Mise à jour des paramètres de la courbe
    fl_setLambdaBetaType_(sp, gamma, k);
    // 3. Réévaluer a/b pour nouveau lambda/beta et reset time.
    fl_setABT_(sp, deltaSlope);
}
void  fl_updateToLambda(FluidPos *fl, float lambda) {
    fl_updateToConstants(fl, 2*lambda, lambda * lambda);
}

void  fl_init(FluidPos* sp, float pos, float lambda, bool asAngle) {
    if(asAngle) pos = float_toNormalizedAngle(pos);
    memset(sp, 0, sizeof(FluidPos));
    sp->_pos = pos;
    sp->def =  pos;
    sp->_time = (uint32_t)ChronoApp_elapsedMS();
    sp->_flags = _sm_type_static | (asAngle ? _sm_flag_angle : 0);
    
    fl_setLambdaBetaType_(sp, 2*lambda, lambda * lambda);
}
void  fl_initGammaK(FluidPos *sp, float pos, float gamma, float k, bool asAngle) {
    if(asAngle) pos = float_toNormalizedAngle(pos);
    memset(sp, 0, sizeof(FluidPos));
    sp->_pos = pos;
    sp->def =  pos;
    sp->_time = (uint32_t)ChronoApp_elapsedMS();
    sp->_flags = _sm_type_static | (asAngle ? _sm_flag_angle : 0);
    
    fl_setLambdaBetaType_(sp, gamma, k);
}
void  fl_set(FluidPos* sp, float pos) {
    float elapsedSec = fl_getElapsedSec_(sp);
    Vector2 deltaSlope = fl_getDeltaAndSlope_(sp, elapsedSec);
    deltaSlope.x += sp->_pos - pos;
    if(sp->_flags & _sm_flag_angle)
        deltaSlope.x = float_toNormalizedAngle(deltaSlope.x);
    fl_setABT_(sp, deltaSlope);
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
/** Changement de référentiel quelconques (avec positions et scales absolues). */
void  fl_newReferential(FluidPos* fp, float pos, float destPos, float scale, float destScale) {
    fp->_pos = (pos - destPos) / destScale;
    fp->_A = fp->_A * scale / destScale;
    fp->_B = fp->_B * scale / destScale;
}
void  fl_newReferentialAsDelta(FluidPos* fp, float scale, float destScale) {
    fp->_pos = fp->_pos * scale / destScale;
    fp->_A = fp->_A * scale / destScale;
    fp->_B = fp->_B * scale / destScale;
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
void  fl_array_writeTo(const FluidPos *sp, float *f, size_t count) {
    float *end = &f[count];
    while(f < end) {
        *f = fl_pos(sp);
        sp ++;
        f ++;
    }
}
Vector2 fl_array_toVec2(const FluidPos *sp) {
    return (Vector2) {{
        fl_pos(sp), fl_pos(&sp[1]),
    }};
}
Vector3 fl_array_toVec3(const FluidPos *sp) {
    return (Vector3) {{
        fl_pos(sp), fl_pos(&sp[1]), fl_pos(&sp[2]),
    }};
}
Vector3 fl_array_toRealVec3(const FluidPos *sp) {
    return (Vector3) {{
        sp[0]._pos, sp[1]._pos, sp[2]._pos,
    }};
}

// Array de 4 smpos -> Vector4 (i.e. simd_float4, 4 aligned float)
Vector4 fl_array_toVec4(const FluidPos *sp) {
    return (Vector4) {{
        fl_pos(&sp[0]), fl_pos(&sp[1]), fl_pos(&sp[2]), fl_pos(&sp[3]),
    }};
}

float   fl_pos(const FluidPos *sp) {
    float elapsedSec = fl_getElapsedSec_(sp);
    return fl_getDelta_(sp, elapsedSec) + sp->_pos;
}
float   fl_real(const FluidPos *sp) {
    return sp->_pos;
}
bool    fl_isStatic(const FluidPos *fl) {
    return (fl->_flags & _sm_types) == _sm_type_static;
}

void  fld_init(FluidPosWithDrift* fld, float pos, float lambda, bool asAngle) {
    fl_init(&fld->fl, pos, lambda, asAngle);
    fld->drift = 0.f;
}
void  fld_set(FluidPosWithDrift* spd, float pos, float drift) {
    float elapsedSec = fl_getElapsedSec_(&spd->fl);
    Vector2 deltaSlope = fl_getDeltaAndSlope_(&spd->fl, elapsedSec);
    deltaSlope.x += spd->fl._pos + elapsedSec * spd->drift - pos;
    if(spd->fl._flags & _sm_flag_angle)
        deltaSlope.x = float_toNormalizedAngle(deltaSlope.x);
    deltaSlope.y += spd->drift - drift;
    fl_setABT_(&spd->fl, deltaSlope);
    if(spd->fl._flags & _sm_flag_angle)
        spd->fl._pos = float_toNormalizedAngle(pos);
    else
        spd->fl._pos = pos;
    spd->drift = drift;
}
float fld_pos(const FluidPosWithDrift* spd) {
    float elapsedSec = fl_getElapsedSec_(&spd->fl);
    return fl_getDelta_(&spd->fl, elapsedSec)
        + spd->fl._pos + elapsedSec * spd->drift;
}
