//
//  systems/system_sound.h
//  Pour jouer des sons. 
//  Utilise OpenAL ou AVFoundation, voir le .c ou .m.
//  (Inclure soit le .m soit le .c, mais pas les deux...)
//
//  Created by Corentin Faucher on 2023-10-31.
//
#ifndef COQ_SOUND_H
#define COQ_SOUND_H
#include "../utils/util_base.h"

#define Sound_volume_count 5

typedef struct SoundBuffer {
    uint32_t const sampleCount;
    uint32_t const sampleRate; // (en Hz, e.g. 22050 Hz)
    size_t   const bufferSize; // (2x sampleCount)
    int16_t        buffer[1];
} SoundBuffer;
SoundBuffer* SoundBuffer_createEmpty(uint32_t sampleCount, uint32_t sampleRate);
void         soundbuffer_test_print(SoundBuffer* sb);

extern bool Sound_isMute; // = false

void  Sound_initWithWavNames(const char** wav_namesOpt, 
                             uint32_t wav_count, uint32_t extraSound_count);
/// Définir l'audio data d'un son "extra" (pas un wav loadé).
/// extraId doit être entre `wav_count` et `wav_count + extraSound_count`.
void  Sound_giveAudioBufferForExtraSound(uint32_t extraId, SoundBuffer** bufferGivenRef); 
void  Sound_play(uint32_t soundId, float volume, int pitch, uint32_t volumeId);

uint32_t Sound_firstExtraId(void);


void Sound_musicStart(void (*const music_callback)(void));
void Sound_musicStop(void);
void Sound_musicSetBeat(uint32_t bpm);

void  Sound_resume_(void);
void  Sound_suspend_(void);

#endif /* sound_h */
