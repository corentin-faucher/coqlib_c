//
//  node_surface.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#ifndef node_surface_h
#define node_surface_h

#include <stdio.h>
#include "node.h"
#include "smtrans.h"

enum Framing {
    framing_outside,
    framing_center,
    framing_inside,
};

typedef struct _Surface {
    Node     nd;    // Peut être casté comme un noeud.
    SmTrans  trShow;
    Texture* texOpt;   // Optionel ou toujours present ?
    Mesh*    _mesh;
    // Les extras... Faire des sous-classes ?
    float    x_margin;  // (String surface)
    float delta;        // (Frame/Bar)
    enum Framing framing;
    SmTrans  trExtra;   // Transition en extra !
    
} NodeSurf;



/// Constructeur general
NodeSurf* _NodeSurf_create(Node* const refOpt,
                          float x, float y, float twoDx, float twoDy,
                          flag_t flags, uint8_t node_place,
                          Texture* const texOpt, Mesh* const mesh, Bool isMeshOwner);
NodeSurf* NodeSurf_createPng(Node* refOpt, float x, float y, float twoDy,
                             flag_t flags, uint8_t node_place,
                             uint pngId);

void      nodesurf_updateDimsWithDeltas(NodeSurf *surf, float twoDx, float twoDy);

void      nodesurf_setTile(NodeSurf *surf, uint i, uint j);
void      nodesurf_setTileI(NodeSurf *surf, uint i);
void      nodesurf_setTileJ(NodeSurf *surf, uint j);

void      nodesurf_frame_updateWithLittleBro(NodeSurf * surf);

NodeSurf* node_asSurfOpt(Node* nd);
NodeSurf* node_defaultUpdateForDrawingAndGetAsSurfOpt(Node* const node);

#endif /* node_surface_h */
