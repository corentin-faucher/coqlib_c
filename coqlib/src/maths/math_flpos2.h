//
//  _math_flpos2.h
//  Version simplifiée de flpos. Pas multi-thread.
//  Juste oscil.-amorti, juste pour la thread event, lambda const = 10.
//  Ne prend qu'un `int128` d'espace (4 floats).
//
//  Created by Corentin Faucher on 2025-01-08.
//
#ifndef COQ_MATH_FL2POS_H
#define COQ_MATH_FL2POS_H

#include <stdint.h>
#include "math_base.h"
#include "../coq_chrono.h"

// Vitesse de convergence.
#define FL2_LAMBDA 10.f // Test lambda constant. Si paramètre changer pour `f->lambda`.

#define FL2_Chrono_elapsedMS  (uint32_t)ChronosEvent.app_elapsedMS
#define FL2_elapsed0Sec (float)(FL2_Chrono_elapsedMS - f->time)*SEC_PER_MS
#define FL2_elapsed1Sec (float)(FL2_Chrono_elapsedMS - f->time + (uint32_t)ChronosEvent.deltaTMS)*SEC_PER_MS

typedef struct FluidPosE {
    float    pos;    // Vers ou on tend.
    float    A, B;   // ~pente et ecart initiale.
//    float    lambda; // ~vitesse de convergence.
    uint32_t time;   // Temps (event) de set.
} FluidPosE;

static inline void  fpE_init(FluidPosE *const f, float const pos) {
    *f = (FluidPosE) { .pos = pos, };
}
/// Setter "smooth" de FluidPos. La position va tendre vers `pos`.
void  fpE_set(FluidPosE *fl, float pos);
void  fpE_setAsAngle(FluidPosE *const f, float theta);

/// Position estimee à l'instant présent
static inline float fpE_pos(FluidPosE const*const f) {
    return expf(-FL2_LAMBDA*FL2_elapsed0Sec) * (f->A + f->B*FL2_elapsed0Sec) + f->pos;
}
/// Position estimee au prochain tic (dans +delta t).
static inline float fpE_posAtNextTick(FluidPosE const*const f) {
    return expf(-FL2_LAMBDA*FL2_elapsed1Sec) * (f->A + f->B*FL2_elapsed1Sec) + f->pos;
}

#endif
