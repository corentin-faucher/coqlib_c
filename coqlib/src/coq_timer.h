 //
//  timer.h
//  Struct pour les timers. Call une fonction `callback` après un certain temps.
//
//  Created by Corentin Faucher on 2023-10-23.
//

#ifndef COQ_TIMER_H
#define COQ_TIMER_H

#include <stdbool.h>
#include <stdint.h>

// Une variable "Timer" est en fait un pointeur vers la structure privé coq_TimerStruct. (i.e. une "variable Java style"...)
typedef struct coq_TimerStruct* Timer;

/// Verification des timers actifs et exécution des `callBack` qui sont dûs.
void Timer_check(void);

/// Création d'un timer qui call `callBack` sur `targetObject` après `deltaTimeMS`.
/// Pour répéter à chaque "tick", mettre deltaTimeMS entre 0 et `Chrono_UpdateDeltaTMS` (50ms).
/// A priori, il est preferable de garder la reference au timer (passer à `timerRef`) pour pouvoir cancel le timer.
/// -> ** S'assurer de caller `timer_cancel` avant de dealloc `target_object`. **
/// targetObjectOpt: l'objet sur lequel le callBack agit (optionel).
void timer_scheduled(Timer *timerRef, int64_t deltaTimeMS, bool isRepeating,
                     void *targetObjectOpt, void (*callBack)(void *targetObjectOpt));

/// Annuler le callback. La ref `*timerRef` est mis à null.
void timer_cancel(Timer *timerRef);
/// Exécuter tout de suite le callback et cancel (remet à NULL).
void timer_doNowAndCancel(Timer *timerRef);



#endif /* timer_h */
