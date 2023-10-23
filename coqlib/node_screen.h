//
//  node_screen.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#ifndef node_screen_h
#define node_screen_h

#include <stdio.h>
#include "node_smooth.h"

typedef struct _NodeRoot NodeRoot;
typedef struct _NodeScreen NodeScreen;

#warning A replacer...
typedef struct {
    uint16_t  keycode;
    uint32_t  keymod;
    Bool      isVirtual;
    char      theChar;
} KeyboardInput;

typedef struct _NodeScreen {
    union {
        Node       nd;
        NodeSmooth ns;    // Peut être casté comme un noeud et smooth.
    };
    /*-- Escape --*/
    void (*escape)(NodeScreen*);
    /*-- Enter --*/
    void (*enter)(NodeScreen*);
    /*-- Key Responder --*/
    void (*keyDown)(NodeScreen*, KeyboardInput);
    void (*keyUp)(NodeScreen*, KeyboardInput);
    void (*modifiersChangedTo)(NodeScreen*, uint32_t);
    /*-- Char Responder --*/
    void (*charAction)(NodeScreen*, char);
} NodeScreen;

NodeScreen* NodeScreen_create(NodeRoot* const root);
void  nodescreen_alignElements(NodeScreen* screen, Bool isOpening);

#endif /* node_screen_h */
