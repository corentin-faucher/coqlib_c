//
//  node_surface.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include <math.h>
#include <stdlib.h>
#include "nodes/node_drawable.h"
#include "nodes/node_root.h"
#include "graphs/graph_colors.h"

static Drawable* drawable_last_ = NULL;

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
    // Largeur a priori (prise du ratio de la texture).
    float sx = surf->n.sy * texture_ratio(texOpt);
    // Largeur supplementaire (peut être négative)
    float extra_x = surf->x_margin * twoDy;
    // Si on fixe la largeur (largeur custom avec marges)
    if(twoDxOpt > 0.f) {
        float sxCustom = fmaxf((twoDxOpt - extra_x) / alpha, 0.f);
        if(surf->n.flags & flag_drawableDontRespectRatio)
            sx = sxCustom; // Fixer direct sans tenir compte du ratio.
        else if((sxCustom > 0.01f*twoDy) && (sxCustom < sx))
            sx = sxCustom; // Fixer pour plafonner la largeur a priori.
    }
    surf->n.sx = sx;
    surf->n.w = fmaxf(alpha + extra_x/sx, 0.01f);
    // (car 2dx = w*sx, sx = (2dx - e)/alpha... voir dessin.)
}
void     drawable_init(Drawable* d, Texture* tex, Mesh* mesh) {
    smtrans_init(&d->trShow);
    smtrans_init(&d->trExtra);
    d->tex = tex;
    d->mesh = mesh;
    if(!texture_isShared(tex)) {
        d->n._type |= node_type_flag_notCopyable;
        if(!mesh_isShared(mesh))
            d->n.deinitOpt = _drawable_deinit_freeTextureAndMesh;
        else
            d->n.deinitOpt = _drawable_deinit_freeTexture;
    } else if(!mesh_isShared(mesh)) {
        d->n._type |= node_type_flag_notCopyable;
        d->n.deinitOpt = _drawable_deinit_freeMesh;
    }
    drawable_last_ = d;
}
Drawable* Drawable_create(Node* const refOpt,
                          Texture* const tex, Mesh* const mesh,
                          flag_t flags, uint8_t node_place) {
    Drawable *d = Node_createEmptyOfType_(node_type_leaf_drawable, sizeof(Drawable),
                                       flags, refOpt, node_place);
    drawable_init(d, tex, mesh);
    return d;
}
Drawable* Drawable_createAndSetDims(Node* const refOpt,
                                    float x, float y, float twoDxOpt, float twoDy,
                                    Texture* const tex, Mesh* const mesh,
                                    flag_t flags, uint8_t node_place) {
    Drawable *d = Node_createEmptyOfType_(node_type_leaf_drawable, sizeof(Drawable),
                                        flags, refOpt, node_place);
    d->n.x = x; d->n.y = y;
    drawable_init(d, tex, mesh);
    drawable_updateDimsWithDeltas(d, twoDxOpt, twoDy);
    return d;
}
Drawable* node_asDrawableOpt(Node* nd) {
    if(nd->_type & node_type_flag_drawable) return (Drawable*)nd;
    return NULL;
}
/*-- Surface d'image (png), peut etre une tile du png... ----------------------------------*/
Drawable* Drawable_createImage(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    return Drawable_createAndSetDims(refOpt, x, y, 0.f, twoDy,
                                     Texture_sharedImage(pngId), mesh_sprite, flags, 0);
}
Drawable* Drawable_createImageWithName(Node* const refOpt, const char* pngName,
                          float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    return Drawable_createAndSetDims(refOpt, x, y, 0.f, twoDy,
                                     Texture_sharedImageByName(pngName), mesh_sprite, flags, 0);
}
void      _drawable_open_imagelanguage(Node* nd) {
    Drawable* d = (Drawable*)nd;
    drawable_setTile(d, Language_current(), 0);
}
Drawable* Drawable_createImageLanguage(Node* refOpt, uint32_t pngId,
                                       float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL)
        printerror("Mesh not init.");
    Drawable *d =  Drawable_createAndSetDims(refOpt, x, y, 0.f, twoDy,
                                     Texture_sharedImage(pngId), mesh_sprite, flags, 0);
    d->n.openOpt = _drawable_open_imagelanguage;
    return d;
}


/*-- Setters --------------------------------------------------------------------*/

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
        if(parent->reshapeOpt)
            parent->reshapeOpt(parent);
    }
}
void      drawable_setTile(Drawable *d, uint32_t i, uint32_t j) {
    uint32_t m = texture_m(d->tex);
    uint32_t n = texture_n(d->tex);
    d->n.piu.i = (float)(i % m);
    d->n.piu.j = (float)((j + i / m) % n);
}
void      drawable_last_setTile(uint32_t i, uint32_t j) {
    if(drawable_last_ == NULL) {
        printwarning("No last drawable.");
        return;
    }
    uint32_t m = texture_m(drawable_last_->tex);
    uint32_t n = texture_n(drawable_last_->tex);
    drawable_last_->n.piu.i = (float)(i % m);
    drawable_last_->n.piu.j = (float)((j + i / m) % n);
}
void      drawable_last_setEmph(float emph) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    drawable_last_->n.piu.emph = emph;
}
void      drawable_last_setShowOptions(bool isHard, bool isPoping) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    smtrans_setOptions(&drawable_last_->trShow, isHard, isPoping);
}

void      drawable_setTileI(Drawable *d, uint32_t i) {
    uint32_t m = texture_m(d->tex);
    d->n.piu.i = (float)(i % m);
}
void      drawable_setTileJ(Drawable *d, uint32_t j) {
    uint32_t n = texture_n(d->tex);
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
    Drawable *d = Node_createEmptyOfType_(node_type_leaf_drawable, sizeof(Drawable),
                                          0, ref, 0);
    drawable_init(d, Texture_sharedImageByName("coqlib_test_frame"), mesh_sprite);
    d->n.openOpt = _drawable_open_testframe_getSizesOfParent;
    if(ref->reshapeOpt) {
        ref->flags |= flag_parentOfReshapable;
        d->n.reshapeOpt = _drawable_open_testframe_getSizesOfParent;
    }
}
void   node_last_tryToAddTestFrame(void) {
    node_tryToAddTestFrame(node_last_nonLeaf);
}

/*-- Surface de String constante. -----------------------------------------------------------*/
Drawable* Drawable_createConstantString(Node* const refOpt, const char* c_str,
                              float x, float y, float maxTwoDxOpt, float twoDy,
                              flag_t flags) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    Drawable* d = Drawable_createAndSetDims(refOpt, x, y, maxTwoDxOpt, twoDy,
                       Texture_sharedConstantString(c_str), mesh_sprite, flags, 0);
    if(stringUTF8_isSingleEmoji(c_str))
        d->n.piu.color = color4_white;
    else
        d->n.piu.color = color4_black;
    return d;
}
/*-- Surface de String localisee. -----------------------------------------------------------*/
Drawable* Drawable_createString(Node* const refOpt, UnownedString str,
                   float x, float y, float maxTwoDxOpt, float twoDy,
                   flag_t flags) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    Drawable* d = Drawable_createAndSetDims(refOpt, x, y, maxTwoDxOpt, twoDy,
                       Texture_createString(str, false), mesh_sprite, flags, 0);
    d->n.piu.color = color4_black;
    drawable_updateDimsWithDeltas(d, maxTwoDxOpt, twoDy);
    return d;
}

/*-- Surface "Fan" (un rond). Utile ? -------------------------------------------------------*/
//Drawable* Drawable_createFan(Node* const refOpt, uint32_t pngId,
//                             float x, float y, float twoDy,
//                             flag_t flags, uint8_t node_place) {
//    if(mesh_sprite == NULL)
//        printerror("Missing Mesh_init().");
//    Drawable* d = Drawable_create(refOpt, Texture_sharedImage(pngId), Mesh_createFan(),
//                                  flags, node_place);
//    d->n.x = x;        d->n.y = y;
//    d->n.w = 1.f;      d->n.h = 1.f;
//    d->n.sx = twoDy;   d->n.sy = twoDy;
//    return d;
//}

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
