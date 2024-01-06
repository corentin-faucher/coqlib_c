//
//  timer.h
//  Struct pour les timers. Call une fonction `callback` après un certain temps.
//
//  Created by Corentin Faucher on 2023-10-23.
//

#ifndef _coq_timer_h
#define _coq_timer_h

#include "_utils_.h"

typedef struct _Timer Timer;

/// Verification des timers actifs...
/// Si caller assez souvent, peut verifier si un noeud est a la poubelle.
/// Mauvaise idee ? Mieu de caller timer_cancel lors d'un node_deinit ?
void Timer_check(void);

/// Création d'un timer qui call `callBack` sur `target_object` après `deltaTimeMS`.
/// A priori, il est preferable de garder la reference au timer (passer à `timerRefOpt`).
/// Si le taget objet doit etre `free` il faut caller `timer_cancel` dans sont deinit.
void timer_scheduled(Timer** timerRefOpt, int64_t deltaTimeMS, bool isRepeating,
                     void* target_object, void (*callBack)(void*));
void timer_cancel(Timer** timerRef);

#endif /* timer_h */
