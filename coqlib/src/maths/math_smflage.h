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

// MARK: - SmoothFlag pour transition d'état binaire smooth dans le event thread. 
// (e.g. passage off -> on en 400 ms)
// Struct public pour avoir la taille, mais les éléments sont "privés".
typedef struct SmoothFlagE {
    int16_t    _t;
//    int16_t    _D;
    uint16_t   _flags;
//    uint16_t   _sub;
} SmoothFlagE;

SmoothFlagE SmoothFlagE_new(bool isOn, uint16_t transTimeTicksOpt);
void        SmoothFlagE_setDefaultTransTimeTicks(uint16_t transTimeTicks);

// Setters et getter du smooth flag ON/OFF.
float smoothflagE_setOn(SmoothFlagE * st);
float smoothflagE_setOff(SmoothFlagE * st);
float smoothflagE_value(SmoothFlagE * st);
// Estimation au temps future (ne met pas à jour l'état)
float smoothflagE_valueNext(SmoothFlagE const* sf);

// Le temps est basé sur le chrono des events... OK ?
#define SFE_Chronos_elapsedMS (int16_t)ChronosEvent.event_elapsedMS
#define SFE_Chronos_elapsedNextMS (int16_t)(ChronosEvent.event_elapsedMS + ChronosEvent.deltaTMS)

#endif
