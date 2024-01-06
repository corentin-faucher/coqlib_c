//
//  node_screen.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#ifndef _coq_node_view_h
#define _coq_node_view_h

#include "_node_fluid.h"
#include "_string_char_and_keycode.h"

typedef struct _Root Root;
typedef struct _View View;

typedef struct _View {
    union {  // Upcasting
        Node       n;
        Fluid      f;
    };
    Root*      root;
    /*-- Escape --*/
    void (*escapeOpt)(View*);
    /*-- Enter --*/
    void (*enterOpt)(View*);
    /*-- Key Responder --*/
    void (*keyDownOpt)(View*, KeyboardInput);
    void (*keyUpOpt)(View*, KeyboardInput);
    void (*modifiersChangedToOpt)(View*, uint32_t);
    /*-- Char Responder --*/
    void (*charActionOpt)(View*, char);
} View;

// Constructeur et init par defaut.
View* View_create(Root* const root, flag_t flags);
// Init pour sub-structs.
void  view_connectToRootAndInit(View* v, Root* root);
// Downcasting
View*  node_asViewOpt(Node* n);

void   view_alignElements(View* v, bool isOpening);

// Open et reshape de Node (pour sous-structs).
void   view_open(Node* const node);
void   view_reshape(Node* const node);

#endif /* node_screen_h */
