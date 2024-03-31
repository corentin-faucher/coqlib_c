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
#include "utils/utils_base.h"
#include "utils/utils_languages.h"

static Drawable* drawable_last_ = NULL;

void drawable_deinit_freeTexture_(Node* n){
    textureref_releaseAndNull_(&((Drawable*)n)->_tex);
}
void drawable_deinit_freeTextureAndMesh_(Node* nd){
    Drawable* d = (Drawable*)nd;
    mesh_destroyAndNull(&d->_mesh);
    textureref_releaseAndNull_(&d->_tex);
}
void     drawable_init_(Drawable* d, Texture* tex, Mesh* mesh, float twoDxOpt, float twoDy) {
    smtrans_init(&d->trShow);
    smtrans_init(&d->trExtra);
    d->_tex = tex;
    d->_mesh = mesh;
    d->twoDxTarget = twoDxOpt; d->twoDyTarget = twoDy;
    if(mesh_isShared(mesh)) {
        d->n.deinitOpt = drawable_deinit_freeTexture_;
    } else {
        d->n.deinitOpt = drawable_deinit_freeTextureAndMesh_;
    }
    drawable_last_ = d;
}
void     drawable_updateDims_(Drawable* const d) {
    // 1. Delta Y : On a h * sy = 2*Dy.
    float beta = texture_beta(d->_tex);
    d->n.sy = d->twoDyTarget / beta;
    d->n.h  = beta;
    
    // 2. Delta X... Ici, il faut faire un dessin...
    float alpha = texture_alpha(d->_tex);
    // Largeur a priori (prise du ratio de la texture).
    float sx = d->n.sy * texture_ratio(d->_tex);
    // Largeur supplementaire (peut être négative)
    float extra_x = d->x_margin * d->twoDyTarget;
    // Si on fixe la largeur (largeur custom avec marges)
    if(d->twoDxTarget > 0.f) {
        float sxCustom = fmaxf((d->twoDxTarget - extra_x) / alpha, 0.f);
        if(d->n.flags & flag_drawableDontRespectRatio)
            sx = sxCustom; // Fixer direct sans tenir compte du ratio.
        else if((sxCustom > 0.01f*d->twoDyTarget) && (sxCustom < sx))
            sx = sxCustom; // Fixer pour plafonner la largeur a priori.
    }
    d->n.sx = sx;
    d->n.w = fmaxf(alpha + extra_x/sx, 0.01f);
    // (car 2dx = w*sx, sx = (2dx - e)/alpha... voir dessin.)
    
    // 3. Ajuster le bigbro frame (si besoin)
    Node* bigbro = d->n._bigBro;
    if((d->n.flags & flag_giveSizeToBigbroFrame) && bigbro)
        node_tryUpdatingAsFrameOfBro(bigbro, &d->n);
    // 4. Donner les dimensions au parent (si besoin)
    Node* parent = d->n._parent;
    if((d->n.flags & flag_giveSizeToParent) && parent) {
        parent->w = node_deltaX(&d->n) * 2.f; // A été ajusté...
        parent->h = d->twoDyTarget;           // (Constant)
        if(parent->reshapeOpt)
            parent->reshapeOpt(parent);
    }
}

__attribute__((deprecated("utiliser `coq_calloc` + `node_init_` + `drawable_init_`.")))
Drawable* Drawable_createImageGeneral(Node* const refOpt,
                          Texture* tex, Mesh* const mesh,
                          float x, float y, float twoDxOpt, float twoDy, float x_margin,
                          flag_t flags, uint8_t node_place) {
    Drawable *d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_leaf_drawable, flags, node_place);
    if(!texture_isSharedPng(tex)) {
        printerror("Not a png."); tex = Texture_sharedImageByName("coqlib_the_cat");
    }
    drawable_init_(d, tex, mesh, twoDxOpt, twoDy);
    d->x_margin = x_margin;
    drawable_updateDims_(d);
    return d;
}
Drawable* Drawable_createString(Node* const refOpt, StringDrawable const str,
                   float x, float y, float maxTwoDxOpt, float twoDy,
                   flag_t flags, uint8_t node_place) {
    if(mesh_sprite == NULL) printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_leaf_drawable, flags, node_place);
    drawable_init_(d, Texture_retainString(str), mesh_sprite, maxTwoDxOpt, twoDy);
    d->x_margin = str.x_margin;
    drawable_updateDims_(d);
    return d;
}
void     drawable_updateAsMutableString(Drawable* d, const char* new_c_str, bool forceRedraw) {
    texture_updateMutableString(d->_tex, new_c_str, forceRedraw);
    drawable_updateDims_(d);
}
void     drawable_updateAsSharedString(Drawable* d, StringDrawable const str) {
    textureref_exchangeSharedStringFor(&d->_tex, str);
    drawable_updateDims_(d);
}
Drawable* Drawable_createImage(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL) printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_leaf_drawable, flags, 0);
    drawable_init_(d, Texture_sharedImage(pngId), mesh_sprite, 0, twoDy);
    drawable_updateDims_(d);
    return d;
}
Drawable* Drawable_createImageWithFixedWidth(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDx, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_leaf_drawable, flags|flag_drawableDontRespectRatio, 0);
    drawable_init_(d, Texture_sharedImage(pngId), mesh_sprite, twoDx, twoDy);
    drawable_updateDims_(d);
    return d;
}
Drawable* Drawable_createImageWithName(Node* const refOpt, const char* pngName,
                          float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL) printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_leaf_drawable, flags, 0);
    drawable_init_(d, Texture_sharedImageByName(pngName), mesh_sprite, 0, twoDy);
    drawable_updateDims_(d);
    return d;
}
void      drawable_updatePngId(Drawable* d, uint32_t newPngId) {
    if(!texture_isSharedPng(d->_tex)) {
        printerror("Not a png.");
        return;
    }
    // (Pour les png c'est safe de changer d'un coup. L'ancienne comme la nouvelle texture sont valide.)
    d->_tex = Texture_sharedImage(newPngId);
}

void      drawable_open_imagelanguage_(Node* nd) {
    Drawable* d = (Drawable*)nd;
    drawable_setTile(d, Language_current(), 0);
}
Drawable* Drawable_createImageLanguage(Node* refOpt, uint32_t pngId,
                                       float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL) printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_leaf_drawable, flags, 0);
    drawable_init_(d, Texture_sharedImage(pngId), mesh_sprite, 0, twoDy);
    drawable_updateDims_(d);
    d->n.openOpt = drawable_open_imagelanguage_;
    return d;
}
Drawable* Drawable_createColor(Node* refOpt, Vector4 color,
                               float x, float y, float twoDx, float twoDy) {
    if(mesh_sprite == NULL) printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_leaf_drawable, flag_drawableDontRespectRatio, 0);
    drawable_init_(d, Texture_sharedImageByName("coqlib_white"), mesh_sprite, twoDx, twoDy);
    drawable_updateDims_(d);
    d->n._piu.color = color;
    return d;
}

Drawable* node_asDrawableOpt(Node* nd) {
    if(nd->_type & node_type_flag_drawable) return (Drawable*)nd;
    return NULL;
}
void  drawableref_fastDestroyAndNull(Drawable** const drawableOptRef) {
    if(*drawableOptRef == NULL) return;
    node_tree_throwToGarbage(&(*drawableOptRef)->n);
    *drawableOptRef = NULL;
}
int       node_isDisplayActive(Node* const node) {
    if(node->_type & node_type_flag_drawable) {
        return smtrans_isActive(((Drawable*)node)->trShow);
    }
    return node->flags & (flag_show|flag_parentOfToDisplay);
}


/*-- Setters --------------------------------------------------------------------*/
void      drawable_setTile(Drawable *d, uint32_t i, uint32_t j) {
    uint32_t m = texture_m(d->_tex);
    uint32_t n = texture_n(d->_tex);
    d->n._piu.i = (float)(i % m);
    d->n._piu.j = (float)((j + i / m) % n);
}
void      drawable_setTileI(Drawable *d, uint32_t i) {
    uint32_t m = texture_m(d->_tex);
    d->n._piu.i = (float)(i % m);
}
void      drawable_setTileJ(Drawable *d, uint32_t j) {
    uint32_t n = texture_n(d->_tex);
    d->n._piu.j = (float)(j % n);
}
void      drawable_last_setTile(uint32_t i, uint32_t j) {
    if(drawable_last_ == NULL) {
        printwarning("No last drawable.");
        return;
    }
    uint32_t m = texture_m(drawable_last_->_tex);
    uint32_t n = texture_n(drawable_last_->_tex);
    drawable_last_->n._piu.i = (float)(i % m);
    drawable_last_->n._piu.j = (float)((j + i / m) % n);
}
void      drawable_last_setEmph(float emph) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    drawable_last_->n._piu.emph = emph;
}
void      drawable_last_setShowOptions(bool isHard, bool isPoping) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    smtrans_setOptions(&drawable_last_->trShow, isHard, isPoping);
}
void      drawable_last_setColor(Vector4 color) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    drawable_last_->n._piu.color = color;
}


/*-- Surface pour visualiser les dimension d'un noeud. (debug only) --*/
void   drawable_open_testframe_getSizesOfParent_(Node* nd) {
    Drawable* d = (Drawable*)nd;
    Node* p = nd->_parent;
    if(!p) { printerror("No parent."); return; }
    d->n.w = 1.f;   d->n.h = 1.f;
    d->n.sx = p->w; d->n.sy = p->h;
}
void   node_tryToAddTestFrame(Node* ref) {
#ifdef DEBUG
    if(mesh_sprite == NULL) { printerror("Missing Mesh_init()."); return; }
    if(!ref) { printerror("No parent."); return; }
    Drawable *d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, ref, 0, 0, 1, 1, node_type_leaf_drawable, flag_notToAlign, 0);
    drawable_init_(d, Texture_sharedImageByName("coqlib_test_frame"), mesh_sprite, 1, 1);
    d->n.openOpt = drawable_open_testframe_getSizesOfParent_;
    if(ref->reshapeOpt) {
        ref->flags |= flag_parentOfReshapable;
        d->n.reshapeOpt = drawable_open_testframe_getSizesOfParent_;
    }
#endif
}
void   node_last_tryToAddTestFrame(void) {
    node_tryToAddTestFrame(node_last_nonLeaf);
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
    Node* const parent = node->_parent;
    if(parent == NULL) {
        printerror("Non-root without parent.");
        return NULL;
    }
    // 1. Cas branche
    if(node->_firstChild != NULL) {
        node_updateModelMatrixWithParentModel(node, &parent->_piu.model, 1.f);
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
    d->n._piu.show = alpha;
    if((d->n.flags & flag_poping) == 0)
        alpha = 1.f;
    node_updateModelMatrixWithParentModel(&d->n, &parent->_piu.model, alpha);
    return d;
}
