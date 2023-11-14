//
//  sound.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-31.
//

#ifndef sound_h
#define sound_h
#include "utils.h"

extern const  uint32_t Sound_volume_count; // == 5
extern const  Bool     Sound_isMute;

void  Sound_initWithWavNames(const char* wav_names[], uint32_t wav_count);
void  Sound_play(uint32_t soundId, float volume, int pitch, uint32_t volumeId);

void  _Sound_resume(void);
void  _Sound_suspend(void);

#endif /* sound_h */
