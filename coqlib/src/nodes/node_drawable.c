//
//  node_surface.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include "node_drawable.h"

#include "../utils/util_base.h"
#include "../systems/system_language.h"
#include <stdlib.h>

// MARK: - ChronoTiny : un mini chrono de 2 octets -
/// Mini chrono de 2 octets, basé sur ChronoRendering.
/// Lapse max de 2^16 ms, i.e. de -32s à +32s.
/// La sruct est simplement {int16_t time}.
/// Optenir un ChronoTiny que l'on "start" a zero.
static inline int16_t chronotiny_startedChrono(void) {
    return (int16_t)ChronosRender.render_elapsedMS;
}
/// Optenir un ChronoTiny que l'on "start" a elapsedMS.
static inline int16_t chronotiny_setToElapsedMS(int16_t elapsedMS) {
    return (int16_t)ChronosRender.render_elapsedMS - elapsedMS;
}
// Temps écoulé... oui, c'est la meme chose que setToElapsedMS ;)
static inline int16_t chronotiny_elapsedMS(int16_t t) {
    return (int16_t)ChronosRender.render_elapsedMS - t;
}

// MARK: - SmoothFlag
enum {
    sm_state_isDown_ =       0,
    sm_state_goingUp_ =      1,
    sm_state_goingDown_ =    2,
    sm_state_goingUpOrDown_ = 3,
    sm_state_isUp_ =         4,
    sm_state_flags_ =        7,
    sm_flag_poping_ =        8,
    sm_flag_hard_ =         16,
};
#define ST_MIN_MS   6  // Minimum elapsed time
static uint16_t     sm_defTransTime_ = 500;
static float        sm_a_ =     0.75 + (0.2) * 0.2;  // (pop factor est de 0.2 par défaut)
static float        sm_b_ =    -0.43 + (0.2) * 0.43;

SmoothFlag SmoothFlag_new(void) {
    return (SmoothFlag) {
        ._D = sm_defTransTime_,
    };
}
void    SmoothFlag_setPopFactor(float popFactor) {
    sm_a_ =  0.75f + popFactor * 0.20f;
    sm_b_ = -0.43f + popFactor * 0.43f;
}
void    SmoothFlag_setDefaultTransTime(uint16_t transTime) {
    sm_defTransTime_ = transTime;
}


float smoothflag_setOn(SmoothFlag *const st) {
    // On verifie les cas du plus probable au moins probable.
    // Callé à chaque frame pour vérifier le on/off de "show".
    float const max = 1.f - (float)st->_sub/(float)UINT16_MAX;
    if(st->_flags & sm_state_isUp_)
        return max;
    int16_t elapsed = chronotiny_elapsedMS(st->_t);
    // Going up
    if(st->_flags & sm_state_goingUp_) {
        if(elapsed > st->_D) {
            st->_flags &= ~sm_state_goingUp_;
            st->_flags |= sm_state_isUp_;
            return max;
        }
    }
    // down -> goingUp ou up 
    else if((st->_flags & sm_state_flags_) == sm_state_isDown_) {
        if(st->_flags & sm_flag_hard_) {
            st->_flags |= sm_state_isUp_;
            return max;
        }
        st->_flags |= sm_state_goingUp_;
        st->_t = chronotiny_startedChrono();
        elapsed = ST_MIN_MS; // (Min 6ms de temps écoulé)
    }
    // going down -> goingUp 
    else {
        st->_flags &= ~sm_state_goingDown_;
        st->_flags |= sm_state_goingUp_;
        if(elapsed < st->_D) {
            elapsed = st->_D - elapsed;
            st->_t = chronotiny_setToElapsedMS(elapsed);
        } else {
            st->_t = chronotiny_startedChrono();
            elapsed = ST_MIN_MS;
        }
    }
    // Ici on est going up... (si up déjà sorti)
    float ratio = (float)elapsed / (float)st->_D;
    if(st->_flags & sm_flag_poping_)
        return max * (sm_a_ + sm_b_ * cosf(M_PI * ratio)
                  + ( 0.5f - sm_a_) * cosf(2.f * M_PI * ratio)
                  + (-0.5f - sm_b_) * cosf(3.f * M_PI * ratio)); // pippop
    return max * (1.f - cosf(M_PI * ratio)) / 2.f; // smooth
}
float smoothflag_setOff(SmoothFlag *const st) {
    if((st->_flags & sm_state_flags_) == sm_state_isDown_)
        return 0.0f;
    int16_t elapsed = chronotiny_elapsedMS(st->_t);
    if(st->_flags & sm_state_goingDown_) {
        if(elapsed > st->_D) {
            st->_flags &= ~sm_state_flags_; // (down)
            return 0.0f;
        }
    }
    // Up 
    else if(st->_flags & sm_state_isUp_) {
        st->_flags &= ~sm_state_flags_; // (down)
        if(st->_flags & sm_flag_hard_)
            return 0.0f;
        st->_flags |= sm_state_goingDown_;
        st->_t = chronotiny_startedChrono();
        elapsed = ST_MIN_MS;
    }
    // Going up
    else {
        st->_flags &= ~sm_state_goingUp_;
        st->_flags |= sm_state_goingDown_;
        if(elapsed < st->_D) {
            elapsed = st->_D - elapsed;
            st->_t = chronotiny_setToElapsedMS(elapsed);
        } else {
            st->_t = chronotiny_startedChrono();
            elapsed = ST_MIN_MS;
        }
    }
    // Ici on est going down... (si down déjà sorti)
    float ratio = (float)elapsed / (float)st->_D;
    float const max = 1.f - (float)st->_sub/(float)UINT16_MAX;
    return  max * (1.f + cosf(M_PI * ratio)) / 2.f; // (smoothDown)
}
float smoothflag_justGetValue(SmoothFlag *const st) {
    if((st->_flags & sm_state_flags_) == sm_state_isDown_)
        return 0.0f;
    float const max = 1.f - (float)st->_sub/(float)UINT16_MAX;
    if(st->_flags & sm_state_isUp_)
        return max;
    int16_t elapsed = chronotiny_elapsedMS(st->_t);
    // On est goingUpOrDown...
    if(elapsed < st->_D) {
        float ratio = (float)elapsed / (float)st->_D;
        if(st->_flags & sm_state_goingUp_) {
            if(st->_flags & sm_flag_poping_)
                return max * (sm_a_ + sm_b_ * cosf(M_PI * ratio)
                    + ( 0.5f - sm_a_) * cosf(2.f * M_PI * ratio)
                    + (-0.5f - sm_b_) * cosf(3.f * M_PI * ratio)); // pippop
            return max * (1.f - cosf(M_PI * ratio)) / 2.f; // smoothUp
        }
        // (going down)
        return  max * (1.f + cosf(M_PI * ratio)) / 2.f; // (smoothDown)
    }
    // Transition fini : up
    if(st->_flags & sm_state_goingUp_) {
        st->_flags &= ~sm_state_goingUp_;
        st->_flags |= sm_state_isUp_;
        return max;
    }
    // Down
    st->_flags &= ~sm_state_flags_;
    return 0.f;
}

void  smoothflag_setMaxValue(SmoothFlag *st, float newMax) {
    st->_sub = UINT16_MAX - (uint16_t)roundf(fminf(1.f, fmaxf(0.f, newMax))*(float)UINT16_MAX);
}
void  smoothflag_setOptions(SmoothFlag *st, bool isHard, bool isPoping) {
    if(isHard)
        st->_flags |= sm_flag_hard_;
    else
        st->_flags &= ~sm_flag_hard_;
    if(isPoping)
        st->_flags |=  sm_flag_poping_;
    else
        st->_flags &= ~sm_flag_poping_;
}
void  smoothflag_setDeltaT(SmoothFlag *st, int16_t deltaT) {
    st->_D = deltaT;
}




static Drawable* drawable_last_ = NULL;
/// Mise à jour ordinaire de la matrice modèle pour un drawable.
void drawable_renderer_updateIU_(Node* const n) {
    Drawable* d = (Drawable*)n;
    float show = drawable_updateShow(d);
    if(!show) return;
    const Matrix4* const pm = node_parentModel(n);
    if((d->n.flags & flag_drawablePoping) == 0)
        show = 1.f;
    Matrix4* m = &n->renderIU.model;
    Vector3 const pos = n->xyz;
    Vector3 const scl = n->scales;
    // Equivalent de :
//    *m = *pm;
//    matrix4_translate(m, pos);
//    matrix4_scale(m, scl);
    m->v0.v = pm->v0.v * scl.x * show;
    m->v1.v = pm->v1.v * scl.y * show;
    m->v2.v = pm->v2.v * scl.z * show;
    m->v3.v = pm->v3.v + pm->v0.v * pos.x + pm->v1.v * pos.y + pm->v2.v * pos.z;
}
void (*Drawable_renderer_defaultUpdateInstanceUniforms)(Node*) = drawable_renderer_updateIU_;
Vector4 Drawable_renderIU_defaultColor = {{ 1, 1, 1, 1 }};

// MARK: -- Init/constructors ---------------------------------------
void      drawable_deinit_(Node* nd){
    Drawable* d = (Drawable*)nd;
    meshref_releaseAndNull(&d->_mesh);
    textureref_releaseAndNull(&d->texr);
}
void      drawable_init(Drawable* const d, Texture* tex, Mesh* mesh,
                        float const twoDxOpt, float const twoDy) {
    if(tex == NULL) { printerror("No texture."); tex = Texture_white; }
    if(mesh == NULL) { printwarning("No mesh."); mesh = Mesh_drawable_sprite; }
    // Init
    d->trShow = SmoothFlag_new();
    d->trExtra = SmoothFlag_new();
    textureref_init(&d->texr, tex);
    d->_mesh = mesh;
    d->n._type |= node_type_drawable;
    // Override de l'update de la matrice model.
    d->n.renderer_updateInstanceUniforms = Drawable_renderer_defaultUpdateInstanceUniforms;
    d->n.deinitOpt = drawable_deinit_;
    d->n.renderIU.color =  Drawable_renderIU_defaultColor;
    d->n.renderIU.uvRect.size = d->texr.dims.DuDv;
    drawable_last_ = d;
    // Set des dimensions...
    d->n.w = 1; d->n.h = 1;
    d->n.sy = twoDy;
    if(twoDxOpt)
        d->n.sx = twoDxOpt;
    else {
        d->n.sx = twoDy * d->texr.dims.tileRatio;
   }
}
Drawable* Drawable_createImage(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDy, flag_t flags) {
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    drawable_init(d, Texture_sharedImage(pngId), Mesh_drawable_sprite, 0, twoDy);
    return d;
}
Drawable* Drawable_createImageWithWidth(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDx, float twoDy, flag_t flags) {
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    drawable_init(d, Texture_sharedImage(pngId), Mesh_drawable_sprite, twoDx, twoDy);
    return d;
}
Drawable* Drawable_createImageWithName(Node* const refOpt, const char* pngName,
                          float x, float y, float twoDy, flag_t flags) {
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    drawable_init(d, Texture_sharedImageByName(pngName), Mesh_drawable_sprite, 0, twoDy);
    return d;
}
void      drawable_open_imagelanguage_(Node* nd) {
    Drawable* d = (Drawable*)nd;
    drawable_setTile(d, Language_current(), 0);
}
Drawable* Drawable_createImageLanguage(Node* refOpt, uint32_t pngId,
                                       float x, float y, float twoDy, flag_t flags) {
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    drawable_init(d, Texture_sharedImage(pngId), Mesh_drawable_sprite, 0, twoDy);
    d->n.openOpt = drawable_open_imagelanguage_;
    return d;
}
Drawable* Drawable_createColor(Node* refOpt, Vector4 color,
                               float x, float y, float twoDx, float twoDy) {
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, 0, 0);
    drawable_init(d, Texture_white, Mesh_drawable_sprite, twoDx, twoDy);
    d->n.renderIU.color = color;
    return d;
}
Drawable* Drawable_createTestFrame(Node* parent, float x, float y, float twoDx, float twoDy) {
    Drawable *d = coq_callocTyped(Drawable);
    node_init(&d->n, parent, x, y, 1, 1, 0, 0);
    drawable_init(d, Texture_sharedImageByName("coqlib_test_frame"), Mesh_drawable_sprite, twoDx, twoDy);
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
    if(!ref) { printerror("No parent."); return; }
    Drawable *d = coq_callocTyped(Drawable);
    node_init(&d->n, ref, 0, 0, 1, 1, flag_notToAlign, 0);
    drawable_init(d, Texture_sharedImageByName("coqlib_test_frame"), Mesh_drawable_sprite, 1, 1);
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

// MARK: - Methods ----------------------------
void      drawable_changeMeshTo(Drawable*const d, Mesh*const newMesh) {
    meshref_releaseAndNull(&d->_mesh);
    d->_mesh = newMesh;
}
float     drawable_updateShow(Drawable *const d) {
    float const show = (d->n.flags & flag_show) ? smoothflag_setOn(&d->trShow) : smoothflag_setOff(&d->trShow);
    d->n.renderIU.show = show;
    if(show < 0.001f) {
        d->n.renderIU.flags &= ~renderflag_toDraw;
        return 0.f;
    }
    d->n.renderIU.flags |= renderflag_toDraw;
    return show;
}

void      drawable_changeTexToPngId(Drawable* d, uint32_t const newPngId) {
    textureref_releaseAndNull(&d->texr);
    textureref_init(&d->texr, Texture_sharedImage(newPngId));
    d->n.renderIU.uvRect.size = d->texr.dims.DuDv; 
    float const twoDy = d->n.sy * d->n.h;
    d->n.w = 1; d->n.h = 1;
    d->n.sy = twoDy;
    d->n.sx = twoDy * d->texr.dims.tileRatio;

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

// MARK: -- Setters -----------------------------------------------
void      drawable_checkRatioWithUVrectAndTexture(Drawable *const d, float const newTwoDyOpt) {
    if(newTwoDyOpt) d->n.sy = newTwoDyOpt;
    d->n.sx = d->n.sy * (float)d->texr.dims.width / (float)d->texr.dims.height * d->n.renderIU.uvRect.w / d->n.renderIU.uvRect.h;
}
void      drawable_last_setTile(uint32_t const i, uint32_t const j) {
    if(drawable_last_ == NULL) {
        printwarning("No last drawable.");
        return;
    }
    drawable_setTile(drawable_last_, i, j);
}
void      drawable_last_setExtra1(float emph) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    drawable_last_->n.renderIU.extra1 = emph;
}
void      drawable_last_setShowOptions(bool isHard, bool isPoping) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    smoothflag_setOptions(&drawable_last_->trShow, isHard, isPoping);
}
void      drawable_last_setColor(Vector4 color) {
    if(drawable_last_ == NULL) { printwarning("No last drawable."); return; }
    drawable_last_->n.renderIU.color = color;
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
//        d->n.sx = d->_tex->ratio * d->n.renderIU.uvRect.w / d->n._iu.draw_uvRect.h * d->_twoDyTarget;
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
