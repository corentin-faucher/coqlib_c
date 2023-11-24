//
//  chrono.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-13.
//

#include "chronometers.h"

#include <math.h>
#include <time.h>
#include <time.h>


/*-- ChronoRender, le chrono pour le rendering. --*/
// variable "prives"
static       int     _CR_isPaused = 0;
int64_t _CR_elapsedMS = 0;
static       int64_t _CR_elapsedAngleMS = 0;
static       int64_t _CR_touchTimeMS = 0;
static const int64_t _CR_angleLoopTimeMS = (int64_t)(24000 * M_PI);
static       int64_t _CR_sleepTime = 16000;

int64_t Chrono_UpdateDeltaTMS = 30;
// Methodes
void    ChronoRender_update(int64_t deltaTMS) {
    if(_CR_isPaused) return;
    _CR_elapsedMS += deltaTMS;
    _CR_elapsedAngleMS += deltaTMS;
    if(_CR_elapsedAngleMS > _CR_angleLoopTimeMS)
        _CR_elapsedAngleMS -= _CR_angleLoopTimeMS;
}
void    ChronoRender_setPaused(int isPaused) {
    _CR_isPaused = isPaused;
    if(isPaused) return;
    _CR_touchTimeMS = _CR_elapsedMS;
//    _CR_touchAngleTimeMS = _CR_elapsedAngleMS;
}
int     ChronoRender_shouldSleep(void) {
    return _CR_elapsedMS - _CR_touchTimeMS > _CR_sleepTime;
}
int64_t ChronoRender_elapsedMS(void) {
    return _CR_elapsedMS;
}
float   ChronoRender_elapsedSec(void) {
    return (float)_CR_elapsedMS / 1000;
}
float   ChronoRender_elapsedAngleSec(void) {
    return (float)_CR_elapsedAngleMS / 1000;
}

/*-- ChronoApp, temps ecoule depuis l'ouverture de l'app. (vrais ms/sec) --*/
// variable "prives"
static int     _CA_isPaused = 1;
static int64_t _CA_time = 0;
static int64_t _CA_lastSleepTime = 0;
static int64_t _CA_startSleepTime = 0;
int64_t        _CA_systemTime(void) {
    struct timespec st;
    clock_gettime(CLOCK_REALTIME, &st);
    return st.tv_sec * 1000 + st.tv_nsec/ONE_MILLION;
}

void    ChronoApp_setPaused(int isPaused) {
    if(isPaused == _CA_isPaused) return;
    _CA_isPaused = isPaused;
    if(isPaused)
        _CA_startSleepTime = _CA_systemTime();
    else
        _CA_lastSleepTime  = _CA_systemTime() - _CA_startSleepTime;
    // Temps écoulé ou temps de pause...
    _CA_time = _CA_systemTime() - _CA_time;
}
int64_t ChronoApp_elapsedMS(void) {
    return _CA_isPaused ? _CA_time : (_CA_systemTime() - _CA_time);
}
float   ChronoApp_lastSleepTimeSec(void) {
    return (float)_CA_lastSleepTime / 1000.f;
}

/** Un chronomètre basé sur le AppChrono (temps écoulé sans les "pause" de l'app).
 * N'est pas actif à l'ouverture. */
void chrono_start(Chrono *c) {
    c->_time = c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS();
    c->isActive = 1;
}
void chrono_stop(Chrono *c) {
    c->_time = 0;
    c->isActive = 0;
}
/// Le temps écoulé depuis "start()" en millisec.
int64_t chrono_elapsedMS(Chrono *c) {
    return c->isActive ? (c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS()) - c->_time
        : c->_time;
}
/// Le temps global où le chrono a commencé (en millisec).
int64_t chrono_startTimeMS(Chrono *c) {
    return c->isActive ? c->_time :
        (c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS()) - c->_time;
}
float   chrono_elapsedSec(Chrono *c) {
    return (float)chrono_elapsedMS(c) / 1000.f;
}
void chrono_pause(Chrono *c) {
    if(!c->isActive) return;
    c->_time = (c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS()) - c->_time;
    c->isActive = 0;
}
void chrono_unpause(Chrono *c) {
    if(c->isActive) return;
    c->_time = (c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS()) - c->_time;
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
        int64_t elapsed = c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS();
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
Bool     countdown_isRinging(Countdown *cd) {
    return chrono_elapsedMS(&cd->c) >= cd->ringTimeMS;
}

ChronoTiny chronotiny_startedChrono(void) {
    return (ChronoTiny)_CR_elapsedMS;
}
ChronoTiny chronotiny_elapsedChrono(int16_t elapsedMS) {
    return (ChronoTiny)_CR_elapsedMS - elapsedMS;
}
// oui, c'est la meme chose que elepsedChrono ;)
int16_t    chronotiny_elapsedMS(ChronoTiny t) {
    return (ChronoTiny)_CR_elapsedMS - t;
}
float      chronotiny_elapsedSec(ChronoTiny t) {
    return (float)((ChronoTiny)_CR_elapsedMS - t) / 1000.f;
}



void    _chronochecker_set(ChronoChecker* cc) {
    cc->_time = _CA_systemTime();
}
int64_t _chronochceker_elapsedMS(ChronoChecker* cc) {
    return _CA_systemTime() - cc->_time;
}
void _chronochecker_toc(ChronoChecker* cc, const char* filename, uint32_t line) {
    int64_t elapsedMS = _CA_systemTime() - cc->_time;
    printf("🐸 Toc ! %lld ms -> %s line %d\n", elapsedMS, filename, line);
}

