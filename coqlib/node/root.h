//
//  root.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#ifndef node_root_h
#define node_root_h

#include "_node.h"
#include "camera.h"
#include "button.h"
#include "view.h"
#include "timer.h"

/// Marges en pixels.
typedef struct {
    double top;
    double left;
    double bottom;
    double right;
} Margins;

// Racine, noeud de base de l'app.
// Une root gère le framing de ses descendants.
// w/h (de Node) sont le "cadre utilisable", e.g. 3.5x2, (de (-1.75,-1) à (1.75,1)).
// frameWidth/frameHeihgt est le cadre complet (avec marges), e.g. 3.7 x 2.2.
// viewWidthPx/viewHeihgtPx est la version en pixel (de la view de l'OS) des frameWidth/frameHeihgt,
// e.g. 1080 x 640.
typedef struct _Root {
    // Upcasting.
    Node        n;            // Peut être casté comme un noeud.
    // Zone active (dans la fenetre) de la root (marges incluses).
    float       frameWidth;
    float       frameHeight;
    // Version pixel de frameWidth/Height.
    float       viewWidthPx;
    float       viewHeightPx;
    // Marges en pixels.
    Margins     margins;
    // Camera. que les vecteurs position/eye, centre, up, e.g. (0,0,4), (0,0,0), (0,1,0).
    // On peut s'amuser à la bouger...
    Camera      camera;
    /// Decalage en y pour la difference de marges haut/bas.
    float       _yShift;
    /// Couleur du fond (clearColor)
    SmoothPos   back_RGBA[4];
    /// Les vues importantes : la vue présentement active, le "backscreen" et le "frontscreen".
    View*       activeView;
    /// En avant plan, e.g. pour les effets de particules... -> voir Sparkles.
    View*       frontView;
    /// Le fond d'écran.
    View*       backView;
    /// Le bouton survolé
//    Button*     hoveredButton;
    /// Le bouton présentement grabbé.
//    Button*     grabbedButton;
    // volatile ?
    Button*     selectedButton;
    /// La derniére position clické/touché.
    Vector2     lastTouchedPos;
    Root*       parentRoot;
    
    /// Events... ? Juste shouldTerminate pour l'instant.
    Bool        shouldTerminate;    
    
    // Méthode à overrider... (optionnel)
    /// Mise a jour reguliere d'objet de la structure. Callé toute les 30~100 ms.
    void        (*iterationUpdate)(Root*);
    void        (*didResume)(Root*);
    void        (*changeScreenAction)(Root*);
    void        (*willTerminate)(Root*);  // Utile ?
} Root;

/// Juste pour init de la structure de base.
/// Il faut creer son propre init de root.
/// (On peut laisser size = 0, sera init avec sizeof(NodeRoot).)
Root* Root_create(void);
/// Init pour sub-struct
void  root_init(Root* root, Node* parentOpt, Root* parentRootOpt);
/// Downcasting.
Root* node_asRootOpt(Node* n);

void      root_changeActiveScreenTo(Root* rt, View* newView);
void      root_setFrame(Root *rt, Margins newMargins, Vector2 newSizesPx, Bool inTransition);
void      root_updateModelMatrix(Root *rt);
/*-- Init d'autres struct utilisant la root. --*/
/// Obtenir le rectangle (en pixels) associé à une position (origin)
///  et dimensions dans le frame de la root.
/// e.g. (0.5, 0.5), (1, 1) -> (540, 320), (290, 290).
Rectangle root_viewRectangleFromPosAndDim(Root *rt, Vector2 pos, Vector2 deltas,
                                          Bool invertedY);
/// Obtenir la position dans le frame de la root à partir de la position de la vue (en pixels).
/// e.g. (540, 320) -> (0.0, 0.0), (centre d'une vue 1080 x 640).
Vector2   root_absposFromViewPos(Root *rt, Vector2 viewPos, Bool invertedY);
/// Init/update la matrice de projection avec le frame de la root (perspective matrix).
void      matrix4_initProjectionWithRoot(Matrix4 *m, Root *rt);

#endif /* node_root_h */
