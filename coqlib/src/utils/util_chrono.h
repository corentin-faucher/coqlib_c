//
//  utils/util_chrono.h
//
//  Created by Corentin Faucher on 2023-10-13.
//
//  Structs chrono pour compter le temps qui passe...
// Il y a quatre chronos "globals" :
// 1. "SystemTime" (privé) : temps de la machine, évalué à chaque call.
// 2. "ChronoApp" : temps écoulé "réel" depuis l'ouverture de l'app, mais sans pause. 
//      Évalué à chaque call, basé sur SystemTime.
// 3. "ChronosRender.render_elapsedMS" : temps de rendering, constant durant le calcul de la frame.
//      Évalué au début de la frame en incrémentant de +1/60 s à chaque frame. (Indépendant des autres)
// 4. "ChronosEvent.event_elapsedMS" : temps des events, constant durant le calcul des events d'un `tic`.
//      Évalué au début du tic (~toute les 50ms). Basé sur ChronoApp.
//
#ifndef COQ_MATH_CHRONO_H
#define COQ_MATH_CHRONO_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
// Le temps utilisé par défaut est la milliseconde (ms).
// Les constantes suivantes sont pour les conversions courantes...
#define ONE_MILLION    1000000 // pour ns <-> ms...
#define MS_PER_HOUR    3600000
#define MS_PER_MINUTE    60000
#define MS_PER_SEC        1000
#define SEC_PER_MS       0.001
// Durée d'un `tick` standard, i.e. 50 ms.
#define MS_PER_TICK         50

// MARK: - ChronoApp : chrono global pour le temps réel écoulé depuis l'ouverture de l'app. --
/// Vrai nombre de ms écoulées depuis l'ouverture de l'app.
int64_t  ChronoApp_elapsedMS(void);
/// Mise en pause (on/off) du temps écoulé.
void     ChronoApp_setPaused(bool isPaused);
/// Durée de la dernière mise en pause (avec ChronoApp_setPaused).
int64_t  ChronoApp_lastSleepTimeMS(void);

// "Capture" de temps.
typedef struct Chronos {
    int64_t const app_elapsedMS;
    int64_t const render_elapsedMS;
    int64_t const event_elapsedMS;
    uint32_t const tick; // Nombre de capture effectué depuis le début (à toute les deltaTMS).
    /// Temp de vie estimé de la capture, i.e. 16 ms pour le render, 50 ms pour le event.
    int64_t const deltaTMS;
} Chronos;

// MARK: - ChronoRender : chrono global pour les animation de rendering. Mis à jour à chaque frame de (typiquement) +1/60 sec (pas le vrai temps écoulé)
/// Capture des chronos mise à jour au début de chaque frame, e.g. 60 fois par sec -> à chaque ~16ms.
extern Chronos ChronosRender;
void     ChronoRender_update(void);
void     ChronoRender_setPaused(bool isPaused);
/// Pas d'activité ? -> Mettre en pause le rendering.
bool      ChronoRender_shouldSleep(void);
void      ChronoRender_setSleepTime(int64_t deltaTMStoSleep);

float    ChronoRender_elapsedSec(void);
/// Temps entre 0 et 24pi sec (pour les fonctions sin).
float    ChronoRender_elapsedAngleSec(void);

// MARK: - ChronoEvent : chrono global pour la gestion des events, timers, empty garbage...
/// Temps incrémenté de +50 ms à chaque tic (ne suis pas exactement ChronoApp).
extern Chronos ChronosEvent;
/// Mise à jour à chaque tic, e.g. 20 fois par sec -> à chaque ~50ms.
void     ChronoEvent_update(void);
/// Changer le delta T entre les tics pour une durée custom.
/// Le changement aura lieu au prochain `ChronoEvent_update`.
void     ChronoEvent_setTicDeltaT(int64_t newDeltaTMS);

// MARK: - Chrono ordinaire. Se base sur ChronoApp.
typedef struct CoqChrono {
    int64_t _time;
    bool isActive;
} Chrono;
void    chrono_start(Chrono *c);
void    chrono_stop(Chrono *c);
/// Le temps écoulé depuis "start()" en millisec ou sec.
int64_t chrono_elapsedMS(const Chrono *c);
float   chrono_elapsedSec(const Chrono *c);
/// Le temps global où le chrono a commencé (en millisec).
int64_t chrono_startTimeMS(const Chrono *c);
void    chrono_pause(Chrono *c);
void    chrono_unpause(Chrono *c);
void    chrono_addMS(Chrono *c, int64_t ms);
void    chrono_removeMS(Chrono *c, int64_t ms);
void    chrono_addSec(Chrono *c, float sec);

// MARK: - Minuterie (peut être casté comme Chrono)
typedef struct CoqCountdown {
    Chrono  c;  // Upcasting as Chrono.
    /// Temps de la minuterie en ms.
    int64_t ringTimeMS;
} Countdown;
/// Fonction de "convenience". Meme chose que chrono_start.
void    countdown_start(Countdown *cd);
/// Fonction de "convenience". Meme chose que chrono_stop.
void    countdown_stop(Countdown *cd);
/// Temps restant avant que ça "sonne". Peut être négatif (ringTimeMS - elapsedMS).
int64_t countdown_remainingMS(const Countdown *cd);
/// Ça sonne, i.e. elapsedMS >= ringTime.
bool    countdown_isRinging(const Countdown *cd);


// MARK: - ChronoChecker : Chrono simple pour debugging (calculer les vrai (system) ms écoulées)
typedef int64_t ChronoChecker;
/// Chrono simple pour debugging (calculer les ms écoulées)
ChronoChecker chronochecker_startNew(void);
int64_t chronochecker_elapsedMS(ChronoChecker const cc);
void    chronochecker_toc_(const ChronoChecker cc, const char*const filename, uint32_t const line);
#define chronochecker_toc(cc)\
chronochecker_toc_(cc, COQ__FILENAME__, __LINE__);

// MARK: `privé`, Évaluation du vrai temps du system (en ms) avec `clock_gettime`.
int64_t  Chrono_systemTimeMS_(void);

#endif /* chrono_h */
