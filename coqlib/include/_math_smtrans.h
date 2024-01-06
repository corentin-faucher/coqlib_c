//
//  _math_smtrans.h
//  Flag ON/OFF smooth.
//
//  Created by Corentin Faucher on 2023-10-14.
//

#ifndef _coq_math_smtrans_h
#define _coq_math_smtrans_h

#include "_math_chrono.h"

typedef struct _SmTrans {
    ChronoTiny _t;
    ChronoTiny _D;
    uint8_t _state;  // On est dans un etat a la fois.
    uint8_t _flags;  // On peut avoir plusieurs flags a la fois.
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
