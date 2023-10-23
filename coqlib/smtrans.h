//
//  smtrans.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#ifndef smtrans_h
#define smtrans_h

#include <stdio.h>
#include "chronometers.h"

typedef struct _SmTrans {
    ChronoTiny _t;
    ChronoTiny _D;
    uint8_t _state;
    uint8_t _flags;
} SmTrans;

void  SmTrans_setPopFactor(float popFactor);
void  SmTrans_setSemiFactor(float semiFactor);
void  SmTrans_setTransTime(ChronoTiny transTime);

void  smtrans_init(SmTrans *st);
int   smtrans_isActive(SmTrans st);
float smtrans_isOnSmooth(SmTrans *st);
float smtrans_setAndGetIsOnSmooth(SmTrans *st, int isOn);

void  smtrans_setIsOn(SmTrans *st, int isOn);
void  smtrans_setIsOnHard(SmTrans *st, int isOn);
void  smtrans_setOptions(SmTrans *st, int isHard, int isPoping);

#endif /* smtrans_h */
