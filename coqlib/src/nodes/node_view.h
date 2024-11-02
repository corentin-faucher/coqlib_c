//
//  node_screen.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#ifndef coq_node_view_h
#define coq_node_view_h

#include "../coq_event.h"
#include "node_button.h"

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
  void (*touchHovering)(View*, Vector2); 
  void (*touchDown)(View*, Vector2, uint32_t);
  void (*touchDrag)(View*, Vector2, uint32_t);
  void (*touchUp)(View*, uint32_t);
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
View* node_asViewOpt(Node *n);

// Méthodes `privées` pour override...
void view_open_(Node* n);
void view_reshape_(Node* n);
/// AlignElement : par défault, caller lors de `open` et `reshape`.
void view_alignElements_(View *v, bool isOpening);
// Action par defaut de survol: `selectionner` un bouton.
void view_touchHoveringDefault_(View* v, Vector2 pos);
// Action de `touch` par defaut: activer un bouton ou dragger un draggable.
void view_touchDownDefault_(View* v, Vector2 pos, uint32_t touchId);
void view_touchDragDefault_(View* v, Vector2 pos, uint32_t touchId);
void view_touchUpDefault_(View* v, uint32_t touchId);

#endif /* node_screen_h */
