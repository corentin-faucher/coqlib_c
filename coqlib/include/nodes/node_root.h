//
//  root.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#ifndef _coq_node_root_h
#define _coq_node_root_h

#include "maths/math_camera.h"
#include "nodes/node_button.h"
#include "nodes/node_view.h"
#include "coq_event.h"

// Racine, noeud de base de l'app.
// Une root gère le framing de ses descendants.
// w/h (de Node) sont le "cadre utilisable", e.g. 3.5x2, (de (-1.75,-1) à (1.75,1)).
// frameWidth/frameHeihgt est le cadre complet (avec marges), e.g. 3.7 x 2.2.
// viewWidthPx/viewHeihgtPx est la version en pixel (de la view de l'OS) des frameWidth/frameHeihgt,
// e.g. 1080 x 640.
typedef struct _Root {
    // Upcasting.
    Node        n;            // Peut être casté comme un noeud.
    /// Zone active (dans la fenetre) de la root (marges incluses), e.g. 3.7 x 2.2 quand w x h = 3.5 x 2.
    Vector2     fullSize;
    /// Version points (typiquement 2x les pixels) de fullSize.
    Vector2     viewSizePt;
    /// Marges en points.
    Margins     margins;
    /// Camera. que les vecteurs eye, centre, up. Typiquement (0,0,4), (0,0,0), (0,1,0).
    /// On peut s'amuser à la bouger...
    Camera      camera;
    /// Decalage en y pour la difference de marges haut/bas.
    float       _yShift;
    /// Couleur du fond (clearColor)
    FluidPos    back_RGBA[4];
    /// La derniére position clické/touché.
    Vector2     lastTouchedPos;
    // References
    /// Le bouton présentement grabbé.
    Button*     buttonSelectedOpt;
    /// Les vues importantes : la vue présentement active, le "backscreen" et le "frontscreen".
    View*       viewActiveOpt;
    /// En avant plan, e.g. pour les effets de particules... -> voir Sparkles.
    View*       viewFrontOpt;
    /// Le fond d'écran.
    View*       viewBackOpt;
    Root*       rootParentOpt;
    /// Préférences
    /// à définir si besoin, il n'y qu'une prédéclaration.
    Prefs**     prefsRef;
    
    /// Events... ? Juste shouldTerminate pour l'instant.
    bool        shouldTerminate;    
    
    /// Fonction utilisee pour mise a jour de la matrice model d'un noeud avant l'affichage.
    /// Par défaut `node_defaultUpdateModelAndGetAsDrawableOpt`.
    Drawable*   (*updateModelAndGetDrawable)(Node*);
    // Actions optionnelles à definir pour differents events...
    void        (*changeScreenActionOpt)(Root*);
    void        (*resizeActionOpt)(Root*, ResizeInfo);
    void        (*didResumeActionOpt)(Root*);
    void        (*systemDidChangedActionOpt)(Root*, SystemChange);

//    void        (*willTerminateOpt)(Root*);  // Utile ?
} Root;

/// Juste pour init de la structure de base.
/// Il faut creer son propre init de root.
/// (On peut laisser size = 0, sera init avec sizeof(NodeRoot).)
Root* Root_create(void);
/// Init pour sub-struct
void  root_init(Root* root, Node* parentOpt, Root* parentRootOpt);
/// Downcasting.
Root* node_asRootOpt(Node* n);

void   root_changeViewActiveTo(Root* rt, View* newViewOpt);
/// Resize de la window.view.
void   root_viewResized(Root *rt, ResizeInfo info);
void   root_updateModelMatrix(Root *rt);
/*-- Init d'autres struct utilisant la root. --*/
/// Obtenir le rectangle (en pixels) associé à une position (origin)
///  et dimensions dans le frame de la root.
/// e.g. (0.5, 0.5), (1, 1) -> (540, 320), (290, 290).
Rectangle root_viewRectangleFromPosAndDim(Root *rt, Vector2 pos, Vector2 deltas,
                                          bool invertedY);
/// Obtenir la position dans le frame de la root à partir de la position de la vue (en pixels).
/// e.g. (540, 320) -> (0.0, 0.0), (centre d'une vue 1080 x 640).
Vector2   root_absposFromViewPos(Root *rt, Vector2 viewPos, bool invertedY);
/// Init/update la matrice de projection avec le frame de la root (perspective matrix).
void      matrix4_initProjectionWithRoot(Matrix4 *m, Root *rt);

#endif /* node_root_h */
