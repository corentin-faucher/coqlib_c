//
//  timer.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-23.
//

#ifndef timer_h
#define timer_h

#include "node.h"

uint  Timer_createAndGetId(int64_t deltaTimeMS, Bool isRepeating,
                           Node* nd, void (*callBack)(Node*));
void  Timer_check(void);
void  timer_cancel(uint timerId);

#endif /* timer_h */
