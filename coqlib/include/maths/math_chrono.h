//
//  _math_chrono.h
//  Structs chrono pour le temps qui passe...
//
//  Created by Corentin Faucher on 2023-10-13.
//

#ifndef COQ_MATH_CHRONO_H
#define COQ_MATH_CHRONO_H

#include <stdbool.h>
#include <math.h>
#include <time.h>

#define ONE_MILLION    1000000
#define MS_PER_HOUR    3600000
#define MS_PER_MINUTE    60000

/// Le delai entre les update Timer, Garbage, Root->iterationUpdate.
/// À 50 ms par defaut,  i.e. 20 refresh / second.
extern int64_t Chrono_UpdateDeltaTMS; // = 50;
/*--Chrono global pour le rendering --*/
/// Mise à jour à chaque frame, e.g. +1/60 sec ~= 16ms au elapsed time.
void     ChronoRender_update(int64_t deltaTMS);
void     ChronoRender_setPaused(bool isPaused);
/// Pas d'activité ? -> Mettre en pause le rendering.
bool      ChronoRender_shouldSleep(void);
/// Nombre de ms écoulées depuis l'ouverture de l'app.
int64_t  ChronoRender_elapsedMS(void);
float    ChronoRender_elapsedSec(void);
/// Temps entre 0 et 24pi sec (pour les fonctions sin).
float    ChronoRender_elapsedAngleSec(void);

/*--Chrono global pour le temps réel de l'app. --*/
void     ChronoApp_setPaused(bool isPaused);
/// Vrai nombre de ms écoulées depuis l'ouverture de l'app.
/// (Pas juste une somme de +1/60sec...)
int64_t  ChronoApp_elapsedMS(void);
/// Durée de la dernière mise en pause (avec ChronoApp_setPaused).
float    ChronoApp_lastSleepTimeSec(void);

/*-- Chrono ordinaire. Se base sur ChronoApp a priori (temps réel). --*/
typedef struct _Chrono {
    int64_t _time;
    bool isActive;
    bool isRendering;  // Si true -> basé sur ChronoRender, sinon sur ChronoApp.
} Chrono;
void    chrono_start(Chrono *c);
void    chrono_stop(Chrono *c);
/// Le temps écoulé depuis "start()" en millisec ou sec.
int64_t chrono_elapsedMS(Chrono *c);
float   chrono_elapsedSec(Chrono *c);
/// Le temps global où le chrono a commencé (en millisec).
int64_t chrono_startTimeMS(Chrono *c);
void    chrono_pause(Chrono *c);
void    chrono_unpause(Chrono *c);
void    chrono_addMS(Chrono *c, int64_t ms);
void    chrono_removeMS(Chrono *c, int64_t ms);
void    chrono_addSec(Chrono *c, float sec);

// Minuterie, peut être casté comme Chrono (mêmes début de struct).
typedef struct _Countdown {
    /// Upcasting as Chrono.
    Chrono  c;
    /// Temps de la minuterie en ms.
    int64_t ringTimeMS;
} Countdown;
/// Fonction de "convenience". Meme chose que chrono_start.
void    countdown_start(Countdown *cd);
/// Fonction de "convenience". Meme chose que chrono_stop.
void    countdown_stop(Countdown *cd);
/// Temps restant avant que ça "sonne". Peut être négatif (ringTimeMS - elapsedMS).
int64_t countdown_remainingMS(Countdown *cd);
/// Ça sonne, i.e. elapsedMS >= ringTime.
bool    countdown_isRinging(Countdown *cd);

/// Convenience de countdown_remainingMS pour avoir un ratio (entre 0 et 1) de se qu'il reste de temps.

/// Mini chrono de 2 octets (basé sur ChronoRendering)
/// Lapse max de 2^16 ms, i.e. de -32s à +32s.
/// La sruct est simplement {int16_t time}.
typedef int16_t ChronoTiny;
/// Optenir un ChronoTiny que l'on "start" a zero.
ChronoTiny chronotiny_startedChrono(void);
/// Optenir un ChronoTiny que l'on "start" a elapsedMS.
ChronoTiny chronotiny_elapsedChrono(int16_t elapsedMS);
int16_t    chronotiny_elapsedMS(ChronoTiny t);
float      chronotiny_elapsedSec(ChronoTiny t);

typedef struct {
    int64_t _time;
} ChronoChecker;

void    chronochecker_set(ChronoChecker* cc);
int64_t chronochecker_elapsedMS(ChronoChecker* cc);
void    chronochecker_toc_(ChronoChecker* cc, const char* filename, uint32_t line);
#define chronochecker_toc(cc)\
chronochecker_toc_(cc, COQ__FILENAME__, __LINE__);

// Internal... elapsed chronorender.
extern int64_t CR_elapsedMS_;

#endif /* chrono_h */
