// Version OpenAL pour les sons.

#include "../coq_sound.h"
#include "../maths/math_chrono.h"
#include "../utils/utils_file.h"

#ifdef __APPLE__
#define AL_SILENCE_DEPRECATION
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
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

static const char**   wav_names_ = NULL;
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

void  Sound_resume(void) {
    if(AL_device_) return;
    if(!wav_count_) {
        printdebug("Sound: nothing to resume.");
        return;
    }
    // Device et context OpenAL
    AL_device_ =  alcOpenDevice(NULL);
    AL_context_ = alcCreateContext(AL_device_, NULL);
    alcMakeContextCurrent(AL_context_);
    // Buffers des sons
    AL_buffer_ids_ = calloc(wav_count_, sizeof(ALuint));
    AL_source_ids_ = calloc(wav_count_, sizeof(ALuint));
    alGenBuffers(wav_count_, AL_buffer_ids_);
    alGenSources(wav_count_, AL_source_ids_);
    for(int i = 0; i < wav_count_; i ++) {
        Sound_alSetAudioBuffer_(AL_buffer_ids_[i], wav_names_[i]);
        alSourcei(AL_source_ids_[i], AL_BUFFER, AL_buffer_ids_[i]);
    }
}

void  Sound_suspend(void) {
    if(!AL_device_) return;
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
    alcCloseDevice(AL_device_);
    AL_device_ = NULL;
}

void  Sound_initWithWavNames(const char* wav_names[], uint32_t wav_count) {
    wav_names_ = wav_names;
    wav_count_ = wav_count;
    Sound_resume();
}

void  Sound_play(uint32_t const soundId, float volume, int pitch, uint32_t volumeId) {
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
