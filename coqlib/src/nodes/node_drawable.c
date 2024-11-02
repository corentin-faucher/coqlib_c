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
void drawable_renderer_updateIU_(Node* const n) {
    Drawable* d = (Drawable*)n;
    float show = smtrans_setAndGetValue(&d->trShow, (d->n.flags & flag_show) != 0);
    // Rien à afficher...
    if(show < 0.001f) {
        n->_iu.render_flags &= ~renderflag_toDraw;
        return;
    }
    n->_iu.render_flags |= renderflag_toDraw;
    const Matrix4* const pm = node_parentModel(n);
    d->n._iu.show = show;
    if((d->n.flags & flag_drawablePoping) == 0)
        show = 1.f;
    Matrix4* m = &n->_iu.model;
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
}
void (*Drawable_renderer_defaultUpdateInstanceUniforms)(Node*) = drawable_renderer_updateIU_;

#pragma mark -- Init/constructors ---------------------------------------
void      drawable_deinit_(Node* nd){
    Drawable* d = (Drawable*)nd;
    if(!d->_mesh->isShared) {
        mesh_engine_deinit(d->_mesh);
        coq_free(d->_mesh);
        d->_mesh = NULL;
    }
    textureref_releaseAndNull_(&d->_tex);
}
void      drawable_init(Drawable* const d, Texture* tex, Mesh* mesh,
                        float const twoDxOpt, float const twoDy) {
    if(tex == NULL) {printerror("No texture."); tex = Texture_white; }
    if(mesh == NULL) { printwarning("No mesh."); mesh = &mesh_sprite; }
    // Init
    smtrans_init(&d->trShow);
    smtrans_init(&d->trExtra);
    d->_tex = tex;
    d->_mesh = mesh;
    d->n._type |= node_type_flag_drawable;
    // Override de l'update de la matrice model.
    d->n.renderer_updateInstanceUniforms = Drawable_renderer_defaultUpdateInstanceUniforms;
    d->n.deinitOpt =   drawable_deinit_;
    d->n._iu.draw_color =   color4_white;
    d->n._iu.draw_uvRect = (Rectangle) { .size = texture_tileDuDv(tex) };
    drawable_last_ = d;
    // Set des dimensions...
    d->n.w = 1; d->n.h = 1;
    d->n.sy = twoDy;
    if(twoDxOpt)
        d->n.sx = twoDxOpt;
    else {
        d->n.sx = twoDy * texture_tileRatio(tex);
   }
}
Drawable* Drawable_createImage(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite.vertex_count == 0) printerror("Missing Mesh_init().");
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    drawable_init(d, Texture_sharedImage(pngId), &mesh_sprite, 0, twoDy);
    return d;
}
Drawable* Drawable_createImageWithWidth(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDx, float twoDy, flag_t flags) {
    if(mesh_sprite.vertex_count == 0)
        printerror("Missing Mesh_init().");
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    drawable_init(d, Texture_sharedImage(pngId), &mesh_sprite, twoDx, twoDy);
    return d;
}
Drawable* Drawable_createImageWithName(Node* const refOpt, const char* pngName,
                          float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite.vertex_count == 0) printerror("Missing Mesh_init().");
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    drawable_init(d, Texture_sharedImageByName(pngName), &mesh_sprite, 0, twoDy);
    return d;
}
void      drawable_open_imagelanguage_(Node* nd) {
    Drawable* d = (Drawable*)nd;
    drawable_setTile(d, Language_current(), 0);
}
Drawable* Drawable_createImageLanguage(Node* refOpt, uint32_t pngId,
                                       float x, float y, float twoDy, flag_t flags) {
    if(mesh_sprite.vertex_count == 0) printerror("Missing Mesh_init().");
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    drawable_init(d, Texture_sharedImage(pngId), &mesh_sprite, 0, twoDy);
    d->n.openOpt = drawable_open_imagelanguage_;
    return d;
}
Drawable* Drawable_createColor(Node* refOpt, Vector4 color,
                               float x, float y, float twoDx, float twoDy) {
    if(mesh_sprite.vertex_count == 0) printerror("Missing Mesh_init().");
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, 0, 0);
    drawable_init(d, Texture_white, &mesh_sprite, twoDx, twoDy);
    d->n._iu.draw_color = color;
    return d;
}
Drawable* Drawable_createTestFrame(Node* parent, float x, float y, float twoDx, float twoDy) {
    if(mesh_sprite.vertex_count == 0) printerror("Missing Mesh_init().");
    Drawable *d = coq_callocTyped(Drawable);
    node_init(&d->n, parent, x, y, 1, 1, 0, 0);
    drawable_init(d, Texture_sharedImageByName("coqlib_test_frame"), &mesh_sprite, twoDx, twoDy);
    return d;
}
void      drawable_open_testframe_getSizesOfParent_(Node* nd) {
    Drawable* d = (Drawable*)nd;
    Node* p = nd->_parent;
    if(!p) { printerror("No parent."); return; }
//    d->n.w = 1.f;   d->n.h = 1.f;
    d->n.sx = p->w; d->n.sy = p->h;
}
void      node_tryToAddTestFrame(Node* ref) {
#ifdef DEBUG
    if(mesh_sprite.vertex_count == 0) { printerror("Missing Mesh_init()."); return; }
    if(!ref) { printerror("No parent."); return; }
    Drawable *d = coq_callocTyped(Drawable);
    node_init(&d->n, ref, 0, 0, 1, 1, flag_notToAlign, 0);
    drawable_init(d, Texture_sharedImageByName("coqlib_test_frame"), &mesh_sprite, 1, 1);
    d->n.openOpt = drawable_open_testframe_getSizesOfParent_;
    if(ref->reshapeOpt) {
        ref->flags |= flag_parentOfReshapable;
        d->n.reshapeOpt = drawable_open_testframe_getSizesOfParent_;
    }
#endif
}
void      node_last_tryToAddTestFrame(void) {
    if(!Node_last) { printerror("No last node."); return; }
    node_tryToAddTestFrame(Node_last);
}

#pragma mark - Methods ----------------------------
void      drawable_changeMeshTo(Drawable* d, Mesh* newMesh) {
    if(!d->_mesh->isShared) {
        mesh_engine_deinit(d->_mesh);
        coq_free(d->_mesh);
    }
    d->_mesh = newMesh;
}
void      drawable_changeTexToPngId(Drawable* d, uint32_t const newPngId) {
    textureref_releaseAndNull_(&d->_tex);
    d->_tex = Texture_sharedImage(newPngId);
    float const twoDy = d->n.sy * d->n.h;
    d->n.w = 1; d->n.h = 1;
    d->n.sy = twoDy;
    d->n.sx = twoDy * texture_tileRatio(d->_tex);

}
Drawable* node_asDrawableOpt(Node* const nOpt) {
    if(!nOpt) return NULL;
    if(nOpt->_type & node_type_flag_drawable) return (Drawable*)nOpt;
    return NULL;
}
void      drawableref_destroyAndNull(Drawable** const drawableOptRef) {
    Drawable* const toDelete = *drawableOptRef;
    if(toDelete == NULL) return;
    *drawableOptRef = NULL;
    if(toDelete->n.flags & (flag_toDelete_)) {
        printwarning("Already to delete.");
        return;
    }
    node_throwToGarbage_(&toDelete->n);
}

#pragma mark -- Setters -----------------------------------------------
void      drawable_setTile(Drawable *d, uint32_t i, uint32_t j) {
    Vector2 Duv = texture_tileDuDv(d->_tex);
    d->n._iu.draw_uvRect = (Rectangle) {
        .origin = {{
            (float)( i % d->_tex->m)                   * Duv.w,
            (float)((j + i / d->_tex->m) % d->_tex->n) * Duv.h,
        }},
        .size = Duv,
    };
}
void      drawable_setTileI(Drawable *d, uint32_t i) {
    uint32_t m = d->_tex->m;
    d->n._iu.draw_uvRect.o_x = (float)( i % m) * d->n._iu.draw_uvRect.w;
}
void      drawable_setTileJ(Drawable *d, uint32_t j) {
    d->n._iu.draw_uvRect.o_y = (float)( j % d->_tex->n) * d->n._iu.draw_uvRect.h;
}
void      drawable_setUVRect(Drawable* d, Rectangle const uvrect) {
    d->n._iu.draw_uvRect = uvrect;
}
void      drawable_checkRatioWithUVrectAndTexture(Drawable *const d, float const newTwoDyOpt) {
    if(newTwoDyOpt) d->n.sy = newTwoDyOpt;
    d->n.sx = d->n.sy * d->_tex->width / d->_tex->height * d->n._iu.draw_uvRect.w / d->n._iu.draw_uvRect.h;
}
void      drawable_last_setTile(uint32_t i, uint32_t j) {
    if(drawable_last_ == NULL) {
        printwarning("No last drawable.");
        return;
    }
    uint32_t m = drawable_last_->_tex->m;
    uint32_t n = drawable_last_->_tex->n;
    drawable_last_->n._iu.draw_uvRect.origin = (Vector2) {{
        (float)( i % m)          * drawable_last_->n._iu.draw_uvRect.w,
        (float)((j + i / m) % n) * drawable_last_->n._iu.draw_uvRect.h
    }};
}
void      drawable_last_setExtra1(float emph) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    drawable_last_->n._iu.extra1 = emph;
}
void      drawable_last_setShowOptions(bool isHard, bool isPoping) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    smtrans_setOptions(&drawable_last_->trShow, isHard, isPoping);
}
void      drawable_last_setColor(Vector4 color) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    drawable_last_->n._iu.draw_color = color;
}


// GARBAGE
//Drawable* Drawable_createString(Node* const refOpt, StringDrawable const str,
//                   float x, float y, float maxTwoDxOpt, float twoDy,
//                   flag_t flags, uint8_t node_place) {
//    if(mesh_sprite.vertex_count == 0) printerror("Missing Mesh_init().");
//    Drawable* d = coq_callocTyped(Drawable);
//    node_init(&d->n, refOpt, x, y, twoDy, twoDy, node_type_n_drawable, flags, node_place);
//    drawable_init_(d, Texture_retainString(str), &mesh_sprite, maxTwoDxOpt, twoDy, str.x_margin);
//    return d;
//}
//void     drawable_updateAsMutableString(Drawable* d, const char* new_c_str, bool forceRedraw) {
//    texture_updateMutableString(d->_tex, new_c_str, forceRedraw);
//    drawable_string_setDims_(d);
//}
//void     drawable_updateAsSharedString(Drawable* d, StringDrawable const str) {
//    textureref_exchangeSharedStringFor(&d->_tex, str);
//    drawable_string_setDims_(d);
//}
//void     drawable_string_setDims_(Drawable* const d) {
//    if(!(d->_tex->flags & tex_flag_string)) { printerror("Not a string drawable."); return; }
//    // 1. Delta Y : On a h * sy = 2*Dy.
//    float const beta = d->_tex->beta;
//    d->n.sy = d->_twoDyTarget / beta;
//    d->n.h  = beta;
//
//    // 2. Delta X... Ici, il faut faire un dessin...
//    float const alpha = d->_tex->alpha;
//    // Largeur a priori (prise du ratio de la texture).
//    float sx = d->n.sy * d->_tex->ratio;
//    // Si on fixe la largeur (largeur custom avec marges)
//    if(d->_twoDxTarget > 0.f) {
//        float sxCustom = fmaxf((d->_twoDxTarget) / alpha, 0.f);
//        if((sxCustom > 0.01f*d->_twoDyTarget) && (sxCustom < sx))
//            sx = sxCustom; // (Fixer pour plafonner à la largeur a priori.)
//    }
//    d->n.sx = sx;
//    d->n.w = fmaxf(alpha, 0.01f);
//    // (car 2dx = w*sx, sx = (2dx - e)/alpha... voir dessin.)
//
//    // 3. Ajuster le bigbro frame (si besoin)
//    Node* bigbro = d->n._bigBro;
//    if((d->n.flags & flag_giveSizeToBigbroFrame) && bigbro)
//        node_tryUpdatingAsFrameOfBro(bigbro, &d->n);
//    // 4. Donner les dimensions au parent (si besoin)
//    Node* parent = d->n._parent;
//    if((d->n.flags & flag_giveSizeToParent) && parent) {
//        parent->w = node_deltaX(&d->n) * 2.f; // A été ajusté...
//        parent->h = d->_twoDyTarget;           // (Constant)
//        if(parent->reshapeOpt)
//            parent->reshapeOpt(parent);
//    }
//}
//void     drawable_image_setDims_(Drawable* const d) {
//    if(!(d->_tex->flags & tex_flag_png)) { printerror("Not a png image."); return; }
//    d->n.sy = d->_twoDyTarget;
//    d->n.w = 1; d->n.h = 1;
//    if(d->_twoDxTarget)
//        d->n.sx = d->_twoDxTarget;
//    else {
//        d->n.sx = d->_tex->ratio * d->n._iu.draw_uvRect.w / d->n._iu.draw_draw_uvRect.h * d->_twoDyTarget;
//   }
//}
//void      drawable_updateTargetDims(Drawable* d, float newTwoDxOpt, float newTwoDy, float newXMargin) {
//    d->_twoDxTarget = newTwoDxOpt;
//    d->_twoDyTarget = newTwoDy;
//    d->_xMargin = newXMargin;
//    if(d->_tex->flags & tex_flag_png)
//        drawable_image_setDims_(d);
//    else if(d->_tex->flags & tex_flag_string)
//        drawable_string_setDims_(d);
//}
//Drawable* Drawable_createFan(Node* const refOpt, uint32_t pngId,
//                             float x, float y, float twoDy,
//                             flag_t flags, uint8_t node_place) {
//    if(mesh_sprite.vertex_count == 0)
//        printerror("Missing Mesh_init().");
//    Drawable* d = Drawable_create(refOpt, Texture_sharedImage(pngId), Mesh_createFan(),
//                                  flags, node_place);
//    d->n.x = x;        d->n.y = y;
//    d->n.w = 1.f;      d->n.h = 1.f;
//    d->n.sx = twoDy;   d->n.sy = twoDy;
//    return d;
//}
