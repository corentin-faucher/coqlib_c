//
//  node_surface.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include <math.h>
#include <stdlib.h>
#include "drawable.h"
#include "root.h"
#include "utils.h"
#include "timer.h"
#include "colors.h"
#include "language.h"

void _drawable_deinit_freeMesh(Node* nd) {
    mesh_destroy(((Drawable*)nd)->mesh);
}
void _drawable_deinit_freeTexture(Node* nd){
    Drawable* surf = (Drawable*)nd;
    if(surf->tex)
        texture_destroy(surf->tex);
}
void _drawable_deinit_freeTextureAndMesh(Node* nd){
    Drawable* surf = (Drawable*)nd;
    mesh_destroy(surf->mesh);
    if(surf->tex)
        texture_destroy(surf->tex);
}

void _drawable_updateScaleYAndHeihgt(Drawable* const surf, float twoDy) {
    float beta = texture_beta(surf->tex);
    surf->n.sy = twoDy / beta;
    surf->n.h  = beta;
}
void _drawable_updateScaleXAndWidth(Drawable* const surf, float twoDxOpt, float twoDy) {
    Texture* texOpt = surf->tex;
    float alpha = texture_alpha(texOpt);
    // Ici, il faut faire un dessin...
    float extra_x = surf->x_margin * twoDy;
    float sx = fmaxf((twoDxOpt - extra_x) / alpha, 0.f);
    if(!(surf->n.flags & flag_drawableDontRespectRatio)) {
        float sxRat = surf->n.sy * texture_ratio(texOpt);
        // Si on prend le ratio, sxDef peut rester un max.
        if(sx < 2.f*extra_x + 0.0001f*twoDy || sxRat < sx)
            sx = sxRat;
    }
    surf->n.sx = sx;
    surf->n.w = alpha + extra_x/sx;  // (car 2dx = w*sx, sx = (2dx - e)/alpha... voir dessin.)
}

Drawable* _Drawable_create(Node* const refOpt,
                           float x, float y, flag_t flags, uint8_t node_place,
                           Texture* const tex, Mesh* const mesh) {
    Drawable *d = _Node_createEmptyOfType(node_type_leaf_drawable, sizeof(Drawable),
                                       flags, refOpt, node_place);
    d->n.x = x;
    d->n.y = y;
    smtrans_init(&d->trShow);
    smtrans_init(&d->trExtra);
    d->tex = tex;
    d->mesh = mesh;
    if(!texture_isShared(tex)) {
        d->n._type |= node_type_flag_notCopyable;
        if(!mesh_isShared(mesh))
            d->n.deinit = _drawable_deinit_freeTextureAndMesh;
        else
            d->n.deinit = _drawable_deinit_freeTexture;
    } else if(!mesh_isShared(mesh)) {
        d->n._type |= node_type_flag_notCopyable;
        d->n.deinit = _drawable_deinit_freeMesh;
    }
    return d;
}
Drawable* node_asDrawableOpt(Node* nd) {
    if(nd->_type & node_type_flag_drawable) return (Drawable*)nd;
    return NULL;
}
/*-- Surface d'image (png), peut etre une tile du png... ----------------------------------*/
Drawable* Drawable_createImage(Node* const refOpt, uint pngId,
                             float x, float y, float twoDy,
                          flag_t flags, uint8_t node_place) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    Drawable* surf = _Drawable_create(refOpt, x, y, flags, node_place,
         Texture_sharedImage(pngId), mesh_sprite);
    drawable_updateDimsWithDeltas(surf, 0.f, twoDy);
    return surf;
}
Drawable* Drawable_createImageWithName(Node* const refOpt, const char* pngName,
                          float x, float y, float twoDy,
                          flag_t flags, uint8_t node_place) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    Drawable* surf = _Drawable_create(refOpt, x, y, flags, node_place,
         Texture_sharedImageByName(pngName), mesh_sprite);
    drawable_updateDimsWithDeltas(surf, 0.f, twoDy);
    return surf;
}
Drawable* Drawable_createImageWithFixedWidth(Node* const refOpt, uint pngId,
                          float x, float y, float twoDx, float twoDy,
                          flag_t flags, uint8_t node_place) {
    if(mesh_sprite == NULL)
        printerror("Mesh not init.");
    Drawable* surf = _Drawable_create(refOpt, x, y, flags|flag_drawableDontRespectRatio, node_place,
         Texture_sharedImage(pngId), mesh_sprite);
    drawable_updateDimsWithDeltas(surf, twoDx, twoDy);
    return surf;
}
void      _node_drawable_imagelanguage_open(Node* nd) {
    Drawable* d = (Drawable*)nd;
    drawable_setTile(d, Language_current(), 0);
}
Drawable* Drawable_createImageLanguage(Node* refOpt, uint32_t pngId,
                                       float x, float y, float twoDy,
                                       flag_t flags, uint8_t node_place) {
    if(mesh_sprite == NULL)
        printerror("Mesh not init.");
    Drawable* d = _Drawable_create(refOpt, x, y, flags, node_place,
         Texture_sharedImage(pngId), mesh_sprite);
    d->n.open = _node_drawable_imagelanguage_open;
    drawable_updateDimsWithDeltas(d, 0.f, twoDy);
    return d;
}
void      drawable_updateDimsWithDeltas(Drawable* const d, float twoDxOpt, float twoDy) {
    _drawable_updateScaleYAndHeihgt(d, twoDy);
    _drawable_updateScaleXAndWidth(d, twoDxOpt, twoDy);
    // 3. Ajuster le frame (si besoin)
    if(d->n.flags & flag_giveSizeToBigbroFrame)
        node_tryUpdatingAsFrameOfBro(d->n.bigBro, &d->n);
    // 4. Donner les dimensions au parent (si besoin)
    Node* parent = d->n.parent;
    if((d->n.flags & flag_giveSizeToParent) && parent) {
        parent->w = node_deltaX(&d->n) * 2.f; // A été ajusté...
        parent->h = twoDy; // (pas change)
        if(parent->reshape)
            parent->reshape(parent);
    }
}
void      drawable_setTile(Drawable *d, uint i, uint j) {
    uint m = texture_m(d->tex);
    uint n = texture_n(d->tex);
    d->n.piu.i = (float)(i % m);
    d->n.piu.j = (float)((j + i / m) % n);
}
void      drawable_setTileI(Drawable *d, uint i) {
    uint m = texture_m(d->tex);
    d->n.piu.i = (float)(i % m);
}
void      drawable_setTileJ(Drawable *d, uint j) {
    uint n = texture_n(d->tex);
    d->n.piu.j = (float)(j % n);
}

/*-- Surface pour visualiser les dimension d'un noeud. (debug only) --*/
void   _drawable_open_testframe_getSizesOfParent(Node* nd) {
    Drawable* d = (Drawable*)nd;
    Node* p = nd->parent;
    if(!p) { printerror("No parent."); return; }
    d->n.w = 1.f;   d->n.h = 1.f;
    d->n.sx = p->w; d->n.sy = p->h;
}
void   node_tryToAddTestFrame(Node* ref) {
#ifndef DEBUG
    return;
#endif
    if(mesh_sprite == NULL) {
        printerror("Missing Mesh_init()."); return;
    }
    if(!ref) {
        printerror("No parent."); return;
    }
    Drawable* d = _Drawable_create(ref, 0.f, 0.f, 0, 0,
                                   Texture_sharedImageByName("coqlib_test_frame"), mesh_sprite);
    d->n.open = _drawable_open_testframe_getSizesOfParent;
    if(ref->reshape) {
        ref->flags |= flag_parentOfReshapable;
        d->n.reshape = _drawable_open_testframe_getSizesOfParent;
    }
}
void   node_last_tryToAddTestFrame(void) {
    node_tryToAddTestFrame(_node_last_created);
}

/*-- Surface de String constante. -----------------------------------------------------------*/
Drawable* Drawable_createConstantString(Node* const refOpt, const char* c_str,
                              float x, float y, float maxTwoDxOpt, float twoDy,
                              flag_t flags, uint8_t node_place) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    Drawable* d = _Drawable_create(refOpt, x, y, flags, node_place,
                                   Texture_sharedConstantString(c_str), mesh_sprite);
    d->n.piu.color = color4_black;
    drawable_updateDimsWithDeltas(d, maxTwoDxOpt, twoDy);
    return d;
}
/*-- Surface de String localisee. -----------------------------------------------------------*/
Drawable* Drawable_createString(Node* const refOpt, UnownedString str,
                   float x, float y, float maxTwoDxOpt, float twoDy,
                   flag_t flags, uint8_t node_place) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    Drawable* d = _Drawable_create(refOpt, x, y, flags, node_place,
                                   Texture_createString(str), mesh_sprite);
    d->n.piu.color = color4_black;
    drawable_updateDimsWithDeltas(d, maxTwoDxOpt, twoDy);
    return d;
}

/*-- Surface "Fan" (un rond). --------------------------------------------------------------*/
Drawable* Drawable_createFan(Node* const refOpt, uint32_t pngId,
                             float x, float y, float twoDy,
                             flag_t flags, uint8_t node_place) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    Drawable* d = _Drawable_create(refOpt, x, y, flags, node_place,
         Texture_sharedImage(pngId), Mesh_createFan());
    d->n.w = 1.f;
    d->n.h = 1.f;
    d->n.sx = 2.f*twoDy;
    d->n.sy = 2.f*twoDy;
    return d;
}
Drawable* node_defaultUpdateModelAndGetAsDrawableOpt(Node* const node) {
    // 0. Cas root
    {
        Root* root = node_asRootOpt(node);
        if(root) {
            root_updateModelMatrix(root);
            return NULL;
        }
    }
    Node* const parent = node->parent;
    if(parent == NULL) {
        printerror("Non-root without parent.");
        return NULL;
    }
    // 1. Cas branche
    if(node->firstChild != NULL) {
        node_updateModelMatrixWithParentModel(node, &parent->piu.model, 1.f);
        return NULL;
    }
    // 3. Cas feuille
    Drawable* d = node_asDrawableOpt(node);
    // Laisser faire si n'est pas affichable...
    if(!d) return NULL;
    // Facteur d'"affichage"
    float alpha = smtrans_setAndGetIsOnSmooth(&d->trShow, (d->n.flags & flag_show) != 0);
    // Rien à afficher...
    if(alpha < 0.001f)
        return NULL;
    d->n.piu.show = alpha;
    if((d->n.flags & flag_poping) == 0)
        alpha = 1.f;
    node_updateModelMatrixWithParentModel(&d->n, &parent->piu.model, alpha);
    return d;
}
