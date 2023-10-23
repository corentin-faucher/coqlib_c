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
static       int64_t _CR_elapsedMS = 0;
static       int64_t _CR_elapsedAngleMS = 0;
static       int64_t _CR_touchTimeMS = 0;
static const int64_t _CR_angleLoopTimeMS = (int64_t)(24000 * M_PI);
static       int64_t _CR_sleepTime = 16000;

// Methodes
void    ChronoRender_update(int frequency) {
    if(_CR_isPaused) return;
    int64_t deltaT = (int64_t)(1000.f / (float)frequency);
    _CR_elapsedMS += deltaT;
    _CR_elapsedAngleMS += deltaT;
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
static int     _CA_isPaused = 0;
static int64_t _CA_time = 0;
static int64_t _CA_lastSleepTime = 0;
static int64_t _CA_startSleepTime = 0;
int64_t        _CA_systemTime(void) {
    struct timespec st;
    clock_gettime(CLOCK_REALTIME, &st);
    return st.tv_sec * 1000 + st.tv_nsec/1000000;
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
    c->time = c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS();
    c->isActive = 1;
}
void chrono_stop(Chrono *c) {
    c->time = 0;
    c->isActive = 0;
}
/// Le temps écoulé depuis "start()" en millisec.
int64_t chrono_elapsedMS(Chrono *c) {
    return c->isActive ? (c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS()) - c->time
        : c->time;
}
/// Le temps global où le chrono a commencé (en millisec).
int64_t chrono_startTimeMS(Chrono *c) {
    return c->isActive ? c->time :
        (c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS()) - c->time;
}
float   chrono_elapsedSec(Chrono *c) {
    return (float)chrono_elapsedMS(c) / 1000.f;
}
void chrono_pause(Chrono *c) {
    if(!c->isActive) return;
    c->time = (c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS()) - c->time;
    c->isActive = 0;
}
void chrono_unpause(Chrono *c) {
    if(c->isActive) return;
    c->time = (c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS()) - c->time;
    c->isActive = 1;
}
void chrono_addMS(Chrono *c, int64_t ms) {
    if(c->isActive)
        c->time -= ms;
    else
        c->time += ms;
}
void chrono_removeMS(Chrono *c, int64_t ms) {
    if(c->isActive) { // time est le starting time.
        int64_t elapsed = c->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS();
        c->time = (elapsed - c->time > ms) ? c->time + ms : elapsed;
    } else { // time est le temps écoulé.
        c->time = (c->time > ms) ? c->time - ms : 0;
    }
}
void chrono_addSec(Chrono *c, float sec) {
    if(sec < 0.f)
        chrono_removeMS(c, (int64_t)(-sec*1000.f));
    else
        chrono_addMS(c, (int64_t)(sec*1000.f));
}

void    countdown_start(Countdown *cd) {
    chrono_start((Chrono*)cd);
}
void    countdown_stop(Countdown *cd) {
    chrono_stop((Chrono*)cd);
}
int     countdown_isRinging(Countdown *cd) {
    if(cd->isActive)
        return (cd->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS()) - cd->time > cd->ringTimeMS;
    else
        return cd->time > cd ->ringTimeMS;
}
int64_t countdown_remainingMS(Countdown *cd) {
    int64_t elapsed = cd->isRendering ? _CR_elapsedMS : ChronoApp_elapsedMS();
    if(elapsed > cd->ringTimeMS)
        return 0;
    else
        return cd->ringTimeMS - elapsed;
}

ChronoTiny chronotiny_start(void) {
    return (ChronoTiny)_CR_elapsedMS;
}
ChronoTiny chronotiny_elapsedMS(ChronoTiny t) {
    return (ChronoTiny)_CR_elapsedMS - t;
}
ChronoTiny chronotiny_setToElapsed(ChronoTiny elapsedMS) {
    return (ChronoTiny)_CR_elapsedMS - elapsedMS;
}
float   chronotiny_elapsedSec(ChronoTiny t) {
    return (float)((ChronoTiny)_CR_elapsedMS - t) / 1000.f;
}
