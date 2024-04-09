//
//  _math_smtrans.h
//  Flag ON/OFF smooth.
//
//  Created by Corentin Faucher on 2023-10-14.
//
#ifndef COQ_MATH_SMTRANS_H
#define COQ_MATH_SMTRANS_H

#include "math_chrono.h"

// (Public pour avoir la taille, mais la structure est "privée".)
typedef struct SmTrans {
    ChronoTiny _t;
    ChronoTiny _D;
    uint16_t   _flags;
    uint16_t   _sub;
} SmTrans;

void  SmTrans_setPopFactor(float popFactor);
void  SmTrans_setSemiFactor(float semiFactor);
void  SmTrans_setTransTime(ChronoTiny transTime);

void  smtrans_init(SmTrans *st);
bool  smtrans_isActive(SmTrans st);
float smtrans_value(SmTrans *st);
float smtrans_setAndGetIsOnSmooth(SmTrans *st, bool isOn);

void  smtrans_setIsOn(SmTrans *st, bool isOn);
void  smtrans_fixIsOn(SmTrans *st, bool isOn);
/// Modifie la valeur "ON", par défaut 1. Doit être entre 0 et 1.
void  smtrans_setMaxValue(SmTrans *st, float newMax);
void  smtrans_setOptions(SmTrans *st, bool isHard, bool isPoping);

#endif /* smtrans_h */
