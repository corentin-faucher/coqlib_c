//
//  coq_sound.h
//  Pour jouer des sons. 
//  Utilise OpenAL ou AVFoundation, voir le .c ou .m.
//  (Inclure soit le .m soit le .c, mais pas les deux...)
//
//  Created by Corentin Faucher on 2023-10-31.
//
#ifndef COQ_SOUND_H
#define COQ_SOUND_H
#include "utils/util_base.h"
#include <stdbool.h>

#define Sound_volume_count 5

extern bool Sound_isMute; // = false

void  Sound_initWithWavNames(const char** wav_namesOpt, uint32_t wav_count);
void  Sound_play(uint32_t soundId, float volume, int pitch, uint32_t volumeId);

void  Sound_resume(void);
void  Sound_suspend(void);

void Sound_musicStart(void (*const music_callback)(void));
void Sound_musicStop(void);
void Sound_musicSetBeat(uint32_t bpm);

#endif /* sound_h */
