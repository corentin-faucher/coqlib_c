//
//  timer.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-23.
//
#include "util_timer.h"

#include "util_chrono.h"
#include "util_base.h"

typedef struct coq_TimerStruct ** TimerRef;
struct coq_TimerStruct {
    void   (*callBack)(void*);
    void    *targetOpt;
    TimerRef referer;
    int64_t  deltaTimeMS;  // Si 0, pas de repetition.
    int64_t  ringTimeMS;
};


#define Timer_count_ 64
static uint32_t     _Timer_active_count = 0;  // (pour debuging)
static struct coq_TimerStruct        _Timer_list[Timer_count_];
static struct coq_TimerStruct* const _Timer_listEnd =     &_Timer_list[Timer_count_];
static struct coq_TimerStruct*       _Timer_freeFirst =    _Timer_list;
static struct coq_TimerStruct*       _Timer_activeFirst =  _Timer_list;  // Premier timer actif.
static struct coq_TimerStruct*       _Timer_activeEnd =    _Timer_list;  // Fin (apres le dernier actif), mieux activeLast ?


void timer_scheduled(Timer *const timerRef, int64_t deltaTimeMS, bool const isRepeating,
                     void *const targetObjectOpt, void (*const callBack)(void* targetObjectOpt)) {
    // Checks...
    if(callBack == NULL) {
        printerror("Missing target_object or callBack.");
        return;
    }
    // Realloc ? Non, les objets garde une ref vers les timers.
    if(_Timer_freeFirst >= _Timer_listEnd) {
        printerror("Too many timer active ! Max timer is %d.", Timer_count_);
        return;
    }
    if(deltaTimeMS < 1) { // Repéter à chaque tick, ou activer tout de suite.
        deltaTimeMS = 1;
    }
    if(timerRef) if(*timerRef) {
        printwarning("Timer should be canceled before scheduling a new job.");
        timer_cancel(timerRef);
    }
    // Setter le nouveau active timer.
    Timer newTimer = _Timer_freeFirst;
    newTimer->callBack = callBack;
    newTimer->targetOpt = targetObjectOpt;
    newTimer->deltaTimeMS = isRepeating ? deltaTimeMS : 0;
    newTimer->ringTimeMS = ChronoApp_elapsedMS() + deltaTimeMS;
    newTimer->referer = timerRef;
    if(timerRef) *timerRef = newTimer;
    else printwarning("Call to timer_schedule without keeping ref.");
    _Timer_active_count ++;
    
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

void timer_deinit_(Timer const removed) {
    if(removed->callBack == NULL) {
        printwarning("Already denit.");
        return;
    }
    // Dereferencer le timer.
    if(removed->referer)
        *removed->referer = (Timer)NULL;
    // Clear.
    memset(removed, 0, sizeof(struct coq_TimerStruct));
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
void timer_cancel(Timer *const timer) {
    if(*timer == NULL) return;
    if(timer != (*timer)->referer) {
        printerror("timerRef is not the timer referer.");
        return;
    }
    timer_deinit_(*timer);
}

void timer_doNowAndCancel(Timer *const timer) {
    if(*timer == NULL) return;
    Timer t = *timer;
    if(timer != t->referer) {
        printerror("timerRef is not the timer referer.");
        return;
    }
    if(t->callBack) t->callBack(t->targetOpt);
    else { printerror("Timer without callback."); }
    timer_deinit_(t);
}

void Timer_check(void) {
    if(_Timer_activeEnd == 0) return;
    struct coq_TimerStruct* t =         _Timer_activeFirst;
    struct coq_TimerStruct* const end = _Timer_activeEnd;
    int64_t currentTime = ChronoApp_elapsedMS();
    for(; t < end; t++) {
        if(t->callBack == NULL)
            continue;
//      Il faut s'assurer de cancel les timer avant de delete un noeud...
//        if(t->target->flags & flag_toDelete_) {
//            timer_deinit_(t);
//            continue;
//        }
        if(t->ringTimeMS > currentTime) {
            // Pas encore le temps...
            continue;
        }
        // Ok, on va executer le callBack.
        void (*callBack)(void*) = t->callBack;
        void*  targetOpt = t->targetOpt;
        // Si "one shot", on deinit tout de suite.
        if(t->deltaTimeMS == 0) {
            timer_deinit_(t);
        }
        // **-------------------------------------------------------------------------------------------** //
        // ** Ici, le call back peut cancel le timer, ou meme recreer un nouveau timer au meme endroit. **
        callBack(targetOpt);
        
        // S'il y a toujour un timer à repeter, mettre à jour son ringTime
        if(t->callBack && (t->ringTimeMS <= currentTime) && t->deltaTimeMS)
            t->ringTimeMS += t->deltaTimeMS;
    }
}
