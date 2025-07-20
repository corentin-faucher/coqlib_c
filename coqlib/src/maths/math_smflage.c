//
//  math_smflag.c
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-09.
//

#include "math_smflage.h"
#include <math.h>
#include "../utils/util_chrono.h"
#include "../utils/util_base.h"

#define SFE_elapsedTicks      (int16_t)ChronosEvent.tick
#define SFE_elapsedNextTicks ((int16_t)ChronosEvent.tick + 1)

// MARK: - ChronoTiny : un mini chrono de 2 octets -
/// Mini chrono de 2 octets.
/// Lapse max de 2^16 ms, i.e. de -32s à +32s.
/// La sruct est simplement {int16_t time}.
/// Optenir un ChronoTiny que l'on "start" a zero.
static inline int16_t chronotinyE_startedChrono(void) {
    return SFE_elapsedTicks;
}
/// Optenir un ChronoTiny que l'on "start" a elapsedMS.
static inline int16_t chronotinyE_setToElapsedTicks(int16_t elapsedTicks) {
    return SFE_elapsedTicks - elapsedTicks;
}
// Temps écoulé... oui, c'est la meme chose que setToElapsedMS ;)
static inline int16_t chronotinyE_elapsedTicks(int16_t t) {
    return SFE_elapsedTicks - t;
}
static inline int16_t chronotinyE_elapsedNextTicks(int16_t t) {
    return SFE_elapsedNextTicks - t;
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
static uint16_t     sm_defTransTimeTicks_ = 10;

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
    float const max = 1.f;
    if(st->_flags & sm_state_isUp_)
        return max;
    int16_t elapsedTicks = chronotinyE_elapsedTicks(st->_t);
    int16_t const deltaTicks = (st->_flags & sm_state_deltaTmask_);
    // Going up
    if(st->_flags & sm_state_goingUp_) {
        if(elapsedTicks > deltaTicks) {
            st->_flags &= ~sm_state_goingUp_;
            st->_flags |= sm_state_isUp_;
            return max;
        }
    }
    // down -> goingUp ou up 
    else if((st->_flags & sm_state_flags_) == sm_state_isDown_) {
        st->_flags |= sm_state_goingUp_;
        st->_t = chronotinyE_startedChrono();
        elapsedTicks = 1; // min
    }
    // going down -> goingUp 
    else {
        st->_flags &= ~sm_state_goingDown_;
        st->_flags |= sm_state_goingUp_;
        if(elapsedTicks < deltaTicks) {
            elapsedTicks = deltaTicks - elapsedTicks;
            st->_t = chronotinyE_setToElapsedTicks(elapsedTicks);
        } else {
            st->_t = chronotinyE_startedChrono();
            elapsedTicks = 1;
        }
    }
    // Ici on est going up... (si up déjà sorti)
    float ratio = (float)elapsedTicks / (float)deltaTicks;
    return max * (1.f - cosf(M_PI * ratio)) / 2.f; // smooth
}
float smoothflagE_setOff(SmoothFlagE *const st) {
    if((st->_flags & sm_state_flags_) == sm_state_isDown_)
        return 0.0f;
    int16_t elapsed = chronotinyE_elapsedTicks(st->_t);
    int16_t const deltaTicks = (st->_flags & sm_state_deltaTmask_);
    if(st->_flags & sm_state_goingDown_) {
        if(elapsed > deltaTicks) {
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
        elapsed = 1;
    }
    // Going up
    else {
        st->_flags &= ~sm_state_goingUp_;
        st->_flags |= sm_state_goingDown_;
        if(elapsed < deltaTicks) {
            elapsed = deltaTicks - elapsed;
            st->_t = chronotinyE_setToElapsedTicks(elapsed);
        } else {
            st->_t = chronotinyE_startedChrono();
            elapsed = 1;
        }
    }
    // Ici on est going down... (si down déjà sorti)
    float ratio = (float)elapsed / (float)deltaTicks;
    float const max = 1.f;
    return  max * (1.f + cosf(M_PI * ratio)) / 2.f; // (smoothDown)
}
float smoothflagE_value(SmoothFlagE *const st) {
    if((st->_flags & sm_state_flags_) == sm_state_isDown_)
        return 0.0f;
    float const max = 1.f; // - (float)st->_sub/(float)UINT16_MAX;
    if(st->_flags & sm_state_isUp_)
        return max;
    int16_t elapsed = chronotinyE_elapsedTicks(st->_t);
    int16_t const deltaTicks = (st->_flags & sm_state_deltaTmask_);
    // On est goingUpOrDown...
    if(elapsed < deltaTicks) {
        float ratio = (float)elapsed / (float)deltaTicks;
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
    int16_t elapsed = chronotinyE_elapsedNextTicks(st->_t);
    int16_t const deltaTicks = (st->_flags & sm_state_deltaTmask_);
    // On est goingUpOrDown...
    if(elapsed < deltaTicks) {
        float ratio = (float)elapsed / (float)deltaTicks;
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
void  smoothflagE_setTransitionTime(SmoothFlagE*const sf, uint16_t const newTransTimeTicks) {
    sf->_flags = (sf->_flags & sm_state_flags_) | 
        (newTransTimeTicks > sm_state_maxDelta_ ? sm_state_maxDelta_ : newTransTimeTicks);
}
