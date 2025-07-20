//
//  smpos.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include "math_flpos.h"

#include <string.h>
#include "../utils/util_chrono.h"

#define _sm_type_static     0x0000
#define _sm_type_oscAmorti  0x0001
#define _sm_type_amortiCrit 0x0002
#define _sm_type_surAmorti  0x0003
#define _sm_types           0x0003
#define _sm_flag_angle      0x0004


// Prendre le temps ChronoRender ou ChronoApp comme ref ?
#define Chrono_inEvent_elapsedMS  (uint32_t)ChronosEvent.render_elapsedMS
#define Chrono_inRender_elapsedMS (uint32_t)ChronosRender.render_elapsedMS
//#define Chrono_inEvent_elapsedMS  (uint32_t)ChronoEvent_chronoApp_elapsedMS
//#define Chrono_inRender_elapsedMS (uint32_t)ChronoRender_chronoApp_elapsedMS
static inline float coretime_inEvent_toElapsedSec_(uint32_t const coreTime) {
    return (float)(Chrono_inEvent_elapsedMS - coreTime) * SEC_PER_MS;
}
static inline float coretime_inRender_toElapsedSec_(uint32_t const coreTime) {
    return (float)(Chrono_inRender_elapsedMS - coreTime) * SEC_PER_MS;
}

Vector2 fl_getDeltaAndSlope_(const FluidPos fl, float const elapsedSec) {
    if((fl._p._flags & _sm_types) == _sm_type_static) return vector2_zeros;
    if((fl._p._flags & _sm_types) == _sm_type_amortiCrit)
        return (Vector2) {{
            expf(-fl._p._lambda * elapsedSec) * (fl._c.A + fl._c.B * elapsedSec),
            expf(-fl._p._lambda * elapsedSec)
            * (fl._c.B*(1 - fl._p._lambda*elapsedSec) - fl._p._lambda*fl._c.A),
        }};
    if((fl._p._flags & _sm_types) == _sm_type_surAmorti)
        return (Vector2) {{
              fl._c.A * expf(-fl._p._lambda * elapsedSec)
            + fl._c.B * expf(-fl._p._beta   * elapsedSec),
             -fl._p._lambda * fl._c.A * expf(-fl._p._lambda * elapsedSec)
            - fl._p._beta   * fl._c.B * expf(-fl._p._beta   * elapsedSec),
        }};
    // _sm_type_oscAmorti
    return (Vector2) {{
        expf(-fl._p._lambda * elapsedSec) *
         (fl._c.A * cosf(fl._p._beta * elapsedSec) + fl._c.B * sinf(fl._p._beta * elapsedSec)),
        expf(-fl._p._lambda * elapsedSec)
        * ( cosf(fl._p._beta * elapsedSec) * (fl._p._beta*fl._c.B   - fl._p._lambda*fl._c.A)
          - sinf(fl._p._beta * elapsedSec) * (fl._p._lambda*fl._c.B + fl._p._beta*fl._c.A) ),
    }};
}
FluidPosParams_ fl_getNewParams_(const float gamma, const float k, const uint32_t oldFlags, const float def) {
    FluidPosParams_ p;
    p.def = def;
    p._flags = oldFlags & (~_sm_types);
    if(gamma == 0 && k == 0) {
        p._flags |= _sm_type_static;
        p._lambda = 0.f;
        p._beta = 0.f;
        return p;
    }
    const float discr = gamma * gamma - 4.f * k;
    if(discr > 0.001f) {
        p._flags |= _sm_type_surAmorti;
        p._lambda = gamma + sqrtf(discr) / 2.f;
        p._beta =   gamma - sqrtf(discr) / 2.f;
        return p;
    }
    if(discr < -0.001f) {
        p._flags |= _sm_type_oscAmorti;
        p._lambda = gamma / 2.f;
        p._beta = sqrtf(-discr);
        return p;
    }
    p._flags |= _sm_type_amortiCrit;
    p._lambda = gamma / 2.f;
    p._beta =   gamma / 2.f;
    return p;
}
FluidPosCore_   fl_inEvent_evalNewCore_(const FluidPosParams_ p, Vector2 const ds, float const pos) {
    FluidPosCore_ c;
    c.pos = pos;
    switch(p._flags & _sm_types) {
        case _sm_type_oscAmorti:
            c.A = ds.x;
            c.B = (ds.y + p._lambda * c.A) / p._beta;
            break;
        case _sm_type_amortiCrit:
            c.A = ds.x;
            c.B = ds.y + p._lambda * c.A;
            break;
        case _sm_type_surAmorti:
            c.A = (p._beta * ds.x + ds.y) / (p._beta - p._lambda);
            c.B = ds.x - c.A;
            break;
        default: // _sm_type_static:
            c.A = 0.f; c.B = 0.f;
            break;
    }
    c.time = Chrono_inEvent_elapsedMS;
    return c;
}

void  fl_updateToConstants(volatile FluidPos *sp, float gamma, float k) {
    // 1. Nouveau paramètres de la courbe
    FluidPosParams_ newP = fl_getNewParams_(gamma, k, sp->_flags, sp->def);
    // 2. Pos et temps actuel. 
    float elapsedSec = coretime_inEvent_toElapsedSec_(sp->_c.time);
    Vector2 deltaSlope = fl_getDeltaAndSlope_(*sp, elapsedSec);
    // 3. Réévaluer a/b pour nouveau lambda/beta et reset time.
    FluidPosCore_ newC = fl_inEvent_evalNewCore_(newP, deltaSlope, sp->_c.pos);
    // 4. Swap. 
    sp->_p.v = newP.v;
    sp->_c.v = newC.v;
}
void  fl_updateToLambda(FluidPos *fl, float lambda) {
    fl_updateToConstants(fl, 2*lambda, lambda * lambda);
}

void  fl_init(FluidPos* sp, float pos, float lambda, bool asAngle) {
    sp->_c = (FluidPosCore_) {
        .pos = asAngle ? float_toNormalizedAngle(pos) : pos,
    };
    sp->_p = fl_getNewParams_(2*lambda, lambda * lambda, asAngle ? _sm_flag_angle : 0, pos);
}
void  fl_initGammaK(FluidPos *sp, float pos, float gamma, float k, bool asAngle) {
    sp->_c = (FluidPosCore_) {
        .pos = asAngle ? float_toNormalizedAngle(pos) : pos,
    };
    sp->_p = fl_getNewParams_(gamma, k, asAngle ? _sm_flag_angle : 0, pos);
}
void  fl_set(FluidPos* const sp, float const pos) {
    float const elapsedSec = coretime_inEvent_toElapsedSec_(sp->_c.time);
    Vector2 deltaSlope = fl_getDeltaAndSlope_(*sp, elapsedSec);
    deltaSlope.x += sp->_c.pos - pos;
    if(sp->_flags & _sm_flag_angle)
        deltaSlope.x = float_toNormalizedAngle(deltaSlope.x);
    // Swap
    sp->_c.v = fl_inEvent_evalNewCore_(sp->_p, deltaSlope, (sp->_flags & _sm_flag_angle) ? float_toNormalizedAngle(pos) : pos).v;
}
void  fl_fix(FluidPos* sp, float pos) {
    FluidPosCore_ newC = {}; // Set à zéro.
    newC.pos = (sp->_flags & _sm_flag_angle) ? float_toNormalizedAngle(pos) : pos;
    // Swap
    sp->_c.v = newC.v;
}
/** Changement de référentiel quelconques (avec positions et scales absolues). */
void  fl_newReferential(FluidPos* fp, float pos, float destPos, float scale, float destScale) {
    FluidPosCore_ c = {
        .pos = (pos - destPos) / destScale,
        .A = fp->_c.A * scale / destScale,
        .B = fp->_c.B * scale / destScale,
        .time = fp->_c.time,
    };
    // Swap
    fp->_c.v = c.v;
}
void  fl_newReferentialAsDelta(FluidPos* fp, float scale, float destScale) {
    FluidPosCore_ c = {
        .pos = fp->_c.pos * scale / destScale,
        .A = fp->_c.A * scale / destScale,
        .B = fp->_c.B * scale / destScale,
        .time = fp->_c.time,
    };
    // Swap
    fp->_c.v = c.v;
}
void  fl_referentialOut(FluidPos *const fp, float const refX, float const refScale) {
    FluidPosCore_ c = {
        .pos = fp->_c.pos * refScale + refX,
        .A = fp->_c.A * refScale,
        .B = fp->_c.B * refScale,
        .time = fp->_c.time,
    };
    // Swap
    fp->_c.v = c.v;
}
void  fl_referentialOutAsDelta(FluidPos *const fp, float const refScale) {
    FluidPosCore_ c = {
        .pos = fp->_c.pos * refScale,
        .A = fp->_c.A * refScale,
        .B = fp->_c.B * refScale,
        .time = fp->_c.time,
    };
    // Swap
    fp->_c.v = c.v;
}

// Les convenience du set...
void  fl_setRelToDef(FluidPos *sp, float shift) {
    fl_set(sp, sp->def + shift);
}
void  fl_move(FluidPos *sp, float shift) {
    fl_set(sp, sp->_c.pos + shift);
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
    fl_set(sp, sp->_c.pos - delta);
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
void  fl_array_fix(FluidPos *sp, float const* f, size_t count) {
    const float *end = &f[count];
    while(f < end) {
        fl_fix(sp, *f);
        sp ++;
        f ++;
    }
}

float fl_evalPos(volatile FluidPos const* fp) {
    FluidPosCore_ c; 
    FluidPosParams_ p;
    c.v = fp->_c.v;
    p.v = fp->_p.v;
    if((p._flags & _sm_types) == _sm_type_static) return c.pos;
    float const elapsedSec = coretime_inRender_toElapsedSec_(c.time);
    if((p._flags & _sm_types) == _sm_type_amortiCrit)
        return expf(-p._lambda * elapsedSec) * (c.A + c.B * elapsedSec) + c.pos;
    if((p._flags & _sm_types) == _sm_type_surAmorti)
        return c.A * expf(-p._lambda * elapsedSec)
             + c.B * expf(-p._beta   * elapsedSec) + c.pos;
    // _sm_type_oscAmorti
    return expf(-p._lambda * elapsedSec) *
         (c.A * cosf(p._beta * elapsedSec) + c.B * sinf(p._beta * elapsedSec)) + c.pos;
}

void  fl_array_writeTo(FluidPos const* sp, float *f, size_t const count) {
    float *const end = &f[count];
    for(; f < end; f++, sp++) {
        *f = fl_evalPos(sp);
    }
}
Vector2 fl_array_toVec2(volatile const FluidPos *sp) {
    return (Vector2) {{
        fl_evalPos(&sp[0]), fl_evalPos(&sp[1]),
    }};
}
Vector3 fl_array_toVec3(volatile const FluidPos *sp) {
    return (Vector3) {{
        fl_evalPos(&sp[0]), fl_evalPos(&sp[1]), fl_evalPos(&sp[2]),
    }};
}
Vector3 fl_array_toRealVec3(const FluidPos *sp) {
    return (Vector3) {{
        sp[0]._c.pos, sp[1]._c.pos, sp[2]._c.pos,
    }};
}

// Array de 4 smpos -> Vector4 (i.e. simd_float4, 4 aligned float)
Vector4 fl_array_toVec4(const FluidPos *sp) {
    return (Vector4) {{
        fl_evalPos(&sp[0]), fl_evalPos(&sp[1]), fl_evalPos(&sp[2]), fl_evalPos(&sp[3]),
    }};
}

float   fl_real(const FluidPos *sp) {
    return sp->_c.pos;
}
bool    fl_isStatic(const FluidPos *fl) {
    return (fl->_flags & _sm_types) == _sm_type_static;
}

void  fld_init(FluidPosWithDrift* fld, float pos, float lambda, bool asAngle) {
    fl_init(&fld->fp, pos, lambda, asAngle);
    fld->drift = 0.f;
}
void  fld_set(FluidPosWithDrift* spd, float pos, float drift) {
    float elapsedSec = coretime_inEvent_toElapsedSec_(spd->fp._c.time);
    Vector2 deltaSlope = fl_getDeltaAndSlope_(spd->fp, elapsedSec);
    deltaSlope.x += spd->fp._c.pos + elapsedSec * spd->drift - pos;
    if(spd->fp._flags & _sm_flag_angle)
        deltaSlope.x = float_toNormalizedAngle(deltaSlope.x);
    deltaSlope.y += spd->drift - drift;
    
    spd->fp._c.v = fl_inEvent_evalNewCore_(spd->fp._p, deltaSlope, 
            (spd->fp._flags & _sm_flag_angle) ? float_toNormalizedAngle(pos) : pos).v;
    spd->drift = drift;
}
float fld_evalPos(FluidPosWithDrift const*const fpd) {

    FluidPosCore_ c; 
    FluidPosParams_ p;
    c.v = fpd->fp._c.v;
    float drift = fpd->drift;
    p.v = fpd->fp._p.v;
    float const elapsedSec = coretime_inRender_toElapsedSec_(c.time);
    if((p._flags & _sm_types) == _sm_type_static) 
        return c.pos + elapsedSec * drift;
    if((p._flags & _sm_types) == _sm_type_amortiCrit)
        return expf(-p._lambda * elapsedSec) * (c.A + c.B * elapsedSec) 
             + c.pos + elapsedSec * drift;
    if((p._flags & _sm_types) == _sm_type_surAmorti)
        return c.A * expf(-p._lambda * elapsedSec)
             + c.B * expf(-p._beta   * elapsedSec)
             + c.pos + elapsedSec * drift;
    // _sm_type_oscAmorti
    return expf(-p._lambda * elapsedSec) *
                (c.A * cosf(p._beta * elapsedSec) + c.B * sinf(p._beta * elapsedSec))
         + c.pos + elapsedSec * drift;
}
