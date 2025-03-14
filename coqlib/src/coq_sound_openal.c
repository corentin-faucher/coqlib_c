//
//  coq_sound_openal.c
//  Version OpenAL pour les sons.
//  Ne semble pas nécessaire de faire une thread séparer pour jouer les sons ?
//  (pas de lag observé...)
//
//  Created by Corentin Faucher on 2023-10-31.
//
#include "coq_sound.h"
#include "utils/util_file.h"
#include "coq_chrono.h"

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

static ALCdevice*   AL_device_ = NULL;
static ALCcontext*  AL_context_ = NULL;
static ALuint*      AL_buffer_ids_ = NULL;
static ALuint*      AL_source_ids_ = NULL;

bool             Sound_isMute = false;
static float     volumes_[Sound_volume_count] = {
    1, 1, 1, 1, 1
};

static const char**   wav_namesOpt_ = NULL;
static uint32_t       wav_count_ = 0;

/// Header d'un fichier .wav. 36 bytes.
typedef struct {
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
} WavHeader;

void Sound_alSetAudioBuffer_(ALuint buffer_id, const char* wavName) {
    // Ouverture du fichier wav
    const char* wav_path = FileManager_getResourcePathOpt(wavName, "wav", "wavs");
    FILE* f = fopen(wav_path, "rb");
    if(!f) { printerror("Cannot open %s.", wav_path); return; }
    // Lecture du header
    WavHeader header;
    fread(&header, sizeof(WavHeader), 1, f);
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
        return;
    }
    // Lire les données juste après `data`.
    fseek(f, pos+4, SEEK_SET);
    uint32_t data_size;
    fread(&data_size, 4, 1, f);
    char* wav_buffer = calloc(1, data_size);
    fread(wav_buffer, data_size, 1, f);
    fclose(f);
    // Donner ces data a OpenAL
    alBufferData(buffer_id, AL_FORMAT_MONO16, wav_buffer, data_size, header.samplesPerSec);

    free(wav_buffer);
}

void  Sound_musicResume_(void);
void  Sound_resume(void) {
    if(AL_device_) return;
    // Device et context OpenAL
    ALCdevice*const device = alcOpenDevice(NULL);
    AL_context_ = alcCreateContext(device, NULL);
    alcMakeContextCurrent(AL_context_);
    // Buffers des sons
    AL_buffer_ids_ = calloc(wav_count_, sizeof(ALuint));
    AL_source_ids_ = calloc(wav_count_, sizeof(ALuint));
    alGenBuffers(wav_count_, AL_buffer_ids_);
    alGenSources(wav_count_, AL_source_ids_);
    for(int i = 0; i < wav_count_; i ++) {
        Sound_alSetAudioBuffer_(AL_buffer_ids_[i], wav_namesOpt_[i]);
        alSourcei(AL_source_ids_[i], AL_BUFFER, AL_buffer_ids_[i]);
    }
    // Prêt...
    AL_device_ = device;
    Sound_musicResume_();
}

void  Sound_musicSuspend_(void);
void  Sound_suspend(void) {
    if(!AL_device_) return;
    Sound_musicSuspend_();
    ALCdevice*const device = AL_device_;
    AL_device_ = NULL;
    // 1. Delier sources et buffer (utile ?)
    for(int i = 0; i < wav_count_; i ++) {
        alSourcei(AL_source_ids_[i], AL_BUFFER, 0);
    }
    // 2. Effacer les sources et les buffers.
    alDeleteBuffers(wav_count_, AL_buffer_ids_);
    alDeleteSources(wav_count_, AL_source_ids_);
    free(AL_buffer_ids_);
    free(AL_source_ids_);
    AL_buffer_ids_ = NULL;
    AL_source_ids_ = NULL;
    // 3. Defaire context et device
    alcMakeContextCurrent(NULL);
    alcDestroyContext(AL_context_);
    AL_context_ = NULL;
    alcCloseDevice(device);
}

void  Sound_initWithWavNames(const char** wav_namesOpt, uint32_t wav_count) {
    if(wav_namesOpt && wav_count) {
        wav_namesOpt_ = wav_namesOpt;
        wav_count_ = wav_count;
    } else if(wav_namesOpt || wav_count) {
        printerror("Missing name or count to init wavs.");
    }
    Sound_resume();
}

void  Sound_play(uint32_t const soundId, float volume, int pitch, uint32_t volumeId) {
    if(!AL_device_) { printwarning("Sound suspended."); return; }
    if(volumeId >= Sound_volume_count) {
        printerror("Volume id overflow %d.", volumeId);
        return;
    }
    if(soundId >= wav_count_) {
        printerror("SoundId overflow %d, wav_count %d.", soundId, wav_count_);
        return;
    }
    if(Sound_isMute || volumes_[volumeId] < 0.01)
        return;
    alSourcef(AL_source_ids_[soundId], AL_GAIN, volume * volumes_[volumeId]);
    alSourcef(AL_source_ids_[soundId], AL_PITCH, powf(2.f,(float)pitch/12.f));
    alSourcePlay(AL_source_ids_[soundId]);
}

// MARK: - Musique
static volatile bool Music_ON_ = false;      // Signal pour continuer à jouer.
static bool Music_shouldPlay_ = false;
static volatile bool Music_Playing_ = false; // Est en train de jouer.
static pthread_t Music_thread_ = {};
static int64_t Music_beatDTMS_ = 300; 
static int64_t Music_maxSleepMS = 50;
static void (*Music_callback_)(void) = NULL;

void* music_loop_(void* unused_) {
    Music_Playing_ = true;
    void (*const callback)(void) = Music_callback_;
    if(!callback) { 
        printerror("Music callback not set.");
        goto quit_playing_music;
    }
    int64_t timeMS = Chrono_systemTimeMS_();
    
    while(Music_ON_) {
        // Jouer l'instant présent.
        callback();
        // Attendre le prochain beat.
        timeMS += Music_beatDTMS_;
        int64_t sleepTimeMS = timeMS - Chrono_systemTimeMS_();
        if(sleepTimeMS < 1) {
            printerror("Bad sleep time %lld.", sleepTimeMS);
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
