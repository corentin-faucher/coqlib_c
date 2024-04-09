 //
//  timer.h
//  Struct pour les timers. Call une fonction `callback` après un certain temps.
//
//  Created by Corentin Faucher on 2023-10-23.
//

#ifndef COQ_TIMER_H
#define COQ_TIMER_H

#include "nodes/node_base.h"

typedef struct _Timer Timer;

/// Verification des timers actifs...
/// Si caller assez souvent, peut verifier si un noeud est a la poubelle.
/// Mauvaise idee ? Mieu de caller timer_cancel lors d'un node_deinit ?
void Timer_check(void);

/// Création d'un timer qui call `callBack` sur `target_object` après `deltaTimeMS`.
/// A priori, il est preferable de garder la reference au timer (passer à `timerRefOpt`).
/// Si le `target_node` est `_flag_toDelete`, le timer sera automatiquement cancelé.
void timer_scheduled(Timer** timerRefOpt, int64_t deltaTimeMS, bool isRepeating,
                     Node* target_node, void (*callBack)(Node*));

void timer_cancel(Timer** timerRef);

/// Création d'un timer qui call `callBack` sur `target_node` après `deltaTimeMS`.
/// A priori, il est preferable de garder la reference au timer (passer à `timerRefOpt`).
/// Si le `target_node` est `_flag_toDelete`, le timer sera automatiquement cancelé.
//void timer_scheduledNode(Timer** timerRefOpt, int64_t deltaTimeMS, bool isRepeating,
//                         Node* target_node, void (*callBack)(Node*));


#endif /* timer_h */
