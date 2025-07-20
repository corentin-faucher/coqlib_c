//
//  chrono.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-13.
//

#include "util_chrono.h"

#include "../maths/math_base.h"

// MARK: Evaluation du vrai temps du system (en ms)
int64_t        Chrono_systemTimeMS_(void) {
    struct timespec st;
    clock_gettime(CLOCK_REALTIME, &st);
    return st.tv_sec * 1000 + st.tv_nsec/ONE_MILLION;
}

// MARK: - ChronoApp, temps ecoule depuis l'ouverture de l'app. (vrais ms/sec)
// variable "prives"
static bool    CA_isPaused_ = true;
static int64_t CA_time_ = 0;
static int64_t CA_lastSleepTime_ = 0;
static int64_t CA_startSleepTime_ = 0;


void    ChronoApp_setPaused(bool isPaused) {
    if(isPaused == CA_isPaused_) return;
    CA_isPaused_ = isPaused;
    if(isPaused)
        CA_startSleepTime_ = Chrono_systemTimeMS_();
    else
        CA_lastSleepTime_  = Chrono_systemTimeMS_() - CA_startSleepTime_;
    // Si unpause : temps de pause, si pause : Temps écoulé.
    CA_time_ = Chrono_systemTimeMS_() - CA_time_;
}
int64_t ChronoApp_elapsedMS(void) {
    return CA_isPaused_ ? CA_time_ : (Chrono_systemTimeMS_() - CA_time_);
}
int64_t  ChronoApp_lastSleepTimeMS(void) {
    return CA_lastSleepTime_;
}

// MARK: - ChronoRender, le chrono pour le rendering. --
Chronos ChronosRender = { .deltaTMS = 16 };
// variable "prives"
static       int     CR_isPaused_ = 0;
static       int64_t CR_sleepTime_ = 16000;
static       int64_t CR_elapsedAngleMS_ = 0;
static       int64_t CR_touchTimeMS_ = 0;
#define CR_angleLoopTimeMS_ (int64_t)(24000 * M_PI)

static Chrono ChronoRender_deltaT_;
void ChronoRender_update(void) {
    if(CR_isPaused_) return;
    int64_t deltaTMS = chrono_elapsedMS(&ChronoRender_deltaT_);
    deltaTMS = deltaTMS < 1 ? 1 : (deltaTMS > 60 ? 60 : deltaTMS);
    chrono_start(&ChronoRender_deltaT_);
    
    *(int64_t*)&ChronosRender.app_elapsedMS =    ChronoApp_elapsedMS();
    *(int64_t*)&ChronosRender.render_elapsedMS = ChronosRender.render_elapsedMS + deltaTMS;
    *(int64_t*)&ChronosRender.event_elapsedMS =  ChronosEvent.event_elapsedMS;
    *(int64_t*)&ChronosRender.deltaTMS =         deltaTMS;
    *(int64_t*)&ChronosRender.tick = ChronosRender.tick + 1; 
    CR_elapsedAngleMS_ += deltaTMS;
    if(CR_elapsedAngleMS_ > CR_angleLoopTimeMS_)
        CR_elapsedAngleMS_ -= CR_angleLoopTimeMS_;
}
void    ChronoRender_setPaused(bool isPaused) {
    CR_isPaused_ = isPaused;
    if(isPaused) {
        chrono_pause(&ChronoRender_deltaT_);
        return;
    }
    chrono_unpause(&ChronoRender_deltaT_);
    CR_touchTimeMS_ = ChronosRender.render_elapsedMS;
}
bool    ChronoRender_shouldSleep(void) {
    return ChronosRender.render_elapsedMS - CR_touchTimeMS_ > CR_sleepTime_;
}
void    ChronoRender_setSleepTime(int64_t deltaTMStoSleep) {
    CR_sleepTime_ = deltaTMStoSleep;
}
float   ChronoRender_elapsedSec(void) {
    return (float)ChronosRender.render_elapsedMS / 1000.f;
}
float   ChronoRender_elapsedAngleSec(void) {
    return (float)CR_elapsedAngleMS_ / 1000.f;
}

// MARK: - ChronoEvent : chrono global pour la gestion des events et timers. Mis à jour à chaque tic. Basé sur le vrai temps écoulé de l'app.
Chronos ChronosEvent = { .deltaTMS = 50 };
int64_t ChronosEvent_nextDeltaTMS = 0;
/// Mise à jour à chaque tic, e.g. 20 fois par sec -> à chaque ~50ms.
void     ChronoEvent_update(void) {
    *(int64_t*)&ChronosEvent.app_elapsedMS =    ChronoApp_elapsedMS();
    *(int64_t*)&ChronosEvent.render_elapsedMS = ChronosRender.render_elapsedMS ;
    if(ChronosEvent_nextDeltaTMS) {
        *(int64_t*)&ChronosEvent.deltaTMS = ChronosEvent_nextDeltaTMS;
        ChronosEvent_nextDeltaTMS = 0;
    }
    *(int64_t*)&ChronosEvent.event_elapsedMS =  ChronosEvent.event_elapsedMS + ChronosEvent.deltaTMS;
    *(int64_t*)&ChronosEvent.tick = ChronosEvent.tick + 1;
}
void     ChronoEvent_setTicDeltaT(int64_t newDeltaTMS) {
    ChronosEvent_nextDeltaTMS = newDeltaTMS;
}

// MARK: - Chrono ordinaire.
/** Un chronomètre basé sur le AppChrono (temps écoulé sans les "pause" de l'app).
 * N'est pas actif à l'ouverture. */
void chrono_start(Chrono *c) {
    c->_time = ChronoApp_elapsedMS(); // c->isRendering ? CR_elapsedMS_ : ChronoApp_elapsedMS();
    c->isActive = 1;
}
void chrono_stop(Chrono *c) {
    c->_time = 0;
    c->isActive = 0;
}
/// Le temps écoulé depuis "start()" en millisec.
int64_t chrono_elapsedMS(const Chrono *c) {
    return c->isActive ? ChronoApp_elapsedMS() - c->_time : c->_time;
}
/// Le temps global où le chrono a commencé (en millisec).
int64_t chrono_startTimeMS(const Chrono *c) {
    return c->isActive ? c->_time : ChronoApp_elapsedMS() - c->_time;
}
float   chrono_elapsedSec(const Chrono *c) {
    return (float)(c->isActive ? ChronoApp_elapsedMS() - c->_time
        : c->_time) / 1000.f;
}
void chrono_pause(Chrono *c) {
    if(!c->isActive) return;
    c->_time = ChronoApp_elapsedMS() - c->_time;
    c->isActive = 0;
}
void chrono_unpause(Chrono *c) {
    if(c->isActive) return;
    c->_time = ChronoApp_elapsedMS() - c->_time;
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
        int64_t elapsed = ChronoApp_elapsedMS();
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
int64_t countdown_remainingMS(const Countdown *cd) {
    return cd->ringTimeMS - chrono_elapsedMS(&cd->c);
}
bool     countdown_isRinging(const Countdown *cd) {
    return chrono_elapsedMS(&cd->c) >= cd->ringTimeMS;
}


// MARK: - ChronoChecker : Chrono simple pour debugging (calculer les vrai (system) ms écoulées)
/// Chrono simple pour debugging (calculer les ms écoulées)
ChronoChecker chronochecker_startNew(void) {
    return Chrono_systemTimeMS_();
}
int64_t       chronochecker_elapsedMS(ChronoChecker const cc) {
    return Chrono_systemTimeMS_() - cc;
}
void chronochecker_toc_(const ChronoChecker cc, const char*const filename, uint32_t const line) {
    printf("🐸 Toc ! %d ms -> %s line %d\n",
        (int)(Chrono_systemTimeMS_() - cc), filename, line);
}
