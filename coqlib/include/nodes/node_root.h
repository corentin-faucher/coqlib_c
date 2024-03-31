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
typedef struct coq_Root {
    Node        n;            // Peut être up-casté comme un noeud.
    
    /// Zone active (dans la fenetre) de la root (marges incluses), e.g. 3.7 x 2.2 quand w x h = 3.5 x 2.
    FluidPos    fullSizeWidth;
    FluidPos    fullSizeHeight;
    /// Decalage en y pour la difference de marges haut/bas.
    FluidPos    yShift;
//    float       _yShift;
//    Vector2     fullSize;
    /// Version points (typiquement 2x les pixels) de fullSize.
    Vector2     viewSizePt;
    /// Marges en points.
    Margins     margins;
    /// Camera. que les vecteurs eye, centre, up. Typiquement (0,0,4), (0,0,0), (0,1,0).
    /// On peut s'amuser à la bouger...
    Camera      camera;
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
    
    /// Events... ? Juste shouldTerminate pour l'instant.
    bool        shouldTerminate;    
    
    /// Fonction utilisee pour mise a jour de la matrice model d'un noeud avant l'affichage.
    /// Par défaut `node_defaultUpdateModelAndGetAsDrawableOpt`.
    Drawable*   (*updateModelAndGetDrawable)(Node*);
    // Actions optionnelles à definir pour differents events...
    /// Action à faire à chaque changement de vue, e.g. faire un bruit.
    void        (*changeViewOpt)(Root*);
    void        (*resizeOpt)(Root*, ResizeInfo);
    void        (*resumeAfterMSOpt)(Root*, int64_t);
//    void        (*willTerminateOpt)(Root*);  // Utile ?
} Root;


/// A priori une root est à `parent = NULL`, pas de parents.
/// Mais on peut faire une sous-root (i.e. avec nouvelle matrice de projection),
/// dans ce cas il faut fournir la root absolue `parentRoot`.
void  root_init(Root* root, Root* parentRootOpt);
/// Downcasting.
Root* node_asRootOpt(Node* n);

void    root_changeViewActiveTo(Root* rt, View* newViewOpt);
/// Resize de la window.view.
void    root_viewResized(Root *rt, ResizeInfo info);
void    root_justSetFrameSize_(Root* r, Vector2 frameSizePt);
void    root_updateModelMatrix(Root *rt);

Button* root_searchActiveButtonOptWithPos(Root* const root, Vector2 const absPos,
                                 Node* const nodeToAvoidOpt);
Button* root_searchFirstButtonOptWithData(Root* root, uint32_t typeOpt, uint32_t data0);
SlidingMenu* root_searchFirstSlidingMenuOpt(Root* root);

bool    root_isLandscape(Root* r);

/// Obtenir le rectangle (en pixels) associé à une position (origin)
///  et dimensions dans le frame de la root.
/// e.g. (0.5, 0.5), (1, 1) -> (540, 320), (290, 290).
Rectangle root_windowRectangleFromBox(Root *rt, Box box, bool invertedY);
/// Semblable à `node_hitBoxInParentReferential`, mais
/// retourne l'espace occupé en pts dans la view de l'OS.
/// invertedY == true pour iOS (y vont vers les bas).
Rectangle node_windowRectangle(Node* n, bool invertedY);
/// Obtenir la position dans le frame de la root à partir de la position de la vue (en pixels).
/// e.g. (540, 320) -> (0.0, 0.0), (centre d'une vue 1080 x 640).
Vector2   root_absposFromViewPos(Root *rt, Vector2 viewPos, bool invertedY);
/// Init/update la matrice de projection avec le frame de la root (perspective matrix).
void      matrix4_initProjectionWithRoot(Matrix4 *m, Root *rt);

#endif /* node_root_h */
