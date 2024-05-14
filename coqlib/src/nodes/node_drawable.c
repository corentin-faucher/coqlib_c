//
//  node_surface.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include "node_drawable.h"

#include "node_root.h"
#include "../graphs/graph_colors.h"
#include "../utils/util_base.h"
#include "../utils/util_language.h"
#include <math.h>
#include <stdlib.h>



static Drawable* drawable_last_ = NULL;

/// Mise à jour ordinaire de la matrice modèle pour un drawable.
Drawable* drawable_updateModel_(Node* const n) {
    Drawable* d = (Drawable*)n;
    float show = smtrans_setAndGetIsOnSmooth(&d->trShow, (d->n.flags & flag_show) != 0);
    // Rien à afficher...
    if(show < 0.001f)
        return NULL;
    const Node* const parent = n->_parent;
    if(!parent) { printwarning("Drawable without parent."); return NULL; }
    const Matrix4* const pm = &parent->_piu.model;
    d->n._piu.show = show;
    if((d->n.flags & flag_poping) == 0)
        show = 1.f;
    Matrix4* m = &n->_piu.model;
    Vector3 pos = node_pos(n);
    Vector2 scl = node_scales(n);
    m->v0.v = pm->v0.v * scl.x * show;
    m->v1.v = pm->v1.v * scl.y * show;
    m->v2 =   pm->v2;  // *scl.z ... si on veut un scaling en z...?
    m->v3 = (Vector4) {{
        pm->v3.x + pm->v0.x * pos.x + pm->v1.x * pos.y + pm->v2.x * pos.z,
        pm->v3.y + pm->v0.y * pos.x + pm->v1.y * pos.y + pm->v2.y * pos.z,
        pm->v3.z + pm->v0.z * pos.x + pm->v1.z * pos.y + pm->v2.z * pos.z,
        pm->v3.w,
    }};
    return d;
}

Drawable* (*Drawable_defaultUpdateModel)(Node*) = drawable_updateModel_; 

void     drawable_deinit_(Node* nd){
    Drawable* d = (Drawable*)nd;
    if(!mesh_isShared(d->_mesh))
        mesh_destroyAndNull(&d->_mesh);
    textureref_releaseAndNull_(&d->_tex);
}
void     drawable_updateDims__(Drawable* const d) {
    // 1. Delta Y : On a h * sy = 2*Dy.
    float beta = d->_tex->beta;
    d->n.sy = d->_twoDyTarget / beta;
    d->n.h  = beta;
    
    // 2. Delta X... Ici, il faut faire un dessin...
    float alpha = d->_tex->alpha;
    // Largeur a priori (prise du ratio de la texture).
    float sx = d->n.sy * d->_tex->ratio;
    // Largeur supplementaire (peut être négative)
    float extra_x = d->_xMargin * d->_twoDyTarget;
    // Si on fixe la largeur (largeur custom avec marges)
    if(d->_twoDxTarget > 0.f) {
        float sxCustom = fmaxf((d->_twoDxTarget - extra_x) / alpha, 0.f);
        if(d->n.flags & flag_drawableDontRespectRatio)
            sx = sxCustom; // Fixer direct sans tenir compte du ratio.
        else if((sxCustom > 0.01f*d->_twoDyTarget) && (sxCustom < sx))
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
        parent->h = d->_twoDyTarget;           // (Constant)
        if(parent->reshapeOpt)
            parent->reshapeOpt(parent);
    }
}
void     drawable_init_(Drawable* const d, Texture* const tex, Mesh* const mesh, 
                        float const twoDxOpt, float const twoDy, float const xMargin) {
    // Init
    smtrans_init(&d->trShow);
    smtrans_init(&d->trExtra);
    d->_tex = tex;
    d->_mesh = mesh;
    d->_twoDxTarget = twoDxOpt; d->_twoDyTarget = twoDy;
    d->_xMargin = xMargin;
    // Override de l'update de la matrice model.
    d->n.updateModel = Drawable_defaultUpdateModel;
    d->n.deinitOpt =   drawable_deinit_;
    d->n._piu.Du = 1.f / (float)umaxu(tex->m, 1);
    d->n._piu.Dv = 1.f / (float)umaxu(tex->n, 1);
    drawable_last_ = d;
    // Set des dimensions...
    drawable_updateDims__(d);
}
__attribute__((deprecated("utiliser `coq_calloc` + `node_init_` + `drawable_init_`.")))
Drawable* Drawable_createImageGeneral(Node* const refOpt,
                          Texture* tex, Mesh* const mesh,
                          float x, float y, float twoDxOpt, float twoDy, float x_margin,
                          flag_t flags, uint8_t node_place) {
    Drawable *d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_n_drawable, flags, node_place);
    if(!(tex->flags & tex_flag_png)) {
        printerror("Not a png."); tex = Texture_sharedImageByName("coqlib_the_cat");
    }
    drawable_init_(d, tex, mesh, twoDxOpt, twoDy, x_margin);
    return d;
}
Drawable* Drawable_createString(Node* const refOpt, StringDrawable const str,
                   float x, float y, float maxTwoDxOpt, float twoDy,
                   flag_t flags, uint8_t node_place) {
    if(mesh_sprite == NULL) printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_n_drawable, flags, node_place);
    drawable_init_(d, Texture_retainString(str), mesh_sprite, maxTwoDxOpt, twoDy, str.x_margin);
    return d;
}
void     drawable_updateAsMutableString(Drawable* d, const char* new_c_str, bool forceRedraw) {
    texture_updateMutableString(d->_tex, new_c_str, forceRedraw);
    drawable_updateDims__(d);
}
void     drawable_updateAsSharedString(Drawable* d, StringDrawable const str) {
    textureref_exchangeSharedStringFor(&d->_tex, str);
    drawable_updateDims__(d);
}
void      drawable_updateTargetDims(Drawable* d, float newTwoDxOpt, float newTwoDy, float newXMargin) {
    d->_twoDxTarget = newTwoDxOpt;
    d->_twoDyTarget = newTwoDy;
    d->_xMargin = newXMargin;
    drawable_updateDims__(d);
}
Drawable* Drawable_createImage(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL) printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_n_drawable, flags, 0);
    drawable_init_(d, Texture_sharedImage(pngId), mesh_sprite, 0, twoDy, 0);
    return d;
}
Drawable* Drawable_createImageWithFixedWidth(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDx, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL)
        printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_n_drawable, flags|flag_drawableDontRespectRatio, 0);
    drawable_init_(d, Texture_sharedImage(pngId), mesh_sprite, twoDx, twoDy, 0);
    return d;
}
Drawable* Drawable_createImageWithName(Node* const refOpt, const char* pngName,
                          float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL) printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_n_drawable, flags, 0);
    drawable_init_(d, Texture_sharedImageByName(pngName), mesh_sprite, 0, twoDy, 0);
    return d;
}
void      drawable_updatePngId(Drawable* d, uint32_t newPngId, bool updateDims) {
    if(!(d->_tex->flags & tex_flag_png)) {
        printerror("Not a png.");
        return;
    }
    // (Pour les png c'est safe de changer d'un coup. L'ancienne comme la nouvelle texture sont valide.)
    d->_tex = Texture_sharedImage(newPngId);
    d->n._piu.Du = 1.f / (float)umaxu(d->_tex->m, 1);
    d->n._piu.Dv = 1.f / (float)umaxu(d->_tex->n, 1);
    if(updateDims)
        drawable_updateDims__(d);
}

void      drawable_open_imagelanguage_(Node* nd) {
    Drawable* d = (Drawable*)nd;
    drawable_setTile(d, Language_current(), 0);
}
Drawable* Drawable_createImageLanguage(Node* refOpt, uint32_t pngId,
                                       float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite == NULL) printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_n_drawable, flags, 0);
    drawable_init_(d, Texture_sharedImage(pngId), mesh_sprite, 0, twoDy, 0);
    d->n.openOpt = drawable_open_imagelanguage_;
    return d;
}
Drawable* Drawable_createColor(Node* refOpt, Vector4 color,
                               float x, float y, float twoDx, float twoDy) {
    if(mesh_sprite == NULL) printerror("Missing Mesh_init().");
    Drawable* d = coq_calloc(1, sizeof(Drawable));
    node_init_(&d->n, refOpt, x, y, twoDy, twoDy, node_type_n_drawable, flag_drawableDontRespectRatio, 0);
    drawable_init_(d, texture_white, mesh_sprite, twoDx, twoDy, 0);
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
    d->n._piu.u0 = (float)( i % d->_tex->m)                   * d->n._piu.Du;
    d->n._piu.v0 = (float)((j + i / d->_tex->m) % d->_tex->n) * d->n._piu.Dv;
}
void      drawable_setTileI(Drawable *d, uint32_t i) {
    d->n._piu.u0 = (float)( i % d->_tex->m) * d->n._piu.Du;
}
void      drawable_setTileJ(Drawable *d, uint32_t j) {
    d->n._piu.v0 = (float)( j % d->_tex->n) * d->n._piu.Dv;
}
void      drawable_setTileFull(Drawable *d, InstanceTile const tile) {
    float du0 = 1.f / (float)umaxu(d->_tex->m, 1);
    float dv0 = 1.f / (float)umaxu(d->_tex->n, 1);
    d->n._piu.Du = (float)tile.Di * du0;
    d->n._piu.Dv = (float)tile.Dj * dv0;
    d->n._piu.u0 = (float)( tile.i % d->_tex->m)                        * du0;
    d->n._piu.v0 = (float)((tile.j + tile.i / d->_tex->m) % d->_tex->n) * dv0;
}
void      drawable_last_setTile(uint32_t i, uint32_t j) {
    if(drawable_last_ == NULL) {
        printwarning("No last drawable.");
        return;
    }
    uint32_t m = drawable_last_->_tex->m;
    uint32_t n = drawable_last_->_tex->n;
    drawable_last_->n._piu.u0 = (float)( i % m)          * drawable_last_->n._piu.Du;
    drawable_last_->n._piu.v0 = (float)((j + i / m) % n) * drawable_last_->n._piu.Dv;
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
    node_init_(&d->n, ref, 0, 0, 1, 1, node_type_n_drawable, flag_notToAlign, 0);
    drawable_init_(d, Texture_sharedImageByName("coqlib_test_frame"), mesh_sprite, 1, 1, 0);
    d->n.openOpt = drawable_open_testframe_getSizesOfParent_;
    if(ref->reshapeOpt) {
        ref->flags |= flag_parentOfReshapable;
        d->n.reshapeOpt = drawable_open_testframe_getSizesOfParent_;
    }
#endif
}
void   node_last_tryToAddTestFrame(void) {
    node_tryToAddTestFrame(node_last_nonDrawable);
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

