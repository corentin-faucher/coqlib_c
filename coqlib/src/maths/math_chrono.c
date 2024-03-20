//
//  chrono.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-13.
//

#include "maths/math_chrono.h"

#include <stdio.h>

/*-- ChronoRender, le chrono pour le rendering. --*/
// variable "prives"
static       int     _CR_isPaused = 0;
int64_t              CR_elapsedMS_ = 0;
static       int64_t _CR_elapsedAngleMS = 0;
static       int64_t _CR_touchTimeMS = 0;
static const int64_t _CR_angleLoopTimeMS = (int64_t)(24000 * M_PI);
static       int64_t _CR_sleepTime = 16000;

int64_t Chrono_UpdateDeltaTMS = 50;
// Methodes
void    ChronoRender_update(int64_t deltaTMS) {
    if(_CR_isPaused) return;
    CR_elapsedMS_ += deltaTMS;
    _CR_elapsedAngleMS += deltaTMS;
    if(_CR_elapsedAngleMS > _CR_angleLoopTimeMS)
        _CR_elapsedAngleMS -= _CR_angleLoopTimeMS;
}
void    ChronoRender_setPaused(bool isPaused) {
    _CR_isPaused = isPaused;
    if(isPaused) return;
    _CR_touchTimeMS = CR_elapsedMS_;
//    _CR_touchAngleTimeMS = _CR_elapsedAngleMS;
}
bool    ChronoRender_shouldSleep(void) {
    return CR_elapsedMS_ - _CR_touchTimeMS > _CR_sleepTime;
}
int64_t ChronoRender_elapsedMS(void) {
    return CR_elapsedMS_;
}
float   ChronoRender_elapsedSec(void) {
    return (float)CR_elapsedMS_ / 1000;
}
float   ChronoRender_elapsedAngleSec(void) {
    return (float)_CR_elapsedAngleMS / 1000;
}

/*-- ChronoApp, temps ecoule depuis l'ouverture de l'app. (vrais ms/sec) --*/
// variable "prives"
static bool    CA_isPaused_ = true;
static int64_t CA_time_ = 0;
static int64_t CA_lastSleepTime_ = 0;
static int64_t CA_startSleepTime_ = 0;
int64_t        CA_systemTime_(void) {
    struct timespec st;
    clock_gettime(CLOCK_REALTIME, &st);
    return st.tv_sec * 1000 + st.tv_nsec/ONE_MILLION;
}

void    ChronoApp_setPaused(bool isPaused) {
    if(isPaused == CA_isPaused_) return;
    CA_isPaused_ = isPaused;
    if(isPaused)
        CA_startSleepTime_ = CA_systemTime_();
    else
        CA_lastSleepTime_  = CA_systemTime_() - CA_startSleepTime_;
    // Temps écoulé ou temps de pause...
    CA_time_ = CA_systemTime_() - CA_time_;
}
int64_t ChronoApp_elapsedMS(void) {
    return CA_isPaused_ ? CA_time_ : (CA_systemTime_() - CA_time_);
}
float   ChronoApp_lastSleepTimeSec(void) {
    return (float)CA_lastSleepTime_ / 1000.f;
}

/** Un chronomètre basé sur le AppChrono (temps écoulé sans les "pause" de l'app).
 * N'est pas actif à l'ouverture. */
void chrono_start(Chrono *c) {
    c->_time = c->isRendering ? CR_elapsedMS_ : ChronoApp_elapsedMS();
    c->isActive = 1;
}
void chrono_stop(Chrono *c) {
    c->_time = 0;
    c->isActive = 0;
}
/// Le temps écoulé depuis "start()" en millisec.
int64_t chrono_elapsedMS(Chrono *c) {
    return c->isActive ? (c->isRendering ? CR_elapsedMS_ : ChronoApp_elapsedMS()) - c->_time
        : c->_time;
}
/// Le temps global où le chrono a commencé (en millisec).
int64_t chrono_startTimeMS(Chrono *c) {
    return c->isActive ? c->_time :
        (c->isRendering ? CR_elapsedMS_ : ChronoApp_elapsedMS()) - c->_time;
}
float   chrono_elapsedSec(Chrono *c) {
    return (float)chrono_elapsedMS(c) / 1000.f;
}
void chrono_pause(Chrono *c) {
    if(!c->isActive) return;
    c->_time = (c->isRendering ? CR_elapsedMS_ : ChronoApp_elapsedMS()) - c->_time;
    c->isActive = 0;
}
void chrono_unpause(Chrono *c) {
    if(c->isActive) return;
    c->_time = (c->isRendering ? CR_elapsedMS_ : ChronoApp_elapsedMS()) - c->_time;
    c->isActive = 1;
}
void chrono_addMS(Chrono *c, int64_t ms) {
    if(c->isActive)
        c->_time -= ms;
    else
        c->_time += ms;
}
void chrono_removeMS(Chrono *c, int64_t ms) {
    if(c->isActive) { // time est le starting time.
        int64_t elapsed = c->isRendering ? CR_elapsedMS_ : ChronoApp_elapsedMS();
        c->_time = (elapsed - c->_time > ms) ? c->_time + ms : elapsed;
    } else { // time est le temps écoulé.
        c->_time = (c->_time > ms) ? c->_time - ms : 0;
    }
}
void chrono_addSec(Chrono *c, float sec) {
    if(sec < 0.f)
        chrono_removeMS(c, (int64_t)(-sec*1000.f));
    else
        chrono_addMS(c, (int64_t)(sec*1000.f));
}

void    countdown_start(Countdown *cd) {
    chrono_start(&cd->c);
}
void    countdown_stop(Countdown *cd) {
    chrono_stop(&cd->c);
}
int64_t countdown_remainingMS(Countdown *cd) {
    return cd->ringTimeMS - chrono_elapsedMS(&cd->c);
}
bool     countdown_isRinging(Countdown *cd) {
    return chrono_elapsedMS(&cd->c) >= cd->ringTimeMS;
}

ChronoTiny chronotiny_startedChrono(void) {
    return (ChronoTiny)CR_elapsedMS_;
}
ChronoTiny chronotiny_elapsedChrono(int16_t elapsedMS) {
    return (ChronoTiny)CR_elapsedMS_ - elapsedMS;
}
// oui, c'est la meme chose que elepsedChrono ;)
int16_t    chronotiny_elapsedMS(ChronoTiny t) {
    return (ChronoTiny)CR_elapsedMS_ - t;
}
float      chronotiny_elapsedSec(ChronoTiny t) {
    return (float)((ChronoTiny)CR_elapsedMS_ - t) / 1000.f;
}



void    chronochecker_set(ChronoChecker* cc) {
    cc->_time = CA_systemTime_();
}
int64_t chronochecker_elapsedMS(ChronoChecker* cc) {
    return CA_systemTime_() - cc->_time;
}
void chronochecker_toc_(ChronoChecker* cc, const char* filename, uint32_t line) {
    int64_t elapsedMS = CA_systemTime_() - cc->_time;
    printf("🐸 Toc ! %lld ms -> %s line %d\n", elapsedMS, filename, line);
}

