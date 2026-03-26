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
// 3. "RendererTimeCapture.render_elapsedMS" : temps de rendering, constant durant le calcul de la frame.
//      Évalué au début de la frame en incrémentant de +1/60 s à chaque frame. (Indépendant des autres)
// 4. "EventTimeCapture.event_elapsedMS" : temps des events, constant durant le calcul des events d'un `tic`.
//      Évalué au début du tic (~toute les 50ms). Basé sur ChronoApp.
//
#ifndef COQ_MATH_CHRONO_H
#define COQ_MATH_CHRONO_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "util_base.h"
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
/// Temps restant avant que ça "sonne". Peut être négatif (ringTimeMS - elapsedMS).
int64_t countdown_remainingMS(const Countdown *cd);
/// Ça sonne, i.e. elapsedMS >= ringTime.
bool    countdown_isRinging(const Countdown *cd);
// Fonctions de "convenience" (repris de chrono).
void    countdown_start(Countdown *cd);
void    countdown_stop(Countdown *cd);


// MARK: - ChronoChecker : Chrono simple pour debugging (calculer les vrai (system) ms écoulées)
typedef int64_t ChronoChecker;
/// Chrono simple pour debugging (calculer les ms écoulées)
ChronoChecker chronochecker_startNew(void);
int64_t chronochecker_elapsedMS(ChronoChecker const cc);
void    chronochecker_toc_(const ChronoChecker cc, const char* filename, uint32_t line);
#define chronochecker_toc(cc) chronochecker_toc_(cc, COQ__FILENAME__, __LINE__)
void    chronochecker_tocWithComment(ChronoChecker cc, const char* comment);
void    chronochecker_sleepRemaining_(ChronoChecker cc, int64_t deltaTMS, bool showWarning, 
                                      const char* filename, uint32_t line);
#define chronochecker_sleepRemaining(cc, deltaTMS, showWarning)\
    chronochecker_sleepRemaining_(cc, deltaTMS, showWarning, COQ__FILENAME__, __LINE__)
    

// MARK: - "Capture" de temps
// Une capture à chaque itération de "Event" (tick) et "Rendering" (frame).
typedef struct TimeCapture {
    int64_t const app_elapsedMS;    // Temps "ChronoApp_elasedMS" au temps de la capture.
    int64_t const render_elapsedMS; // Dernier temps de capture pour RendererTimeCapture.
    int64_t const event_elapsedMS;  // Dernier temps de capture pour EventTimeCapture.
    uint32_t const tick; // Nombre de capture effectuées depuis le début (à toute les deltaTMS).
    /// Temp de vie estimé de la capture, i.e. 16 ms pour le render, 50 ms pour le event.
    int64_t const deltaTMS;
} TimeCapture;

// MARK: - RendererTimeCapture : Les temps pour le renderer. 
// Mis à jour à chaque frame de (typiquement) +1/60 sec (pas le vrai temps écoulé)
// Dans le cas du renderer, deltaTMS n'est pas nécessairement constant 
// et correspond au dernier delta t.
extern TimeCapture RendererTimeCapture;
// Update : *** doit être callé au début de la fonction render d'une frame ***
void     RendererTimeCapture_update(void);
/// Temps entre 0 et 60 sec (pour les fonctions sin, cos dans les shaders).
float    RendererTimeCapture_elapsedAngleSec(void);
void     RendererTimeCapture_setAngleLoopTime(float angleLoopTimeSec);

// MARK: - EventTimeCapture : Les temps pour les events.
/// Mis à jour à toute les 50 ms (tick).
extern TimeCapture EventTimeCapture;
/// Mise à jour à chaque tick, e.g. 20 fois par sec -> à chaque ~50ms.
void     EventTimeCapture_update(void);
/// Changer le delta T entre les tics pour une durée custom
/// ou pour faire des "bullet times" 
/// (garder 50 ms seconde pour la physique, mais faire les mise à jour aux 200ms par exemple).
/// Le changement aura lieu au prochain `EventTimeCapture_update`.
void     EventTimeCapture_setTicDeltaT(int64_t newDeltaTMS);


// MARK: `privé`, Évaluation du vrai temps du system (en ms) avec `clock_gettime`.
int64_t  Chrono_systemTimeMS_(void);

#endif /* chrono_h */
