//
//  math_smflag.c
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-09.
//

#include "math_smflage.h"
#include <math.h>
#include "../coq_chrono.h"
#include "../utils/util_base.h"


// MARK: - ChronoTiny : un mini chrono de 2 octets -
/// Mini chrono de 2 octets.
/// Lapse max de 2^16 ms, i.e. de -32s à +32s.
/// La sruct est simplement {int16_t time}.
/// Optenir un ChronoTiny que l'on "start" a zero.
static inline int16_t chronotinyE_startedChrono(void) {
    return SFE_Chronos_elapsedMS;
}
/// Optenir un ChronoTiny que l'on "start" a elapsedMS.
static inline int16_t chronotinyE_setToElapsedMS(int16_t elapsedMS) {
    return SFE_Chronos_elapsedMS - elapsedMS;
}
// Temps écoulé... oui, c'est la meme chose que setToElapsedMS ;)
static inline int16_t chronotinyE_elapsedMS(int16_t t) {
    return SFE_Chronos_elapsedMS - t;
}
static inline int16_t chronotinyE_elapsedNextMS(int16_t t) {
    return SFE_Chronos_elapsedNextMS - t;
}

// MARK: - SmoothFlag
enum {
    sm_state_isDown_ =       0x0000,
    sm_state_goingUp_ =      0x2000,
    sm_state_goingDown_ =    0x4000,
    sm_state_goingUpOrDown_ =0x6000,
    sm_state_isUp_ =         0x8000,
    sm_state_flags_ =        0xE000,
    sm_state_maxDelta_ =     0x028F, // 655 ticks -> 32750 ms < 32767.
    sm_state_deltaTmask_ =   0x03FF,
};
#define ST_MIN_MS   6  // Minimum elapsed time
static uint16_t     sm_defTransTimeTicks_ = 10;
#define SF_DELTATMS ((st->_flags & sm_state_deltaTmask_) * 50)

SmoothFlagE SmoothFlagE_new(bool const isOn, uint16_t const transTimeTicksOpt) {
    return (SmoothFlagE) {
        ._flags = (transTimeTicksOpt ? (transTimeTicksOpt > sm_state_maxDelta_ ? sm_state_maxDelta_ : transTimeTicksOpt) : sm_defTransTimeTicks_) 
                | (isOn ? sm_state_isUp_ : sm_state_isDown_),
    };
}

void    SmoothFlagE_setDefaultTransTimeTicks(uint16_t transTimeTicks) {
    if(transTimeTicks > sm_state_maxDelta_) { 
        printwarning("trans time too big, max is 8191 ms.");
        transTimeTicks = sm_state_maxDelta_;
    }
    sm_defTransTimeTicks_ = transTimeTicks;
}


float smoothflagE_setOn(SmoothFlagE *const st) {
    // On verifie les cas du plus probable au moins probable.
    // Callé à chaque frame pour vérifier le on/off de "show".
    float const max = 1.f; // - (float)st->_sub/(float)UINT16_MAX;
    if(st->_flags & sm_state_isUp_)
        return max;
    int16_t elapsedMS = chronotinyE_elapsedMS(st->_t);
    // Going up
    if(st->_flags & sm_state_goingUp_) {
        if(elapsedMS > SF_DELTATMS) {
            st->_flags &= ~sm_state_goingUp_;
            st->_flags |= sm_state_isUp_;
            return max;
        }
    }
    // down -> goingUp ou up 
    else if((st->_flags & sm_state_flags_) == sm_state_isDown_) {
//        if(st->_flags & sm_flag_hard_) {
//            st->_flags |= sm_state_isUp_;
//            return max;
//        }
        st->_flags |= sm_state_goingUp_;
        st->_t = chronotinyE_startedChrono();
        elapsedMS = ST_MIN_MS; // (Min 6ms de temps écoulé)
    }
    // going down -> goingUp 
    else {
        st->_flags &= ~sm_state_goingDown_;
        st->_flags |= sm_state_goingUp_;
        if(elapsedMS < SF_DELTATMS) {
            elapsedMS = SF_DELTATMS - elapsedMS;
            st->_t = chronotinyE_setToElapsedMS(elapsedMS);
        } else {
            st->_t = chronotinyE_startedChrono();
            elapsedMS = ST_MIN_MS;
        }
    }
    // Ici on est going up... (si up déjà sorti)
    float ratio = (float)elapsedMS / (float)SF_DELTATMS;
//    if(st->_flags & sm_flag_poping_)
//        return max * (sm_a_ + sm_b_ * cosf(M_PI * ratio)
//                  + ( 0.5f - sm_a_) * cosf(2.f * M_PI * ratio)
//                  + (-0.5f - sm_b_) * cosf(3.f * M_PI * ratio)); // pippop
    return max * (1.f - cosf(M_PI * ratio)) / 2.f; // smooth
}
float smoothflagE_setOff(SmoothFlagE *const st) {
    if((st->_flags & sm_state_flags_) == sm_state_isDown_)
        return 0.0f;
    int16_t elapsed = chronotinyE_elapsedMS(st->_t);
    if(st->_flags & sm_state_goingDown_) {
        if(elapsed > SF_DELTATMS) {
            st->_flags &= ~sm_state_flags_; // (down)
            return 0.0f;
        }
    }
    // Up 
    else if(st->_flags & sm_state_isUp_) {
        st->_flags &= ~sm_state_flags_; // (down)
//        if(st->_flags & sm_flag_hard_)
//            return 0.0f;
        st->_flags |= sm_state_goingDown_;
        st->_t = chronotinyE_startedChrono();
        elapsed = ST_MIN_MS;
    }
    // Going up
    else {
        st->_flags &= ~sm_state_goingUp_;
        st->_flags |= sm_state_goingDown_;
        if(elapsed < SF_DELTATMS) {
            elapsed = SF_DELTATMS - elapsed;
            st->_t = chronotinyE_setToElapsedMS(elapsed);
        } else {
            st->_t = chronotinyE_startedChrono();
            elapsed = ST_MIN_MS;
        }
    }
    // Ici on est going down... (si down déjà sorti)
    float ratio = (float)elapsed / (float)SF_DELTATMS;
    float const max = 1.f; // - (float)st->_sub/(float)UINT16_MAX;
    return  max * (1.f + cosf(M_PI * ratio)) / 2.f; // (smoothDown)
}
float smoothflagE_value(SmoothFlagE *const st) {
    if((st->_flags & sm_state_flags_) == sm_state_isDown_)
        return 0.0f;
    float const max = 1.f; // - (float)st->_sub/(float)UINT16_MAX;
    if(st->_flags & sm_state_isUp_)
        return max;
    int16_t elapsed = chronotinyE_elapsedMS(st->_t);
    // On est goingUpOrDown...
    if(elapsed < SF_DELTATMS) {
        float ratio = (float)elapsed / (float)SF_DELTATMS;
        if(st->_flags & sm_state_goingUp_) {
//            if(st->_flags & sm_flag_poping_)
//                return max * (sm_a_ + sm_b_ * cosf(M_PI * ratio)
//                    + ( 0.5f - sm_a_) * cosf(2.f * M_PI * ratio)
//                    + (-0.5f - sm_b_) * cosf(3.f * M_PI * ratio)); // pippop
            return max * (1.f - cosf(M_PI * ratio)) / 2.f; // smoothUp
        }
        // (going down)
        return  max * (1.f + cosf(M_PI * ratio)) / 2.f; // (smoothDown)
    }
    // Transition fini : up
    if(st->_flags & sm_state_goingUp_) {
        st->_flags &= ~sm_state_goingUp_;
        st->_flags |= sm_state_isUp_;
        return max;
    }
    // Down
    st->_flags &= ~sm_state_flags_;
    return 0.f;
}
float smoothflagE_valueNext(SmoothFlagE const*const st) {
    if((st->_flags & sm_state_flags_) == sm_state_isDown_)
        return 0.0f;
    float const max = 1.f; // - (float)st->_sub/(float)UINT16_MAX;
    if(st->_flags & sm_state_isUp_)
        return max;
    int16_t elapsed = chronotinyE_elapsedNextMS(st->_t);
    // On est goingUpOrDown...
    if(elapsed < SF_DELTATMS) {
        float ratio = (float)elapsed / (float)SF_DELTATMS;
        if(st->_flags & sm_state_goingUp_) {
            return max * (1.f - cosf(M_PI * ratio)) / 2.f; // smoothUp
        }
        // (going down)
        return  max * (1.f + cosf(M_PI * ratio)) / 2.f; // (smoothDown)
    }
    // Transition fini : up
    if(st->_flags & sm_state_goingUp_) {
        return max;
    }
    // Down
    return 0.f;
}

//void  smoothflag_setMaxValue(SmoothFlag *st, float newMax) {
//    st->_sub = UINT16_MAX - (uint16_t)roundf(fminf(1.f, fmaxf(0.f, newMax))*(float)UINT16_MAX);
//}
//void  smoothflag_setOptions(SmoothFlag *st, bool isHard, bool isPoping) {
//    if(isHard)
//        st->_flags |= sm_flag_hard_;
//    else
//        st->_flags &= ~sm_flag_hard_;
//    if(isPoping)
//        st->_flags |=  sm_flag_poping_;
//    else
//        st->_flags &= ~sm_flag_poping_;
//}
//void  smoothflag_setDeltaT(SmoothFlag *st, int16_t deltaT) {
//    st->_D = deltaT;
//}
