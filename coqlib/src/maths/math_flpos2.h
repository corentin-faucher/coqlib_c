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
#include "../utils/util_chrono.h"

// Vitesse de convergence.
#define FPE_LAMBDA 0.25f // Test lambda constant. Si paramètre changer pour `f->lambda`.

#define FPE_globalTicks      ChronosEvent.tick
#define FPE_elapsedTicks     (float)(FPE_globalTicks - f->time)
#define FPE_elapsedNextTicks (float)(FPE_globalTicks + 1 - f->time)


typedef struct FluidPosE {
    float    pos;    // Vers ou on tend.
    float    A, B;   // ~pente et ecart initiale.
//    float    lambda; // ~vitesse de convergence.
    uint32_t time;   // Temps (event) de set.
} FluidPosE;

static inline void  fpE_init(FluidPosE *const f, float const pos) {
    *f = (FluidPosE) { .pos = pos, };
}
static inline FluidPosE FluidPosE_new(float const pos) {
    return (FluidPosE) { .pos = pos };
}
/// Setter "smooth" de FluidPos. La position va tendre vers `pos`.
void  fpE_set(FluidPosE *fl, float pos);
void  fpE_setAsAngle(FluidPosE *const f, float theta);

/// Position estimee à l'instant présent
static inline float fpE_pos(FluidPosE const*const f) {
    return expf(-FPE_LAMBDA*FPE_elapsedTicks) * (f->A + f->B*FPE_elapsedTicks) + f->pos;
}
/// Position estimee au prochain tic (dans +delta t).
static inline float fpE_posAtNextTick(FluidPosE const*const f) {
    return expf(-FPE_LAMBDA*FPE_elapsedNextTicks) * (f->A + f->B*FPE_elapsedNextTicks) + f->pos;
}

#endif
