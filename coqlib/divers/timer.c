//
//  timer.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-23.
//
#include "timer.h"
#include "_node.h"
#include "chronometers.h"
#include "utils.h"

typedef struct _Timer {
    void    (*callBack)(Node*);
    Node*   node;
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


void timer_scheduled(Timer** timerRefOpt, int64_t deltaTimeMS, Bool isRepeating,
               Node* node_target, void (*callBack)(Node*)) {
    if(node_target == NULL || callBack == NULL) {
        printerror("Missing node_target or callBack.");
        return;
    }
    // Realloc ? Non, les objets garde une ref vers les timers.
    if(_Timer_freeFirst >= _Timer_listEnd) {
        printerror("Too many timer active ! Max timer is %d.", _Timer_count);
        return;
    }
//    if(_Timer_freeFirst >= _Timer_listEnd) {
//        if(_Timer_list) {
//            printwarning("Need to realloc _Timer_list.");
//        }
//        // Realloc
//        _Timer_listCount += 16;
//        Timer* oldHead = _Timer_list;
//        _Timer_list = coq_realloc(_Timer_list, sizeof(Timer) * _Timer_listCount);
//        // Set pointers... Stuff dangereux... ?
//        ptrdiff_t delta = (char*)_Timer_list - (char*)oldHead;
//        _Timer_listEnd = &_Timer_list[_Timer_listCount];
//        _Timer_freeFirst =   (Timer*)((char*)_Timer_freeFirst + delta);
//        _Timer_activeFirst = (Timer*)((char*)_Timer_activeFirst + delta);
//        _Timer_activeEnd =   (Timer*)((char*)_Timer_activeEnd + delta);
//
//        printdebug("🐸 scheduled list %p,  activeFirst %p, activeFirst with delta %p, delta %p.",
//                   _Timer_list,
//                   _Timer_activeFirst,
//                   _Timer_activeEnd,
//                   (void*)delta
//                   );
//    }
    if(deltaTimeMS < 1) {
        printwarning("deltaTimeMS < 1.");
        deltaTimeMS = 1;
    }
    // Setter le nouveau active timer.
    Timer* newTimer = _Timer_freeFirst;
    newTimer->callBack = callBack;
    newTimer->node = node_target;
    newTimer->deltaTimeMS = isRepeating ? deltaTimeMS : 0;
    newTimer->ringTimeMS = ChronoApp_elapsedMS() + deltaTimeMS;
    newTimer->referer = timerRefOpt;
    _Timer_active_count ++;
    if(timerRefOpt)
        *timerRefOpt = newTimer;
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
    
    /* Version Index...
    if(_Timer_freeFirst >= _Timer_listCount) {
        _Timer_listCount += 10;
        _Timer_list = realloc(_Timer_list, sizeof(Timer) * _Timer_listCount);
    }
    if(deltaTimeMS == 0 && isRepeating) {
        printwarning("deltaTimeMS == 0 -> reserved for no repeat. To repeat every frame put deltaTimeMS == 1.");
        deltaTimeMS = 1;
    }
    Timer* newTimer = &_Timer_list[_Timer_freeFirst];
    newTimer->callBack = callBack;
    newTimer->node = node_target;
    newTimer->deltaTimeMS = isRepeating ? deltaTimeMS : 0;
    newTimer->ringTimeMS = ChronoApp_elapsedMS() + deltaTimeMS;
    if(_Timer_freeFirst < _Timer_activeFirst) {
        _Timer_activeFirst = _Timer_freeFirst;
    }
    
    _Timer_freeFirst ++;
    
    if(_Timer_activeEnd < _Timer_freeFirst)
        _Timer_activeEnd = _Timer_freeFirst;
    // S'il y en a d'autres, cherche le premier emplacement libre...
    while(_Timer_freeFirst < _Timer_activeEnd) {
        if(_Timer_list[_Timer_freeFirst].callBack)
            _Timer_freeFirst ++;
        else
            break;
    }
    return newTimer; */
}
void _timer_deinit(Timer* removed) {
    if(removed->callBack == NULL || removed->node == NULL) {
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
        if(t->node->flags & _flag_toDelete) {
            _timer_deinit(t);
            continue;
        }
        // Sonne ?
        if(t->ringTimeMS > currentTime) {
            continue;
        }
        // ** Ici, le call back peut cancel le timer, ou meme recreer un nouveau timer au meme endroit. **
        t->callBack(t->node);
        // Reverifier si changement : plus de timer ou nouveau a caller plus tard -> aller au prochain.
        if(t->callBack == NULL || t->ringTimeMS > currentTime)
            continue;
        // Repete ?
        if(t->deltaTimeMS) {
            t->ringTimeMS += t->deltaTimeMS;
            continue;
        }
        // Just one shot.
        _timer_deinit(t);
    }
}
