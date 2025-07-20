//
//  math_smflag.h
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-09.
//
#ifndef COQ_MATH_SMFLAGE_H
#define COQ_MATH_SMFLAGE_H

#include <stdint.h>
#include <stdbool.h>

// MARK: - SmoothFlagE pour transition d'état binaire smooth dans le event thread. 
// (e.g. passage off -> on en 400 ms)
// Struct public pour avoir la taille, mais les éléments sont "privés".
typedef struct SmoothFlagE {
    int16_t    _t;
    uint16_t   _flags;
} SmoothFlagE;

SmoothFlagE SmoothFlagE_new(bool isOn, uint16_t transTimeTicksOpt);
void        SmoothFlagE_setDefaultTransTimeTicks(uint16_t transTimeTicks);

// Setters et getter du smooth flag ON/OFF.
float smoothflagE_setOn(SmoothFlagE * st);
float smoothflagE_setOff(SmoothFlagE * st);
float smoothflagE_value(SmoothFlagE * st);
// Estimation au temps future (ne met pas à jour l'état)
float smoothflagE_valueNext(SmoothFlagE const* sf);
void  smoothflagE_setTransitionTime(SmoothFlagE* sf, uint16_t newTransTimeTicks);


#endif
