//
//  coq_sound.m
//  Version AVAudio pour jouer des sons.
//  Utiliser de préférence OpenAL (semble moins bogué... même si `deprecated`)
//
//  Created by Corentin Faucher on 2023-10-31.
//


#include "coq_sound.h"

#import <AVFoundation/AVFoundation.h>
#include "math_chrono.h"


bool             Sound_isMute = false;
static float     _volumes[Sound_volume_count] = {
    1, 1, 1, 1, 1
};

static AVAudioEngine    *avEngine_ = nil;
// La thread pour les sons. (Fait lagger quand on part un son)
static dispatch_queue_t  coqSound_dispatch_queue_ = NULL;

struct Player_ {
    AVAudioPlayerNode*    audio_player;
    AVAudioUnitTimePitch* pitch_controller;
    int64_t               expirationTimeMS;
};
const uint32_t               player_count_ = 5;
static struct Player_        players_[player_count_] = {};

typedef struct {
    size_t            count;
    AVAudioPCMBuffer* buffers[1];
} BufferArray_;
static const char**           Sound_wavNames_ = NULL;
static uint32_t               Sound_wavCount_ = 0;
static BufferArray_*          Sound_buffers_ = NULL;

AVAudioPCMBuffer* Sound_createBuffer_(NSString* wavName, AVAudioFormat* expectedFormat) {
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

void  Sound_resume(void) {
    if(avEngine_ != nil) { return; }
    if(Sound_wavNames_ == NULL || Sound_wavCount_ == 0) {
        printerror("No wav sound to load."); return;
    }
    // 1. Buffers
    Sound_buffers_ = coq_callocArray(BufferArray_, AVAudioPCMBuffer*, Sound_wavCount_);
    Sound_buffers_->count = Sound_wavCount_;
    NSString* wavName = [NSString stringWithUTF8String:Sound_wavNames_[0]];
    Sound_buffers_->buffers[0] = Sound_createBuffer_(wavName, nil);
    AVAudioFormat* format = Sound_buffers_->buffers[0].format;
    for(uint32_t sndId = 1; sndId < Sound_wavCount_; sndId ++) {
        NSString* wavName = [NSString stringWithUTF8String:Sound_wavNames_[sndId]];
        Sound_buffers_->buffers[sndId] = Sound_createBuffer_(wavName, format);
        wavName = nil;
    }
    // 2. Engine
    AVAudioEngine* engine = [[AVAudioEngine alloc] init];
    // 3. Attach/connect 5 AudioPlayers with their PitchControler (On peut jouer jusqu'à 5 sons en même temps...)
    for(uint32_t playerId = 0; playerId < player_count_; playerId ++) {
        players_[playerId].audio_player = [[AVAudioPlayerNode alloc] init];
        players_[playerId].pitch_controller = [[AVAudioUnitTimePitch alloc] init];
        players_[playerId].expirationTimeMS = ChronoApp_elapsedMS();
        [engine attachNode:players_[playerId].audio_player];
        [engine attachNode:players_[playerId].pitch_controller];
        [engine connect:players_[playerId].audio_player
                      to:players_[playerId].pitch_controller format:format];
        [engine connect:players_[playerId].pitch_controller
                      to:engine.mainMixerNode format:format];
    }
    // (superflu?)
    [engine connect:[engine mainMixerNode] to:[engine outputNode] format:format];
    // 4. Prepare sound engine
    [engine prepare];
    // 5. Sound queue
    if(coqSound_dispatch_queue_ == NULL)
        coqSound_dispatch_queue_ = dispatch_queue_create("sound.queue", NULL);
    // 6. Ok, fini -> set engine
    avEngine_ = engine;
}

void  Sound_suspend(void) {
    if(avEngine_ == nil) {
        printwarning("Sound already deinit."); return;
    }
    // Suspendre dans la thread sound_queue (pour pas faire boguer un sound_play)
    dispatch_async(coqSound_dispatch_queue_, ^{
        [avEngine_ stop];
        for(uint32_t playerId = 0; playerId < player_count_; playerId ++) {
            players_[playerId].audio_player = nil;
            players_[playerId].pitch_controller = nil;
        }
        
        if(Sound_buffers_) {
            for(int i = 0; i < Sound_buffers_->count; i++) {
                Sound_buffers_->buffers[i] = nil;
            }
            coq_free(Sound_buffers_);
            Sound_buffers_ = NULL;
        }
        avEngine_ = nil;
    });
}

void  Sound_initWithWavNames(const char* wav_names[], uint32_t wav_count) {
    Sound_wavNames_ = wav_names;
    Sound_wavCount_ = wav_count;
    Sound_resume();
}

void  Sound_play(uint32_t const soundId, float volume, int pitch, uint32_t volumeId) {
    if(volumeId >= Sound_volume_count) {
        printerror("Volume id overflow %d.", volumeId);
        return;
    }
    if(soundId >= Sound_wavCount_) {
        printerror("SoundId overflow %d, wav_count %d.", soundId, Sound_wavCount_);
        return;
    }
    if(Sound_isMute || _volumes[volumeId] < 0.01)
        return;
    if(coqSound_dispatch_queue_ == NULL) {
        printerror("No Sound queue."); return;
    }
    dispatch_async(coqSound_dispatch_queue_, ^{
        if(avEngine_ == nil) return;
        if(![avEngine_ isRunning]) {
            NSError* error;
            [avEngine_ startAndReturnError:&error];
            if(error != nil) {
                printerror("Cannot start sound engine.");
                return;
            }
        }
        if(Sound_buffers_->buffers[soundId] == nil) {
            printerror("Sound buffer not inif for sound %d.", soundId);
            return;
        }
        // 1. Trouver un player disponible...
//        uint32_t bestPlayerId = 0;
//        int64_t  bestRemainingMS = 20000;
        struct Player_* player = NULL;
//        bool     foundFree = false;
        for(uint32_t playerId = 0; playerId < player_count_; playerId ++) {
            int64_t remaining = players_[playerId].expirationTimeMS - ChronoApp_elapsedMS();
            // Ok, trouve...
            if(remaining <= 0) {
                player = &players_[playerId];
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
//            printwarning("No available player.");
            return;
        }
        if(player->audio_player == nil || player->pitch_controller == nil) {
            printerror("Player not init.");
            return;
        }
        // 3. Mise à jour du expiredTime (en ms)
        AVAudioPCMBuffer* buffer = Sound_buffers_->buffers[soundId];
        double durationSec = (double)buffer.frameLength / buffer.format.sampleRate;
        player->expirationTimeMS = ChronoApp_elapsedMS() + (int64_t)(durationSec * 1000.0 + 300.0);
        // 4. Préparation du player et pitchControl
        if(player->audio_player.isPlaying) {
            [player->audio_player stop];
        }
        player->pitch_controller.pitch = (float)(pitch * 100);
        player->audio_player.volume = volume * _volumes[volumeId];
        [player->audio_player scheduleBuffer:Sound_buffers_->buffers[soundId] completionHandler:nil];
        // 5. Jouer le son !
        [player->audio_player play];
    });
}
