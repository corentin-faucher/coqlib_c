//
//  node_screen.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#ifndef coq_node_view_h
#define coq_node_view_h

#include "node_button.h"
#include "../utils/util_event.h"

typedef struct coq_View View;
typedef struct coq_View {
  union { // Upcasting
    Node  n;
    Fluid f;
  };
  /// La derniére position clické/touché.
  Vector2 lastTouchedPos;
  /// Le bouton présentement grabbé.
  Button *buttonSelectedOpt;
  
  // Méthodes pour répondre à des évènements.
  /*-- Enter responder --*/
  void (*enterOpt)(View *);
  /*-- Escape --*/
  void (*escapeOpt)(View *);
  /*-- Key Responder --*/
  void (*keyDownOpt)(View *, KeyboardInput);
  void (*keyUpOpt)(View *, KeyboardInput);
  void (*modifiersChangedToOpt)(View *, uint32_t);
    /*-- Mouse/touch --*/
    void (*touchHovering)(NodeTouch nt); 
    void (*touchDown)(NodeTouch nt);
    void (*touchDrag)(NodeTouch nt);
    void (*touchUp)(NodeTouch nt);
    /*-- GamePad Responder --*/
    void (*gamePadDownOpt)(View *, GamePadInput);
    void (*gamePadUpOpt)(View *, GamePadInput);
    void (*gamePadValueOpt)(View *, GamePadInput);
    /*-- Char Responder --*/
    void (*charActionOpt)(View *, char);
    /*-- Changement system --*/
    void (*systemChangedOpt)(View *, SystemChange);
} View;

/// Init (init aussi les supers : Node et Fluid).
void  view_and_super_init(View* v, Root* root, flag_t flags);

// Downcasting
static inline View* node_asViewOpt(Node* nOpt) {
    return (nOpt && (nOpt->_type & node_type_view)) ? (View*)nOpt : NULL;
}

// Méthodes `privées` pour override...
void view_open_(Node* n);
void view_reshape_(Node* n);
/// AlignElement : par défault, caller lors de `open` et `reshape`.
void view_alignElements_(View *v, bool isOpening);
// Action par defaut de survol: `selectionner` un bouton.
void view_touchHoveringDefault_(NodeTouch nt);
// Action de `touch` par defaut: activer un bouton ou dragger un draggable.
void view_touchDownDefault_(NodeTouch nt);
void view_touchDragDefault_(NodeTouch nt);
void view_touchUpDefault_(NodeTouch nt);

#endif /* node_screen_h */
