//
//  smtrans.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include "math_smtrans.h"

//typedef struct _SmTrans {
//    ChronoTiny _t;
//    ChronoTiny _D;
//    uint8_t _state;  // On est dans un etat a la fois.
//    uint8_t _flags;  // On peut avoir plusieurs flags a la fois.
//} SmTrans;

enum {
    sm_state_isDown_ =       0,
    sm_state_goingUp_ =      1,
    sm_state_goingDown_ =    2,
    sm_state_goingUpOrDown_ = 3,
    sm_state_isUp_ =         4,
    sm_state_flags_ =        7,
    sm_flag_poping_ =        8,
    sm_flag_hard_ =         16,
};

static ChronoTiny   sm_defTransTime_ = 500;
static float        sm_a_ =     0.75 + (0.2) * 0.2;  // (pop factor est de 0.2 par défaut)
static float        sm_b_ =    -0.43 + (0.2) * 0.43;

// Valeur "smooth" isOn, entre 0.f et 1.f.
float sm_value_(const SmTrans st) {
    if((st._flags & sm_state_flags_) == sm_state_isDown_)
        return 0.f;
    float max = 1.f - (float)st._sub/(float)UINT16_MAX;
    if(st._flags & sm_state_isUp_)
        return max;
    float ratio = (float)chronotiny_elapsedMS(st._t) / (float)st._D;
    if(st._flags & sm_state_goingDown_)
        return  max * (1.f + cosf(M_PI * ratio)) / 2.f; // (smoothDown)
    // Sinon, il reste going up.
    if(st._flags & sm_flag_poping_)
        return max * (sm_a_ + sm_b_ * cosf(M_PI * ratio)
                  + ( 0.5f - sm_a_) * cosf(2.f * M_PI * ratio)
                  + (-0.5f - sm_b_) * cosf(3.f * M_PI * ratio)); // pippop
    return max * (1.f - cosf(M_PI * ratio)) / 2.f; // smooth
}

void    SmTrans_setPopFactor(float popFactor) {
    sm_a_ =  0.75f + popFactor * 0.20f;
    sm_b_ = -0.43f + popFactor * 0.43f;
}
void    SmTrans_setDefaultTransTime(ChronoTiny transTime) {
    sm_defTransTime_ = transTime;
}

void  smtrans_init(SmTrans *st) {
    *st = (SmTrans){ 0, sm_defTransTime_, 0, 0 };
}
bool  smtrans_isActive(SmTrans const st) {
    return st._flags & sm_state_flags_;  // Car sm_state_isDown == 0.
}
float smtrans_value(SmTrans *st) {
    if(st->_flags & sm_state_goingUpOrDown_) if(chronotiny_elapsedMS(st->_t) > st->_D) {
        if(st->_flags & sm_state_goingUp_)
            st->_flags |= sm_state_isUp_;
        st->_flags &= ~sm_state_goingUpOrDown_;
    }
    return sm_value_(*st);
}
float smtrans_setAndGetValue(SmTrans *st, bool isOn) {
    smtrans_setIsOn(st, isOn);
    return sm_value_(*st);
}
void  smtrans_setIsOn(SmTrans *st, bool isOn) {
    if(isOn) {
        // On verifie les cas du plus probable au moins probable.
        if(st->_flags & sm_state_isUp_)
            return;
        int16_t elapsed = chronotiny_elapsedMS(st->_t);
        if(st->_flags & sm_state_goingUp_) {
            if(elapsed > st->_D) {
                st->_flags &= ~sm_state_goingUp_;
                st->_flags |= sm_state_isUp_;
            }
            return;
        }
        if((st->_flags & sm_state_flags_) == sm_state_isDown_) {
            if(st->_flags & sm_flag_hard_)
                st->_flags |= sm_state_isUp_;
            else {
                st->_flags |= sm_state_goingUp_;
                st->_t = chronotiny_startedChrono();
            }
            return;
        }
        // sm_state_goingDown
        st->_flags &= ~sm_state_goingDown_;
        st->_flags |= sm_state_goingUp_;
        if(elapsed < st->_D)
            st->_t = chronotiny_elapsedChrono(st->_D - elapsed);
        else
            st->_t = chronotiny_startedChrono();
        return;
    }
    // Cas isOn == false
    if((st->_flags & sm_state_flags_) == sm_state_isDown_)
        return;
    ChronoTiny elapsed = chronotiny_elapsedMS(st->_t);
    if(st->_flags & sm_state_goingDown_) {
        if(elapsed > st->_D)
            st->_flags &= ~sm_state_flags_; // (down)
        return;
    }
    if(st->_flags & sm_state_isUp_) {
        st->_flags &= ~sm_state_flags_; // (down)
        if(!(st->_flags & sm_flag_hard_)) {
            st->_flags |= sm_state_goingDown_;
            st->_t = chronotiny_startedChrono();
        }
        return;
    }
    // sm_state_goingUp
    st->_flags &= ~sm_state_goingUp_;
    st->_flags |= sm_state_goingDown_;
    if(elapsed < st->_D)
        st->_t = chronotiny_elapsedChrono(st->_D - elapsed);
    else
        st->_t = chronotiny_startedChrono();
}
void  smtrans_fixIsOn(SmTrans *st, bool isOn) {
    st->_flags &= ~sm_state_flags_;
    if(isOn) st->_flags |= sm_state_isUp_;
}
void  smtrans_setMaxValue(SmTrans *st, float newMax) {
    st->_sub = UINT16_MAX - (uint16_t)roundf(fminf(1.f, fmaxf(0.f, newMax))*(float)UINT16_MAX);
}
void  smtrans_setOptions(SmTrans *st, bool isHard, bool isPoping) {
    if(isHard)
        st->_flags |= sm_flag_hard_;
    else
        st->_flags &= ~sm_flag_hard_;
    if(isPoping)
        st->_flags |=  sm_flag_poping_;
    else
        st->_flags &= ~sm_flag_poping_;
}

void  smtrans_setDeltaT(SmTrans *st, ChronoTiny deltaT) {
    st->_D = deltaT;
}
