//
//  chrono.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-13.
//

#ifndef chrono_h
#define chrono_h

#include <stdio.h>
/*--Chrono global pour le rendering --*/
/// Mise à jour à chaque frame, e.g. +1/60 sec au elapsed time.
void     ChronoRender_update(int frequency);
void     ChronoRender_setPaused(int isPaused);
/// Pas d'activité ? -> Mettre en pause le rendering.
int      ChronoRender_shouldSleep(void);
/// Nombre de ms écoulées depuis l'ouverture de l'app.
int64_t  ChronoRender_elapsedMS(void);
float    ChronoRender_elapsedSec(void);
/// Temps entre 0 et 24pi sec (pour les fonctions sin).
float    ChronoRender_elapsedAngleSec(void);

/*--Chrono global pour le temps réel de l'app. --*/
void     ChronoApp_setPaused(int isPaused);
/// Vrai nombre de ms écoulées depuis l'ouverture de l'app.
/// (Pas juste une somme de +1/60sec...)
int64_t  ChronoApp_elapsedMS(void);
/// Durée de la dernière mise en pause (avec ChronoApp_setPaused).
float    ChronoApp_lastSleepTimeSec(void);

/*-- Chrono ordinaire. Se base sur ChronoApp a priori (temps réel). --*/
typedef struct _Chrono {
    int64_t time;
    int isActive;
    int isRendering;  // Si true -> basé sur ChronoRender, sinon sur ChronoApp.
} Chrono;
void chrono_start(Chrono *c);
void chrono_stop(Chrono *c);
/// Le temps écoulé depuis "start()" en millisec ou sec.
int64_t chrono_elapsedMS(Chrono *c);
float   chrono_elapsedSec(Chrono *c);
/// Le temps global où le chrono a commencé (en millisec).
int64_t chrono_startTimeMS(Chrono *c);
void chrono_pause(Chrono *c);
void chrono_unpause(Chrono *c);
void chrono_addMS(Chrono *c, int64_t ms);
void chrono_removeMS(Chrono *c, int64_t ms);
void chrono_addSec(Chrono *c, float sec);

// Minuterie (peut être casté comme Chrono, mêmes début de struct)
typedef struct _Countdown {
    int64_t time;
    int isActive;
    int isRendering;  // Basé sur ChronoRender, sinon sur ChronoApp.
    int64_t ringTimeMS;
} Countdown;
void    countdown_start(Countdown *cd);
void    countdown_stop(Countdown *cd);
int     countdown_isRinging(Countdown *cd);
int64_t countdown_remainingMS(Countdown *cd);
// On peut utiliser les autres méthodes : chrono_pause, chrono_addMS, etc.
// en castant avec (Chrono*).

/// Mini chrono de 2 octets (basé sur ChronoRendering)
/// Lapse max de 2^16 ms = 65sec.
/// La sruct est simplement {int16_t time}.
typedef int16_t ChronoTiny;
#warning Pour etre coherent, passer par ref l'instance ?
ChronoTiny chronotiny_start(void);
ChronoTiny chronotiny_elapsedMS(ChronoTiny t);
ChronoTiny chronotiny_setToElapsed(ChronoTiny elapsedMS);
float      chronotiny_elapsedSec(ChronoTiny t);

#endif /* chrono_h */
