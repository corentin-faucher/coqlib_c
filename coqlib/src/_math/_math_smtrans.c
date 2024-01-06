//
//  smtrans.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include "_math/_math_smtrans.h"

//typedef struct _SmTrans {
//    ChronoTiny _t;
//    ChronoTiny _D;
//    uint8_t _state;  // On est dans un etat a la fois.
//    uint8_t _flags;  // On peut avoir plusieurs flags a la fois.
//} SmTrans;

const static uint8_t _sm_flag_poping = 1;
const static uint8_t _sm_flag_hard = 2;
const static uint8_t _sm_flag_semi = 4;

const static uint8_t _sm_state_isDown = 0;
const static uint8_t _sm_state_goingUp = 1;
const static uint8_t _sm_state_goingDown = 2;
const static uint8_t _sm_state_isUp = 4;

static float        _sm_semiFactor =   0.4f;
static ChronoTiny   _sm_defTransTime = 500;
static float        _sm_a =     0.75 + (0.2) * 0.2;  // (pop factor est de 0.2 par défaut)
static float        _sm_b =    -0.43 + (0.2) * 0.43;

// Valeur "smooth" isOn, entre 0.f et 1.f.
float _sm_isOnSmooth(const SmTrans st) {
    if(st._state == _sm_state_isDown)
        return 0.f;
    float max = (st._flags & _sm_flag_semi) ? _sm_semiFactor : 1.f;
    if(st._state == _sm_state_isUp)
        return max;
    float ratio = (float)chronotiny_elapsedMS(st._t) / (float)st._D;
    if(st._state == _sm_state_goingDown)
        return  max * (1.f + cosf(M_PI * ratio)) / 2.f; // (smoothDown)
    // _sm_state_goingUp
    if(st._flags & _sm_flag_poping)
        return max * (_sm_a + _sm_b * cosf(M_PI * ratio)
                  + ( 0.5f - _sm_a) * cosf(2.f * M_PI * ratio)
                  + (-0.5f - _sm_b) * cosf(3.f * M_PI * ratio)); // pippop
    return max * (1.f - cosf(M_PI * ratio)) / 2.f; // smooth
}

void    SmTrans_setPopFactor(float popFactor) {
    _sm_a =  0.75f + popFactor * 0.20f;
    _sm_b = -0.43f + popFactor * 0.43f;
}
void    SmTrans_setSemiFactor(float semiFactor) {
    _sm_semiFactor = semiFactor;
}
void    SmTrans_setTransTime(ChronoTiny transTime) {
    _sm_defTransTime = transTime;
}

void  smtrans_init(SmTrans *st) {
    *st = (SmTrans){ 0, _sm_defTransTime, 0, 0 };
}
int   smtrans_isActive(SmTrans st) {
    return st._state;  // Car _sm_state_isDown == 0.
}
float smtrans_isOnSmooth(SmTrans *st) {
    if(st->_state & (_sm_state_goingUp|_sm_state_goingDown)) {
        if(chronotiny_elapsedMS(st->_t) > st->_D)
            st->_state = (st->_state == _sm_state_goingUp) ?
            _sm_state_isUp : _sm_state_isDown;
    }
    return _sm_isOnSmooth(*st);
}
float smtrans_setAndGetIsOnSmooth(SmTrans *st, int isOn) {
    smtrans_setIsOn(st, isOn);
    return _sm_isOnSmooth(*st);
}
void  smtrans_setIsOn(SmTrans *st, int isOn) {
    if(isOn) {
        // On verifie les cas du plus probable au moins probable.
        if(st->_state == _sm_state_isUp)
            return;
        int16_t elapsed = chronotiny_elapsedMS(st->_t);
        if(st->_state == _sm_state_goingUp) {
            if(elapsed > st->_D)
                st->_state = _sm_state_isUp;
            return;
        }
        if(st->_state == _sm_state_isDown) {
            if(st->_flags & _sm_flag_hard)
                st->_state = _sm_state_isUp;
            else {
                st->_state = _sm_state_goingUp;
                st->_t = chronotiny_startedChrono();
            }
            return;
        }
        // _sm_state_goingDown
        st->_state = _sm_state_goingUp;
        if(elapsed < st->_D)
            st->_t = chronotiny_elapsedChrono(st->_D - elapsed);
        else
            st->_t = chronotiny_startedChrono();
        return;
    }
    // isOn == false
    if(st->_state == _sm_state_isDown)
        return;
    ChronoTiny elapsed = chronotiny_elapsedMS(st->_t);
    if(st->_state == _sm_state_goingDown) {
        if(elapsed > st->_D)
            st->_state = _sm_state_isDown;
        return;
    }
    if(st->_state == _sm_state_isUp) {
        if(st->_flags & _sm_flag_hard)
            st->_state = _sm_state_isDown;
        else {
            st->_state = _sm_state_goingDown;
            st->_t = chronotiny_startedChrono();
        }
        return;
    }
    // _sm_state_goingUp
    st->_state = _sm_state_goingDown;
    if(elapsed < st->_D)
        st->_t = chronotiny_elapsedChrono(st->_D - elapsed);
    else
        st->_t = chronotiny_startedChrono();
}
void  smtrans_setIsOnHard(SmTrans *st, int isOn) {
    st->_state = isOn ? _sm_state_isUp : _sm_state_isDown;
}
void  smtrans_setOptions(SmTrans *st, int isHard, int isPoping) {
    if(isHard)
        st->_flags |= _sm_flag_hard;
    else
        st->_flags &= ~_sm_flag_hard;
    if(isPoping)
        st->_flags |=  _sm_flag_poping;
    else
        st->_flags &= ~_sm_flag_poping;
}
