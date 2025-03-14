//
//  math_flpos2.c
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-08.
//
#include "math_flpos2.h"

//static inline float coretime_inEvent_toElapsedSec_(uint32_t const coreTime) {
//    return (float)(Chrono_inEvent_elapsedMS - coreTime) * SEC_PER_MS;
//}

static inline float fl2_delta_(FluidPosE const*const f) {
    return expf(-FL2_LAMBDA*FL2_elapsed0Sec) * (f->A + f->B*FL2_elapsed0Sec);
}
static inline float fl2_slope_(FluidPosE const*const f) {
    return expf(-FL2_LAMBDA*FL2_elapsed0Sec)
        * (f->B*(1 - FL2_LAMBDA*FL2_elapsed0Sec) - FL2_LAMBDA*f->A);
}

//void  fl2_updateToLambda(FluidPos2 *const f, float const lambda) {
//    // Réévaluer a/b pour nouveau lambda et reset time.
//    float const delta = fl2_delta_(f);
//    f->A = delta;
//    f->B = fl2_slope_(f) + FL2_LAMBDA * delta;
////    f->lambda = lambda;
//    f->time = FL2_Chrono_elapsedMS;
//}

void  fpE_set(FluidPosE *const f, float const pos) {
    float const delta = fl2_delta_(f) + f->pos - pos;
    float const slope = fl2_slope_(f);
    f->pos = pos;
    f->A = delta;
    f->B = slope + FL2_LAMBDA*delta;
    f->time = FL2_Chrono_elapsedMS;
}
void fpE_setAsAngle(FluidPosE *const f, float const pos) {
    float const delta = float_toNormalizedAngle(fl2_delta_(f) + f->pos - pos);
    float const slope = fl2_slope_(f);
    f->pos = float_toNormalizedAngle(pos);
    f->A = delta;
    f->B = slope + FL2_LAMBDA*delta;
    f->time = FL2_Chrono_elapsedMS;
}

//void  fpE_fix(FluidPosE *const f, float const pos) {
//    f->pos = pos;
//    f->A = 0;
//    f->B = 0;
//}
