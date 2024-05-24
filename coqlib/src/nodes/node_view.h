//
//  node_screen.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#ifndef _coq_node_view_h
#define _coq_node_view_h

#include "../coq_event.h"
#include "node_button.h"

typedef struct coq_View View;
typedef struct coq_View {
  union { // Upcasting
    Node n;
    Fluid f;
  };
  /// La derniére position clické/touché.
  Vector2 lastTouchedPos;
  /// Le bouton présentement grabbé.
  Button *buttonSelectedOpt;
  /*-- Enter responder --*/
  void (*enterOpt)(View *);
  // <- Jusqu'ici la structure est +/- commune à Button...
  //    i.e. Une view peut être caster comme un button pour root, prefs, action.
  /*-- Escape --*/
  void (*escapeOpt)(View *);
  /*-- Key Responder --*/
  void (*keyDownOpt)(View *, KeyboardInput);
  void (*keyUpOpt)(View *, KeyboardInput);
  void (*modifiersChangedToOpt)(View *, uint32_t);
  /*-- Mouse/touch --*/
  void (*touchHovering)(View*, Vector2); 
  void (*touchDown)(View*, Vector2);
  void (*touchDrag)(View*, Vector2);
  void (*touchUp)(View*);
  /*-- GamePad Responder --*/
  void (*gamePadDownOpt)(View *, GamePadInput);
  void (*gamePadUpOpt)(View *, GamePadInput);
  void (*gamePadValueOpt)(View *, GamePadInput);
  /*-- Char Responder --*/
  void (*charActionOpt)(View *, char);
  /*-- Changement system --*/
  void (*systemChangedOpt)(View *, SystemChange);
} View;

/// Constructeur et init.
/// Size est la taille d'une sous-structure (si 0 -> sizeof(View)).
View *View_create(Root *const root, flag_t flags, size_t sizeOpt);
// Downcasting
View *node_asViewOpt(Node *n);

// Méthodes `privées` pour override...
void view_alignElements_(View *v, bool isOpening);
void view_touchHoveringDefault_(View* v, Vector2 pos);
void view_touchDownDefault_(View* v, Vector2 pos);
void view_touchDragDefault_(View* v, Vector2 pos);
void view_touchUpDefault_(View* v);

#endif /* node_screen_h */
