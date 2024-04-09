//
//  coq_sound.h
//  Pour jouer des sons. 
//  (Utilise OpenAL ou AVFoundation, voir le .c ou .m)
//
//  Created by Corentin Faucher on 2023-10-31.
//
#ifndef COQ_SOUND_H
#define COQ_SOUND_H
#include "utils/utils_base.h"
#include <stdbool.h>

#define Sound_volume_count 5

extern bool Sound_isMute; // = false

void  Sound_initWithWavNames(const char* wav_names[], uint32_t wav_count);
void  Sound_play(uint32_t soundId, float volume, int pitch, uint32_t volumeId);

void  Sound_resume(void);
void  Sound_suspend(void);

#endif /* sound_h */
