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
void  SmTrans_setDefaultTransTime(ChronoTiny transTime);

void  smtrans_init(SmTrans *st);

// Getters
bool  smtrans_isActive(SmTrans st); // (Taille 8 bytes... inutile de passer par ref pour getter)
float smtrans_value(SmTrans *st);   // (Ici, optenir la valeur met aussi à jour l'état)

// Setters
float smtrans_setAndGetValue(SmTrans *st, bool isOn);
void  smtrans_setIsOn(SmTrans *st, bool isOn);
void  smtrans_fixIsOn(SmTrans *st, bool isOn);
/// Modifie la valeur "ON", par défaut 1. Doit être entre 0 et 1.
void  smtrans_setMaxValue(SmTrans *st, float newMax);
void  smtrans_setOptions(SmTrans *st, bool isHard, bool isPoping);
void  smtrans_setDeltaT(SmTrans *st, ChronoTiny deltaT);

#endif /* smtrans_h */
