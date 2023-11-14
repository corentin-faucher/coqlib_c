//
//  sound.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-31.
//

#import <AVFoundation/AVFoundation.h>
#include "sound.h"
#include "chronometers.h"

const  Bool      Sound_isMute = false;
const  uint32_t  Sound_volume_count = 5;
static float     _volumes[Sound_volume_count] = {
    1, 1, 1, 1, 1
};

static AVAudioEngine*        _engine = nil;
static dispatch_queue_t      _sound_queue = NULL;

typedef struct {
    AVAudioPlayerNode*    audio_player;
    AVAudioUnitTimePitch* pitch_controller;
    int64_t               expirationTimeMS;
} _Player;
const uint32_t               _player_count = 5;
static _Player               _players[_player_count];

typedef struct {
    size_t            count;
    AVAudioPCMBuffer* buffers[1];
} _BufferArray;
static const char**           _wav_names = NULL;
static uint32_t               _wav_count = 0;
static _BufferArray*          _buffers = NULL;

AVAudioPCMBuffer* _Sound_createBuffer(NSString* wavName, AVAudioFormat* expectedFormat) {
    // 1. Loader le wav.
    NSURL* url = [NSBundle.mainBundle URLForResource:wavName withExtension:@"wav" subdirectory:@"wavs"];
    if(url == nil) {
        NSLog(@"Cannot get sound url for %@.", wavName);
        return nil;
    }
    AVAudioFile* file = [[AVAudioFile alloc] initForReading:url error:nil];
    url = nil;
    if(file == nil) {
        NSLog(@"Cannot load sound %@.", wavName);
        return nil;
    }
    // 2. En faire un audio buffer.
    uint32_t count = (uint32_t)file.length;
    AVAudioFormat* format = file.processingFormat;
    if(expectedFormat != nil) if(![format isEqual:expectedFormat]) {
        NSLog(@"%@ not the same format as others.", wavName);
        file = nil;
        format = nil;
        return nil;
    }
    
    AVAudioPCMBuffer* buffer = [[AVAudioPCMBuffer alloc]
                                initWithPCMFormat:format frameCapacity:count];
    format = nil;
    if(buffer == nil) {
        NSLog(@"Can't init buffer for %@.", wavName);
        file = nil;
        return nil;
    }
    NSError *error = nil;
    [file readIntoBuffer:buffer error:&error];
    file = nil;
    if(error != nil) {
        NSLog(@"Can't read file into buffer for %@.", wavName);
        return nil;
    }
    return buffer;
}

void  _Sound_resume(void) {
    if(_engine != nil) { return; }
    if(_wav_names == NULL || _wav_count == 0) {
        printerror("No wav sound to load."); return;
    }
    // 1. Buffers
    size_t size_buffer_array = sizeof(_BufferArray) + sizeof(AVAudioPCMBuffer*) * (_wav_count - 1);
    _buffers = coq_calloc(1, size_buffer_array);
    _buffers->count = _wav_count;
    NSString* wavName = [NSString stringWithUTF8String:_wav_names[0]];
    _buffers->buffers[0] = _Sound_createBuffer(wavName, nil);
    AVAudioFormat* format = _buffers->buffers[0].format;
    for(uint32_t sndId = 1; sndId < _wav_count; sndId ++) {
        NSString* wavName = [NSString stringWithUTF8String:_wav_names[sndId]];
        _buffers->buffers[sndId] = _Sound_createBuffer(wavName, format);
        wavName = nil;
    }
    // 2. Engine
    _engine = [[AVAudioEngine alloc] init];
    // 3. Attach/connect 5 AudioPlayers with their PitchControler (On peut jouer jusqu'à 5 sons en même temps...)
    for(uint32_t playerId = 0; playerId < _player_count; playerId ++) {
        _players[playerId].audio_player = [[AVAudioPlayerNode alloc] init];
        _players[playerId].pitch_controller = [[AVAudioUnitTimePitch alloc] init];
        _players[playerId].expirationTimeMS = ChronoApp_elapsedMS();
        [_engine attachNode:_players[playerId].audio_player];
        [_engine attachNode:_players[playerId].pitch_controller];
        [_engine connect:_players[playerId].audio_player
                      to:_players[playerId].pitch_controller format:format];
        [_engine connect:_players[playerId].pitch_controller
                      to:_engine.mainMixerNode format:format];
    }
    // (superflu?)
    [_engine connect:[_engine mainMixerNode] to:[_engine outputNode] format:format];
    // 4. Prepare sound engine
    [_engine prepare];
    // 5. Sound queue
    if(_sound_queue == NULL)
        _sound_queue = dispatch_queue_create("sound.queue", NULL);
}

void  _Sound_suspend(void) {
    if(_engine == nil) {
        printwarning("Sound already deinit."); return;
    }
    [_engine stop];
    _engine = nil;
    
    for(uint32_t playerId = 0; playerId < _player_count; playerId ++) {
        _players[playerId].audio_player = nil;
        _players[playerId].pitch_controller = nil;
    }
    
    if(_buffers) {
        for(int i = 0; i < _buffers->count; i++) {
            _buffers->buffers[i] = nil;
        }
        coq_free(_buffers);
        _buffers = NULL;
    }
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
    if(Sound_isMute || _volumes[volumeId] < 0.01 || _engine == nil)
        return;
    
    dispatch_async(_sound_queue, ^{
        if(![_engine isRunning]) {
            NSError* error;
            [_engine startAndReturnError:&error];
            if(error != nil) {
                printerror("Cannot start sound engine.");
                return;
            }
        }
        if(_buffers->buffers[soundId] == nil) {
            printerror("Sound buffer not inif for sound %d.", soundId);
            return;
        }
        // 1. Trouver un player disponible...
//        uint32_t bestPlayerId = 0;
//        int64_t  bestRemainingMS = 20000;
        _Player* player = NULL;
//        Bool     foundFree = false;
        for(uint32_t playerId = 0; playerId < _player_count; playerId ++) {
            int64_t remaining = _players[playerId].expirationTimeMS - ChronoApp_elapsedMS();
            // Ok, trouve...
            if(remaining <= 0) {
                player = &_players[playerId];
//                bestPlayerId = playerId;
//                foundFree = true;
                break;
            }
            // Sinon, on enregistre le meilleur jusqu'à présent.
            // Superflu ?
//            if(remaining < bestRemainingMS) {
//                bestRemainingMS = remaining;
//                bestPlayerId = playerId;
//            }
        }
        if(!player) {
            printwarning("No available player.");
            return;
        }
        if(player->audio_player == nil || player->pitch_controller == nil) {
            printerror("Player not init.");
            return;
        }
        // 3. Mise à jour du expiredTime (en ms)
        AVAudioPCMBuffer* buffer = _buffers->buffers[soundId];
        double durationSec = (double)buffer.frameLength / buffer.format.sampleRate;
        player->expirationTimeMS = ChronoApp_elapsedMS() + (int64_t)(durationSec * 1000.0 + 300.0);
        // 4. Préparation du player et pitchControl
        if(player->audio_player.isPlaying) {
            [player->audio_player stop];
        }
        player->pitch_controller.pitch = (float)(pitch * 100);
        player->audio_player.volume = volume * _volumes[volumeId];
        [player->audio_player scheduleBuffer:_buffers->buffers[soundId] completionHandler:nil];
        // 5. Jouer le son !
        [player->audio_player play];
    });
}
