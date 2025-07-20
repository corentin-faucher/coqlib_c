//
//  coq_sound_openal.c
//  Version OpenAL pour les sons.
//  Ne semble pas nécessaire de faire une thread séparer pour jouer les sons ?
//  (pas de lag observé...)
//
//  Created by Corentin Faucher on 2023-10-31.
//
#include "system_sound.h"

#include "../maths/math_base.h"
#include "../systems/system_file.h"
#include "../utils/util_chrono.h"

#include <math.h>
#include <pthread.h>

#ifdef __APPLE__
#define AL_SILENCE_DEPRECATION
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#ifdef __linux__
#include <AL/al.h>
#include <AL/alc.h>
#endif

SoundBuffer* SoundBuffer_createEmpty(uint32_t sampleCount, uint32_t sampleRate) {
    SoundBuffer*const buffer = coq_callocArray(SoundBuffer, int16_t, sampleCount);
    uint_initConst(&buffer->sampleCount, sampleCount);
    uint_initConst(&buffer->sampleRate, sampleRate);
    size_initConst(&buffer->bufferSize, sizeof(int16_t)*sampleCount);
    return buffer;
}
void         soundbuffer_test_print(SoundBuffer*const sb) {
    if(!sb) { printerror("No sound buffer."); return; }
    printf("🦫 Sound buffer : sample rate %d, count %d, samples :\n  ",
           sb->sampleRate, sb->sampleCount);
    uint32_t end = uminu(100, sb->sampleCount);
    for(int i = 0; i < end; i++)
        printf("%d ", sb->buffer[i]);
    printf("\n");
}

static ALCdevice*   AL_device_ = NULL;
static ALCcontext*  AL_context_ = NULL;
static ALuint*      AL_buffer_ids_ = NULL;
static ALuint*      AL_source_ids_ = NULL;

bool             Sound_isMute = false;
static float     volumes_[Sound_volume_count] = {
    1, 1, 1, 1, 1
};

static const char**   wav_namesOpt_ = NULL;
static uint32_t       waves_count_ = 0;
static uint32_t       extra_count_ = 0;
static uint32_t       total_count_ = 0;
static uint32_t       sound_sampleRateOpt_ = 0;
static SoundBuffer**  extraSoundBuffers = NULL;
void Sound_freeExtraBuffers_(void) {
    if(!extraSoundBuffers) return;
    for(int i = 0; i < extra_count_; i++) {
        if(extraSoundBuffers[i])
            coq_free(extraSoundBuffers[i]);
    }
    coq_free(extraSoundBuffers);
    extraSoundBuffers = NULL;
}

/// Header d'un fichier .wav. 36 bytes.
struct WavHeader_ {
    char     chunkId[4];
    uint32_t chunkSize;
    char     format[4];
    char     subChunkId[4];
    uint32_t subChunkSize;
    uint16_t formatTag;
    uint16_t channels;
    uint32_t samplesPerSec;
    uint32_t avgBytesPerSec;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
}  __attribute__((packed));
SoundBuffer* SoundBuffer_createOptFromWavFile_(const char* wavName) {
    // Ouverture du fichier wav
    const char* wav_path = FileManager_getResourcePathOpt(wavName, "wav", "wavs");
    guard_let(FILE*, f, fopen(wav_path, "rb"), 
              printerror("Cannot open %s.", wav_path), NULL)
    // Lecture du header
    struct WavHeader_ header;
    fread(&header, sizeof(struct WavHeader_), 1, f);
    // Chercher le text "data" après le header (de 36 bytes)
    char text_data[5] = {0};
    bool text_data_found = false;
    int pos = 36;
    while(pos < 60) {
        fseek(f, pos, SEEK_SET);
        fread(text_data, 5, 1, f);
        if(strncmp(text_data, "data", 4) == 0) {
            text_data_found = true;
            break;
        }
        pos ++;
    }
    if(!text_data_found) {
        printerror("No string `data` in wav.");
        fclose(f);
        return NULL;
    }
    // Lire les données juste après `data`.
    fseek(f, pos+4, SEEK_SET);
    uint32_t bufferSize;
    fread(&bufferSize, 4, 1, f);
    uint32_t sampleCount = bufferSize / sizeof(int16_t);
    
    SoundBuffer* buffer = SoundBuffer_createEmpty(sampleCount, header.samplesPerSec);
    fread(buffer->buffer, bufferSize, 1, f);
    fclose(f);
    return buffer;
}
void Sound_checkSampleRate_(uint32_t const sampleRate) {
    if(!sound_sampleRateOpt_) {
        sound_sampleRateOpt_ = sampleRate;
    } else if(sound_sampleRateOpt_ != sampleRate) {
        printwarning("Various sample rate in wavs %d and %d.",
        sound_sampleRateOpt_, sampleRate);
    }
}

void  Sound_musicResume_(void);
void  Sound_resume_(void) {
    if(AL_device_) return;
    // Device et context OpenAL
    ALCdevice*const device = alcOpenDevice(NULL);
    AL_context_ = alcCreateContext(device, NULL);
    alcMakeContextCurrent(AL_context_);
    // Buffers des sons
    if(total_count_) {
        AL_buffer_ids_ = calloc(total_count_, sizeof(ALuint));
        AL_source_ids_ = calloc(total_count_, sizeof(ALuint));
        alGenBuffers(total_count_, AL_buffer_ids_);
        alGenSources(total_count_, AL_source_ids_);
        for(int i = 0; i < total_count_; i ++) {
            if(i < waves_count_) {
                with_beg(SoundBuffer, buffer, SoundBuffer_createOptFromWavFile_(wav_namesOpt_[i]))
                Sound_checkSampleRate_(buffer->sampleRate);
                alBufferData(AL_buffer_ids_[i], AL_FORMAT_MONO16, buffer->buffer, 
                    (int)buffer->bufferSize, buffer->sampleRate);
                with_end(buffer)
            } else if(extraSoundBuffers[i - waves_count_]) {
                SoundBuffer*const buffer = extraSoundBuffers[i - waves_count_];
                Sound_checkSampleRate_(buffer->sampleRate);
                alBufferData(AL_buffer_ids_[i], AL_FORMAT_MONO16, buffer->buffer, 
                    (int)buffer->bufferSize, buffer->sampleRate);
            }
            alSourcei(AL_source_ids_[i], AL_BUFFER, AL_buffer_ids_[i]);
        }
    }
    // Prêt...
    AL_device_ = device;
    Sound_musicResume_();
}

void  Sound_musicSuspend_(void);
void  Sound_suspend_(void) {
    if(!AL_device_) return;
    Sound_musicSuspend_();
    ALCdevice*const device = AL_device_;
    AL_device_ = NULL;
    // 1. Delier sources et buffer (utile ?)
    if(total_count_) {
        for(int i = 0; i < total_count_; i ++) {
            alSourcei(AL_source_ids_[i], AL_BUFFER, 0);
        }
        // 2. Effacer les sources et les buffers.
        alDeleteBuffers(total_count_, AL_buffer_ids_);
        alDeleteSources(total_count_, AL_source_ids_);
        free(AL_buffer_ids_);
        free(AL_source_ids_);
        AL_buffer_ids_ = NULL;
        AL_source_ids_ = NULL;
    }
    // 3. Defaire context et device
    alcMakeContextCurrent(NULL);
    alcDestroyContext(AL_context_);
    AL_context_ = NULL;
    alcCloseDevice(device);
}

void  Sound_initWithWavNames(const char**const wav_namesOpt, 
            uint32_t const wav_count, uint32_t const extraSound_count) 
{
    if(AL_device_) { printerror("Sound active."); return; }
    if(wav_namesOpt && wav_count) {
        wav_namesOpt_ = wav_namesOpt;
        waves_count_ = wav_count;
    } else {
        if(wav_namesOpt || wav_count)
            printerror("Missing name or count to init wavs.");
        wav_namesOpt_ = NULL;
        waves_count_ = 0;
    }
    Sound_freeExtraBuffers_();
    extra_count_ = extraSound_count;
    if(extra_count_) extraSoundBuffers = coq_callocSimpleArray(extra_count_, SoundBuffer*);
    total_count_ = waves_count_ + extra_count_;
    
    Sound_resume_();
}

void  Sound_giveAudioBufferForExtraSound(uint32_t const extraId, SoundBuffer**const bufferGivenRef) 
{
    if(!extraSoundBuffers || extraId < waves_count_ || extraId >= total_count_) {
        printerror("extraId %d overflow.", extraId); return;
    }
    guard_let(SoundBuffer*, buffer, *bufferGivenRef, printerror("No buffer."), )
    *bufferGivenRef = NULL;
    Sound_checkSampleRate_(buffer->sampleRate);
    if(extraSoundBuffers[extraId - waves_count_]) {
        coq_free(extraSoundBuffers[extraId - waves_count_]);
        extraSoundBuffers[extraId - waves_count_] = NULL;
    }
    extraSoundBuffers[extraId - waves_count_] = buffer;
    // Si live, update OpenAL buffer.
    if(!AL_device_) return;
    printdebug("🐶");
    alBufferData(AL_buffer_ids_[extraId], AL_FORMAT_MONO16, buffer->buffer, 
                    (int)buffer->bufferSize, buffer->sampleRate);
    alSourcei(AL_source_ids_[extraId], AL_BUFFER, AL_buffer_ids_[extraId]);
}

void  Sound_play(uint32_t const soundId, float volume, int pitch, uint32_t volumeId) {
    if(!AL_device_) { printwarning("Sound suspended."); return; }
    if(volumeId >= Sound_volume_count) {
        printerror("Volume id overflow %d.", volumeId);
        return;
    }
    if(soundId >= total_count_) {
        printerror("SoundId overflow %d, count %d.", soundId, total_count_);
        return;
    }
    if(Sound_isMute || volumes_[volumeId] < 0.01)
        return;
//    printdebug("Playing sound %d : gain %f, pitch %f.", 
//        soundId, volume * volumes_[volumeId], powf(2.f,(float)pitch/12.f));
    alSourcef(AL_source_ids_[soundId], AL_GAIN, volume * volumes_[volumeId]);
    alSourcef(AL_source_ids_[soundId], AL_PITCH, powf(2.f,(float)pitch/12.f));
    alSourcePlay(AL_source_ids_[soundId]);
}

uint32_t Sound_firstExtraId(void) {
    if(total_count_ <= waves_count_) {
        printerror("No extra sound.");
    }
    return waves_count_;
}

// MARK: - Musique
static volatile bool Music_ON_ = false;      // Signal pour continuer à jouer.
static bool Music_shouldPlay_ = false;
static volatile bool Music_Playing_ = false; // Est en train de jouer.
static pthread_t Music_thread_ = {};
static int64_t Music_beatDTMS_ = 300; 
static int64_t Music_maxSleepMS = 50;
static void (*Music_callback_)(void) = NULL;
#include <inttypes.h>
void* music_loop_(void* unused_) {
    Music_Playing_ = true;
    void (*const callback)(void) = Music_callback_;
    if(!callback) { 
        printerror("Music callback not set.");
        goto quit_playing_music;
    }
    int64_t timeMS = Chrono_systemTimeMS_();
    PRId64;
    while(Music_ON_) {
        // Jouer l'instant présent.
        callback();
        // Attendre le prochain beat.
        timeMS += Music_beatDTMS_;
        int64_t sleepTimeMS = timeMS - Chrono_systemTimeMS_();
        if(sleepTimeMS < 1) {
            printerror("Bad sleep time %d.", (int)sleepTimeMS);
            sleepTimeMS = 1;
        }
        while(sleepTimeMS && Music_ON_) {
            int64_t subSleepTime = sleepTimeMS;
            if(subSleepTime > Music_maxSleepMS + 1) {
                subSleepTime = Music_maxSleepMS;
            }
            sleepTimeMS -= subSleepTime;
            struct timespec time = {0, subSleepTime*ONE_MILLION};
            nanosleep(&time, NULL);
        }
    }
quit_playing_music:
    Music_Playing_ = false;
    return NULL;
}

void Sound_musicResume_(void) {
    if(!Music_shouldPlay_) return;
    if(Music_ON_) {
        printwarning("Already on.");
        return; 
    }
    if(Music_Playing_) {
        printwarning("Already playing.");
        // TODO: Retry later ?
        return;
    }
    Music_ON_ = true;
    pthread_create(&Music_thread_, NULL, music_loop_, NULL);
}

void Sound_musicSuspend_(void) {
    if(!Music_ON_) return;
    Music_ON_ = false;
    pthread_join(Music_thread_, NULL);
}

void Sound_musicStart(void (*const music_callback)(void)) {
    Music_shouldPlay_ = false;
    Sound_musicSuspend_();
    if(!music_callback) {
        printerror("No music callback.");
        return;
    }
    Music_shouldPlay_ = true;
    Music_callback_ = music_callback;
    Sound_musicResume_();
}
void Sound_musicStop(void) {
    Music_shouldPlay_ = false;
    Sound_musicSuspend_();
}

void Sound_musicSetBeat(uint32_t bpm) {
    if(bpm > 500) {
        printwarning("Music bpm too high %d.", bpm);
        bpm = 500;
    }
    if(!bpm) Music_beatDTMS_ = 300;
    else Music_beatDTMS_ = 60000.f / (float)bpm;
}



#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
