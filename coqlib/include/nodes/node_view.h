//
//  node_screen.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#ifndef _coq_node_view_h
#define _coq_node_view_h

#include "node_fluid.h"
#include "coq_event.h"

typedef struct coq_View View;
typedef struct coq_View {
    union {  // Upcasting
        Node       n;
        Fluid      f;
    };
    /*-- Enter --*/
    void (*enterOpt)(View*);
    // <- Jusqu'ici la structure est +/- commune à Button...
    //    i.e. Une view peut être caster comme un button pour root, prefs, action.
    /*-- Escape --*/
    void (*escapeOpt)(View*);
    /*-- Key Responder --*/
    void (*keyDownOpt)(View*, KeyboardInput);
    void (*keyUpOpt)(View*, KeyboardInput);
    void (*modifiersChangedToOpt)(View*, uint32_t);
    /*-- Char Responder --*/
    void (*charActionOpt)(View*, char);
    /*-- Changement system --*/
    void (*systemChangedOpt)(View*, SystemChange);
} View;

/// Constructeur et init.
/// Size est la taille d'une sous-structure (si 0 -> sizeof(View)).
View* View_create(Root* const root, flag_t flags, size_t sizeOpt);
// Downcasting
View*  node_asViewOpt(Node* n);

// Open et reshape de Node (pour sous-structs).
void   view_open(Node* const node);    // (Ne font que caller `view_alignElements`)
void   view_reshape(Node* const node);
void   view_alignElements(View* v, bool isOpening);

#endif /* node_screen_h */
