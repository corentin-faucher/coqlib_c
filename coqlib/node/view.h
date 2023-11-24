//
//  node_screen.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#ifndef node_screen_h
#define node_screen_h

#include "fluid.h"
#include "utils.h"

typedef struct _Root Root;
typedef struct _View View;

typedef struct {
    uint16_t  keycode;
    uint32_t  keymod;
    Bool      isVirtual;
    char      theChar;
} KeyboardInput;

typedef struct _View {
    union {  // Upcasting
        Node       n;
        Fluid      f;
    };
    Root*      root;
    /*-- Escape --*/
    void (*escape)(View*);
    /*-- Enter --*/
    void (*enter)(View*);
    /*-- Key Responder --*/
    void (*keyDown)(View*, KeyboardInput);
    void (*keyUp)(View*, KeyboardInput);
    void (*modifiersChangedTo)(View*, uint32_t);
    /*-- Char Responder --*/
    void (*charAction)(View*, char);
} View;

// Constructeur et init par defaut.
View* View_create(Root* const root, flag_t flags);
// Init pour sub-structs.
void  view_connectToRootAndInit(View* v, Root* root);
// Downcasting
View*  node_asViewOpt(Node* n);

void   view_alignElements(View* v, Bool isOpening);

// Open et reshape de Node (pour sous-structs).
void   view_open(Node* const node);
void   view_reshape(Node* const node);

#endif /* node_screen_h */
