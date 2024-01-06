//
//  timer.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-23.
//
#include "coq_timer.h"
#include "_math/_math_chrono.h"

typedef struct _Timer {
    void    (*callBack)(void*);
    void*   target;
    Timer** referer;
    int64_t deltaTimeMS;  // Si 0, pas de repetition.
    int64_t ringTimeMS;
} Timer;

#define _Timer_count 64
static uint32_t     _Timer_active_count = 0;  // (pour debuging)
static Timer        _Timer_list[_Timer_count];
static Timer* const _Timer_listEnd =     &_Timer_list[_Timer_count];
static Timer*       _Timer_freeFirst =   _Timer_list;
static Timer*       _Timer_activeFirst = _Timer_list;  // Premier timer actif.
static Timer*       _Timer_activeEnd =   _Timer_list;  // Fin (apres le dernier actif), mieux activeLast ?


void timer_scheduled(Timer** timerRefOpt, int64_t deltaTimeMS, bool isRepeating,
               void* target_object, void (*callBack)(void*)) {
    if(target_object == NULL || callBack == NULL) {
        printerror("Missing target_object or callBack.");
        return;
    }
    // Realloc ? Non, les objets garde une ref vers les timers.
    if(_Timer_freeFirst >= _Timer_listEnd) {
        printerror("Too many timer active ! Max timer is %d.", _Timer_count);
        return;
    }
    if(deltaTimeMS < 1) {
        printwarning("deltaTimeMS < 1.");
        deltaTimeMS = 1;
    }
    // Setter le nouveau active timer.
    Timer* newTimer = _Timer_freeFirst;
    newTimer->callBack = callBack;
    newTimer->target = target_object;
    newTimer->deltaTimeMS = isRepeating ? deltaTimeMS : 0;
    newTimer->ringTimeMS = ChronoApp_elapsedMS() + deltaTimeMS;
    _Timer_active_count ++;
    if(timerRefOpt) {
        if(*timerRefOpt) {
            printwarning("New timer before cancelinng the previous one.");
            timer_cancel(timerRefOpt);
        }
        *timerRefOpt = newTimer;
    }
    newTimer->referer = timerRefOpt;
    // Replacer les pointeurs debut et fin actives.
    if(_Timer_activeFirst > newTimer) {
        _Timer_activeFirst = newTimer;
    }
    if(_Timer_activeEnd <= newTimer)
        _Timer_activeEnd = newTimer + 1;
    // Cherche le premier espace libre.
    _Timer_freeFirst ++;
    while(_Timer_freeFirst < _Timer_activeEnd) {
        if(_Timer_freeFirst->callBack != NULL)
            _Timer_freeFirst ++;
        else
            break;
    }
}
void _timer_deinit(Timer* removed) {
    if(removed->callBack == NULL || removed->target == NULL) {
        printwarning("Already denit.");
        return;
    }
    // Dereferencer le timer.
    if(removed->referer)
        *(removed->referer) = (Timer*)NULL;
    // Clear.
    memset(removed, 0, sizeof(Timer));
    _Timer_active_count --;
    if(removed < _Timer_freeFirst) {
        _Timer_freeFirst = removed;
    }
    // Si c'etait le dernier, replacer la fin sur le premier actif.
    if(removed == _Timer_activeEnd - 1) {
        _Timer_activeEnd --;
        while(_Timer_activeEnd > _Timer_list) {
            if((_Timer_activeEnd - 1)->callBack == NULL)
                _Timer_activeEnd --;
            else
                break;
        }
    }
    // (cas tout efface... -> remise a zero)
    if(_Timer_activeFirst > _Timer_activeEnd)
        _Timer_activeFirst = _Timer_activeEnd;
    // Si c'etait le premier, replacer le debut sur le premier actif.
    if(removed == _Timer_activeFirst) {
        while(_Timer_activeFirst < _Timer_activeEnd) {
            if(_Timer_activeFirst->callBack == NULL)
                _Timer_activeFirst ++;
            else
                break;
        }
    }
}
void timer_cancel(Timer** timerRef) {
    if(*timerRef == NULL) return;
    if(timerRef != (*timerRef)->referer) {
        printerror("timerRef is not the timer referer.");
        return;
    }
    _timer_deinit(*timerRef);
}
void Timer_check(void) {
    if(_Timer_activeEnd == 0) return;
    Timer* t =         _Timer_activeFirst;
    Timer* const end = _Timer_activeEnd;
    int64_t currentTime = ChronoApp_elapsedMS();
    for(; t < end; t++) {
        if(t->callBack == NULL)
            continue;
        if(t->ringTimeMS > currentTime) {
            // Pas encore le temps...
            continue;
        }
        // Ok, on va executer le callBack.
        void  (*callBack)(void*) = t->callBack;
        void* target = t->target;
        // Si "one shot", on deinit tout de suite.
        if(t->deltaTimeMS == 0) {
            _timer_deinit(t);
        }
        // ** Ici, le call back peut cancel le timer, ou meme recreer un nouveau timer au meme endroit. **
        callBack(target);
        // S'il y a toujour un timer a repeter, mettre a jour son ringTime
        if(t->callBack && (t->ringTimeMS <= currentTime) && t->deltaTimeMS)
            t->ringTimeMS += t->deltaTimeMS;
    }
}
