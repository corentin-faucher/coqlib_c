//
//  root.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#ifndef coq_node_root_h
#define coq_node_root_h

#include "node_button.h"
#include "node_view.h"
#include "../utils/util_event.h"

// Racine, noeud de base de l'app.
// Une root gère le framing de ses descendants.
// w/h (de Node) sont le "cadre utilisable", e.g. 3.5x2, (de (-1.75,-1) à
// (1.75,1)). frameWidth/frameHeihgt est le cadre complet (avec marges),
// e.g. 3.7 x 2.2. viewWidthPx/viewHeihgtPx est la version en pixel (de la view
// de l'OS) des frameWidth/frameHeihgt, e.g. 1080 x 640.
typedef struct coq_Root {
  Node n; // Peut être up-casté comme un noeud.

  /// Zone active (dans la fenetre) de la root (marges incluses), e.g. 3.7 x 2.2
  /// quand w x h = 3.5 x 2.
  FluidPos fullSizeWidth;
  FluidPos fullSizeHeight;
  /// Decalage en y pour la difference de marges haut/bas.
  FluidPos yShift;
  //    float       _yShift;
  //    Vector2     fullSize;
  /// Version points (typiquement 2x les pixels) de fullSize.
  Vector2 viewSizePt;
  /// Marges en points.
  Margins margins;
  /// Matrice de projection (optionnel, par défaut pour root : M = P * V.)
  Matrix4 projectionOpt;
  /// Positionnement de la Camera : eye, centre, up. Typiquement (0,0,4), (0,0,0),
  /// Voir `root_renderer_defaultUpdateIU_`.
  /// (0,1,0). On peut s'amuser à la bouger...
  FluidPos camera_center[3]; // Où on regarde (origine typiquement)
  FluidPos camera_eye[3];    // Où est situé la caméra (typiquement un peu au dessus du plan xy, e.g. (0, 0, 5))
  FluidPos camera_up[3];     // "Haut" de la caméra (typiquement vers les y positifs, e.g. (0, 1, 0))
  /// Couleur du fond (clearColor)
  FluidPos back_RGBA[4];
  /// Lumière ambiante (entre 0 et 1)
  FluidPos ambiantLight;
  // Les vues importantes : la vue présentement active, le "backscreen" et le "frontscreen".
  View *viewActiveOpt;
  /// En avant plan, e.g. pour les effets de particules... -> voir Sparkles.
  View *viewFrontOpt;
  /// Le fond d'écran.
  View *viewBackOpt;
  Node *toDeleteViewNodeOpt;
  Root *rootParentOpt;

  // Actions optionnelles à definir pour differents events...
  /// Action à faire à chaque changement de vue, e.g. faire un bruit.
  void (*changeViewOpt)(Root *);
  void (*resizeOpt)(Root *, ViewSizeInfo);
  void (*resumeAfterMSOpt)(Root *, int64_t);
  Timer _timer;
} Root;

/// A priori une root est à `parent = NULL`, pas de parents.
/// Mais on peut faire une sous-root (i.e. avec nouvelle matrice de projection),
/// dans ce cas il faut fournir la root absolue `parentRoot` et le parent direct `parent` 
/// (parentRoot et parent peuvent être le même noeud).
void root_and_super_init(Root *root, Root *parentRootOpt, Node* parentOpt);
/// Downcasting.
static inline Root* node_asRootOpt(Node const*const nOpt) {
    return (nOpt && (nOpt->_type & node_type_root)) ? (Root*)nOpt : NULL;
}

/// Changement de view (fermeture/release de l'ancienne view et ouverture de la nouvelle)
void root_changeViewActiveTo(Root *rt, View *newViewOpt);
/// Resize de la window.view.
void root_viewResized(Root *rt, ViewSizeInfo info);
/// Resize temporaire de la window.view (durant une rotation de l'écran)
void root_justSetFrameSize_(Root *r, Vector2 frameSizePt);

/// Obtenir le rectangle (en pixels) associé à une position (origin)
///  et dimensions dans le frame de la root.
/// e.g. (0.5, 0.5), (1, 1) -> (540, 320), (290, 290).
Rectangle root_windowRectangleFromBox(Root const* rt, Box box, bool invertedY);
/// Semblable à `node_hitBoxInParentReferential`, mais
/// retourne l'espace occupé en pts dans la view de l'OS.
/// invertedY == true pour iOS (y vont vers les bas).
Rectangle node_windowRectangle(Node const* n, bool invertedY);
/// Obtenir la position dans le frame de la root à partir de la position de la
/// vue (en pixels). e.g. (540, 320) -> (0.0, 0.0), (centre d'une vue 1080 x
/// 640).
Vector2 root_absposFromViewPos(Root *rt, Vector2 viewPos, bool invertedY);

/// Init/update la matrice de projection avec le frame de la root (perspective
/// matrix).
//void matrix4_initProjectionWithRoot(Matrix4 *m, Root *rt);

#endif /* node_root_h */
