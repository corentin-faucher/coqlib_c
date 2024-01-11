// Version OpenAL pour les sons.

#include "coq_sound.h"
#include "_math/_math_chrono.h"
#include "_utils/_utils_file.h"

#ifdef __APPLE__
#define AL_SILENCE_DEPRECATION
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif
#ifdef __linux__
#include <AL/al.h>
#include <AL/alc.h>
#endif



static ALCdevice*   _AL_device = NULL;
static ALCcontext*  _AL_context = NULL;
static ALuint*      _AL_buffer_ids = NULL;
static ALuint*      _AL_source_ids = NULL;

bool             Sound_isMute = false;
static float     _volumes[Sound_volume_count] = {
    1, 1, 1, 1, 1
};

static const char**   _wav_names = NULL;
static uint32_t       _wav_count = 0;

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

void _Sound_alSetAudioBuffer(ALuint buffer_id, const char* wavName) {
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

void  _Sound_resume(void) {
    if(_AL_device) return;
    if(!_wav_count) {
        printdebug("Sound: nothing to resume.");
        return;
    }
    // Device et context OpenAL
    _AL_device =  alcOpenDevice(NULL);
    _AL_context = alcCreateContext(_AL_device, NULL);
    alcMakeContextCurrent(_AL_context);
    // Buffers des sons
    _AL_buffer_ids = calloc(_wav_count, sizeof(ALuint));
    _AL_source_ids = calloc(_wav_count, sizeof(ALuint));
    alGenBuffers(_wav_count, _AL_buffer_ids);
    alGenSources(_wav_count, _AL_source_ids);
    for(int i = 0; i < _wav_count; i ++) {
        _Sound_alSetAudioBuffer(_AL_buffer_ids[i], _wav_names[i]);
        alSourcei(_AL_source_ids[i], AL_BUFFER, _AL_buffer_ids[i]);
    }
}

void  _Sound_suspend(void) {
    if(!_AL_device) return;
    // 1. Delier sources et buffer (utile ?)
    for(int i = 0; i < _wav_count; i ++) {
        alSourcei(_AL_source_ids[i], AL_BUFFER, 0);
    }
    // 2. Effacer les sources et les buffers.
    alDeleteBuffers(_wav_count, _AL_buffer_ids);
    alDeleteSources(_wav_count, _AL_source_ids);
    free(_AL_buffer_ids);
    free(_AL_source_ids);
    _AL_buffer_ids = NULL;
    _AL_source_ids = NULL;
    // 3. Defaire context et device
    alcMakeContextCurrent(NULL);
    alcDestroyContext(_AL_context);
    _AL_context = NULL;
    alcCloseDevice(_AL_device);
    _AL_device = NULL;
}

void  Sound_initWithWavNames(const char* wav_names[], uint32_t wav_count) {
    _wav_names = wav_names;
    _wav_count = wav_count;
    _Sound_resume();
}

void  Sound_play(uint32_t const soundId, float volume, int pitch, uint32_t volumeId) {
    if(volumeId >= Sound_volume_count) {
        printerror("Volume id overflow %d.", volumeId);
        return;
    }
    if(soundId >= _wav_count) {
        printerror("SoundId overflow %d, wav_count %d.", soundId, _wav_count);
        return;
    }
    if(Sound_isMute || _volumes[volumeId] < 0.01)
        return;
    alSourcef(_AL_source_ids[soundId], AL_GAIN, volume * _volumes[volumeId]);
    alSourcef(_AL_source_ids[soundId], AL_PITCH, powf(2.f,(float)pitch/12.f));
    alSourcePlay(_AL_source_ids[soundId]);
}
