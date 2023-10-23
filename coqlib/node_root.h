//
//  node_root.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#ifndef node_root_h
#define node_root_h

#include <stdio.h>
#include "node.h"
#include "camera.h"
#include "node_selectable.h"
#include "node_screen.h"

/// Marges en pixels.
typedef struct {
    double top;
    double left;
    double bottom;
    double right;
} Margins;

typedef struct _NodeRoot NodeRoot;

// Racine, noeud de base de l'app.
// Une root gère le framing de ses descendants.
// w/h (de Node) sont le "cadre utilisable", e.g. 3.5x2, (de (-1.75,-1) à (1.75,1)).
// frameWidth/frameHeihgt est le cadre complet (avec marges), e.g. 3.7 x 2.2.
// viewWidthPx/viewHeihgtPx est la version en pixel (de la view de l'OS) des frameWidth/frameHeihgt,
// e.g. 1080 x 640.
typedef struct _NodeRoot {
    Node        nd;            // Peut être casté comme un noeud.
    float       frameWidth;
    float       frameHeight;
    float       viewWidthPx;
    float       viewHeightPx;
    Margins     margins;
    Camera      camera;
    float       yShift;
    float       zShift;
    
    NodeScreen* activeScreen;
    NodeSel*    selectedNode;
    NodeSel*    grabbedNode;
    
    void        (*willDrawFrame)(NodeRoot*);
    void        (*didResume)(NodeRoot*);
    void        (*changeScreenAction)(NodeRoot*);
} NodeRoot;

/// Juste pour init de la structure de base.
/// Il faut creer son propre init de root.
NodeRoot* NodeRoot_createEmpty(size_t size) ;
void      noderoot_changeToActiveScreen(NodeRoot* rt, NodeScreen* newScreen);

void      noderoot_setFrame(NodeRoot *rt, float widthPx, float heightPx, Bool inTransition);
void      noderoot_updateModelMatrix(NodeRoot *rt);
/*-- Init d'autres struct utilisant la root. --*/
/// Obtenir le rectangle (en pixels) associé à une position (origin)
///  et dimensions dans le frame de la root.
/// e.g. (0.5, 0.5), (1, 1) -> (540, 320), (290, 290).
Rectangle noderoot_viewRectangleFromPosAndDim(NodeRoot *rt, Vector2 pos, Vector2 deltas,
                                              Bool invertedY);
/// Obtenir la position dans le frame de la root à partir de la position de la vue (en pixels).
/// e.g. (540, 320) -> (0.0, 0.0), (centre d'une vue 1080 x 640).
Vector2   noderoot_posFromViewPos(NodeRoot *rt, Vector2 viewPos, Bool invertedY);

void      matrix4_initProjectionWithNodeRoot(Matrix4 *m, NodeRoot *rt);

#endif /* node_root_h */
