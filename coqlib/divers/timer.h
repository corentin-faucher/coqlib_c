//
//  timer.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-23.
//

#ifndef timer_h
#define timer_h
#include <stdlib.h>
#include "_node.h"

typedef struct _Timer Timer;
// Verification des timers actifs...
// Si caller assez souvent, peut verifier si un noeud est a la poubelle.
// Mauvaise idee ? Mieu de caller timer_cancel lors d'un node_deinit ?
void      Timer_check(void);
void      timer_scheduled(Timer** timerRefOpt, int64_t deltaTimeMS, Bool isRepeating,
                     Node* node_target, void (*callBack)(Node*));
void      timer_cancel(Timer** timerRef);

#endif /* timer_h */
