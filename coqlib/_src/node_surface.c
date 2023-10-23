//
//  node_surface.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include <math.h>
#include <stdlib.h>
#include "node_surface.h"
#include "node_root.h"
#include "utils.h"

void _node_surf_denit_freeMesh(Node* nd) {
    mesh_destroy(((NodeSurf*)nd)->_mesh);
}
void _nodesurf_updateScaleYAndHeihgt(NodeSurf* const surf, float twoDy) {
    float beta = texture_beta(surf->texOpt);
    surf->nd.sy = twoDy / beta;
    surf->nd.h  = beta;
}
void _nodesurf_updateScaleXAndWidth(NodeSurf* const surf, float twoDx, float twoDy) {
    Texture* texOpt = surf->texOpt;
    float alpha = texture_alpha(texOpt);
    // Ici, il faut faire un dessin...
    float extra_x = surf->x_margin * twoDy;
    float sx = fmaxf((twoDx - extra_x) / alpha, 0.f);
    if(!(surf->nd.flags & flag_surfaceDontRespectRatio)) {
        float sxRat = surf->nd.sy * texture_ratio(texOpt);
        // Si on prend le ratio, sxDef peut rester un max.
        if(sx < 2.f*extra_x + 0.0001f*twoDy || sxRat < sx)
            sx = sxRat;
    }
    surf->nd.sx = sx;
    surf->nd.w = alpha + extra_x/sx;  // (car 2dx = w*sx, sx = (2dx - e)/alpha... voir dessin.)
}

NodeSurf* _NodeSurf_create(Node* const refOpt,
                          float x, float y, float twoDx, float twoDy,
                          flag_t flags, uint8_t node_place,
                          Texture* const texOpt, Mesh* const mesh, Bool isMeshOwner) {
    Node *nd = _Node_createEmptyOfType(node_type_surface, sizeof(NodeSurf),
                                       flags, refOpt, node_place);
    nd->x = x;
    nd->y = y;
    NodeSurf *surf = (NodeSurf*)nd;
    smtrans_init(&surf->trShow);
    smtrans_init(&surf->trExtra);
    surf->texOpt = texOpt;
    surf->_mesh = mesh;
    if(isMeshOwner)
        nd->deinit = _node_surf_denit_freeMesh;
#warning Ne pas faire par defaut...?
    nodesurf_updateDimsWithDeltas(surf, twoDx, twoDy);
    
    return surf;
}
NodeSurf* NodeSurf_createPng(Node* const refOpt, float x, float y, float twoDy,
                          flag_t flags, uint8_t node_place,
                          uint pngId) {
    printdebug("mesh_sprite %p", mesh_sprite);
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    return _NodeSurf_create(refOpt, x, y, -1.f, twoDy, flags, node_place,
                           Texture_getPngTexture(pngId), mesh_sprite, false);
}
NodeSurf* NodeSurf_createBar(Node* const refOpt, enum Framing framing,
                             float delta, float twoDx, uint pngId) {
    Mesh* meshBar = Mesh_createBar();
    NodeSurf* surf = _NodeSurf_create(refOpt, 0, 0, twoDx, 2.f*delta,
        flag_surfaceDontRespectRatio, 0, Texture_getPngTexture(pngId),
        meshBar, true);
    surf->framing = framing;
    surf->delta = delta;
    return surf;
}
void nodesurf_bar_update(NodeSurf* bar, float twoDx, Bool fix) {
    float smallDeltaX;
    switch(bar->framing) {
        case framing_inside:  smallDeltaX = fmaxf(0.f, 0.5f * twoDx - 2*bar->delta); break;
        case framing_center:  smallDeltaX = fmaxf(0.f, 0.5f * twoDx -   bar->delta); break;
        case framing_outside: smallDeltaX =            0.5f * twoDx;                 break;
    }
    float xPos = 0.5f * smallDeltaX / (smallDeltaX + 2.f * bar->delta);
    float actualTwoDx = 2 * (smallDeltaX + 2 * bar->delta);
    // Ici, on suppose que twoDy = sy, i.e. beta = 1.
    _nodesurf_updateScaleXAndWidth(bar, actualTwoDx, bar->nd.sy);
    bar->_mesh->vertices[2].x = -xPos;
    bar->_mesh->vertices[3].x = -xPos;
    bar->_mesh->vertices[4].x =  xPos;
    bar->_mesh->vertices[5].x =  xPos;
}

void nodesurf_updateDimsWithDeltas(NodeSurf* const surf, float twoDx, float twoDy) {
    _nodesurf_updateScaleYAndHeihgt(surf, twoDy);
    _nodesurf_updateScaleXAndWidth(surf, twoDx, twoDy);
    // 3. Ajuster le frame (si besoin)
    Node* bro = surf->nd.bigBro;
    if((surf->nd.flags & flag_giveSizeToBigbroFrame) && bro)
        if((bro->_type & node_type_surface) && (bro->flags == flag_surfaceIsFrame))
            nodesurf_frame_updateWithLittleBro((NodeSurf*)bro);
    // 4. Donner les dimensions au parent (si besoin)
    Node* parent = surf->nd.parent;
    if((surf->nd.flags & flag_giveSizeToParent) && parent) {
        parent->w = node_deltaX(&surf->nd) * 2.f; // A été ajusté...
        parent->h = twoDy; // (pas change)
        if(parent->reshape)
            parent->reshape(parent);
    }
}

void      nodesurf_setTile(NodeSurf *surf, uint i, uint j) {
    uint m = texture_m(surf->texOpt);
    uint n = texture_n(surf->texOpt);
    surf->nd.piu.i = (float)(i % m);
    surf->nd.piu.j = (float)((j + i / m) % n);
}
void      nodesurf_setTileI(NodeSurf *surf, uint i) {
    uint m = texture_m(surf->texOpt);
    surf->nd.piu.i = (float)(i % m);
}
void      nodesurf_setTileJ(NodeSurf *surf, uint j) {
    uint n = texture_n(surf->texOpt);
    surf->nd.piu.j = (float)(j % n);
}

void      nodesurf_frame_updateWithLittleBro(NodeSurf * surf) {
#warning TODO
}

NodeSurf* node_asSurfOpt(Node* nd) {
    return (nd->_type & node_type_surface) ? (NodeSurf*)nd : NULL;
}

NodeSurf* node_defaultUpdateForDrawingAndGetAsSurfOpt(Node* const node) {
    // 0. Cas root
    if(node->_type & node_type_root) {
        noderoot_updateModelMatrix((NodeRoot*)node);
        return NULL;
    }
    Node* const parent = node->parent;
    if(parent == NULL) {
        printerror("Non root sans parent.");
        return NULL;
    }
    // 1. Cas branche
    if(node->firstChild != NULL) {
        node_updateModelMatrixWithParentReferential(node, parent);
        return NULL;
    }
    // 3. Cas feuille
    // Laisser faire si n'est pas affichable...
    if((node->_type & node_type_surface) == 0)
        return NULL;
    NodeSurf* surface = (NodeSurf*)node;
    
    // Facteur d'"affichage"
    float alpha = smtrans_setAndGetIsOnSmooth(&surface->trShow, (node->flags & flag_show) != 0);
    // Rien à afficher...
    if(alpha < 0.001f) {
        return NULL;
    }
    node->piu.show = alpha;
    // Model matrix
    
    
    node_updateModelMatrixWithParentReferential(node, parent);
    
    return surface;
}
