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

/// Verification des timers actifs et exécution des `callBack` qui sont dûs.
void Timer_check(void);

/// Création d'un timer qui call `callBack` sur `targetObject` après `deltaTimeMS`.
/// Pour répéter à chaque "tick", mettre deltaTimeMS à 1 ms.
/// A priori, il est preferable de garder la reference au timer (passer à `timerRefOpt`) pour pouvoir cancel le timer.
/// S'assurer de caller `timer_cancel` avant de dealloc `target_object`.
/// targetObjectOpt: l'objet sur lequel le callBack agit (optionel).
void timer_scheduled(Timer** timerRefOpt, int64_t deltaTimeMS, bool isRepeating,
                     void* targetObjectOpt, void (*callBack)(void* targetObjectOpt));

/// Annuler le callback. La ref `*timerRef` est mis à null.
void timer_cancel(Timer** timerRef);


// Garbage...
/// Création d'un timer qui call `callBack` sur `target_node` après `deltaTimeMS`.
/// A priori, il est preferable de garder la reference au timer (passer à `timerRefOpt`).
/// Si le `target_node` est `_flag_toDelete`, le timer sera automatiquement cancelé.
//void timer_scheduledNode(Timer** timerRefOpt, int64_t deltaTimeMS, bool isRepeating,
//                         Node* target_node, void (*callBack)(Node*));


#endif /* timer_h */
