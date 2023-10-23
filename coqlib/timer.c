//
//  timer.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-23.
//
#include <stdlib.h>
#include "timer.h"
#include "node.h"
#include "chronometers.h"
#include "utils.h"

typedef struct _Timer {
//    void    (*callBack)(void*);
    void    (*callBack)(Node*);
    Node*   node;
    int64_t deltaTimeMS;
    int64_t ringTimeMS;
} Timer;

static Timer* _Timer_list = NULL;
static uint   _Timer_firstFreeIndex = 0;
static uint   _Timer_listBegin = 0;  // Premier timer actif.
static uint   _Timer_listEnd = 0;    // taille actuelle, juste apres le dernier element.
static uint   _Timer_listCount = 0;  // taille disponible (apres reallloc)

uint  Timer_createAndGetId(int64_t deltaTimeMS, Bool isRepeating,
               Node* nd, void (*callBack)(Node*)) {
    if(_Timer_firstFreeIndex >= _Timer_listCount) {
        _Timer_listCount += 10;
        _Timer_list = realloc(_Timer_list, sizeof(Timer) * _Timer_listCount);
    }
    uint id = _Timer_firstFreeIndex;
    Timer* newTimer = &_Timer_list[_Timer_firstFreeIndex];
    newTimer->callBack = callBack;
    newTimer->node = nd;
    newTimer->deltaTimeMS = isRepeating ? deltaTimeMS : 0;
    newTimer->ringTimeMS = ChronoApp_elapsedMS() + deltaTimeMS;
    if(_Timer_firstFreeIndex < _Timer_listBegin) {
        _Timer_listBegin = _Timer_firstFreeIndex;
    }
    
    _Timer_firstFreeIndex ++;
    
    if(_Timer_listEnd < _Timer_firstFreeIndex)
        _Timer_listEnd = _Timer_firstFreeIndex;
    // S'il y en a d'autres, cherche le premier emplacement libre...
    while(_Timer_firstFreeIndex < _Timer_listEnd) {
        if(_Timer_list[_Timer_firstFreeIndex].callBack)
            _Timer_firstFreeIndex ++;
        else
            break;
    }
    return id;
}
void _timer_remove(Timer* timer) {
    timer->callBack = NULL;
    timer->node = NULL;
    const uint removedIndex = (uint)(timer - _Timer_list);
    
    if(removedIndex < _Timer_firstFreeIndex) {
        _Timer_firstFreeIndex = removedIndex;
    }
    // Si c'etait le dernier, replacer la fin sur le premier actif.
    if(removedIndex == _Timer_listEnd - 1) {
        _Timer_listEnd --;
        while(_Timer_listEnd > 0) {
            if(_Timer_list[_Timer_listEnd - 1].callBack == NULL)
                _Timer_listEnd --;
            else
                break;
        }
    }
    // (cas tout efface... -> remise a zero)
    if(_Timer_listBegin > _Timer_listEnd)
        _Timer_listBegin = _Timer_listEnd;
    // Si c'etait le premier, replacer le debut sur le premier actif.
    if(removedIndex == _Timer_listBegin) {
        while(_Timer_listBegin < _Timer_listEnd) {
            if(_Timer_list[_Timer_listBegin].callBack == NULL)
                _Timer_listBegin ++;
            else
                break;
        }
    }
}
void timer_cancel(uint timerId) {
    if(timerId >= _Timer_listEnd) {
        printerror("Bad timer Id %d", timerId);
        return;
    }
    _timer_remove(&_Timer_list[timerId]);
}
void Timer_check(void) {
    if(_Timer_listEnd == 0) return;
    Timer* t = &_Timer_list[_Timer_listBegin];
    Timer* end = &_Timer_list[_Timer_listEnd];
    int64_t currentTime = ChronoApp_elapsedMS();
    for(; t < end; t++) {
        if(t->node->flags & _flag_deleted) {
            _timer_remove(t);
            continue;
        }
        if(t->callBack == NULL)
            continue;
        if(t->ringTimeMS < currentTime)
            continue;
        t->callBack(t->node);
        if(t->deltaTimeMS) // Repete
            t->ringTimeMS += t->deltaTimeMS;
        else // Just one shot.
            _timer_remove(t);
    }
}
