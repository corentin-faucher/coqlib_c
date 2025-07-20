//
//  pop_disk.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-28.
//

#include "node_poping.h"

#include "node_tree.h"
#include "node_root.h"
#include "node_drawable_multi.h"
#include "../systems/system_sound.h"

typedef struct PopingNode_ {
    union {
        Node   n;
        Fluid  f;
    };
    PopingNode_** refererOpt;
    Countdown     countdown;
    void        (*callbackOpt)(Fluid*, Countdown*);
} PopingNode_;
PopingNode_* node_asPopingNodeOpt_(Node* n) {
    if(!n) return NULL;
    if(n->_type & node_type_poping)
        return (PopingNode_*)n;
    return NULL;
}

// MARK: - Poping Base
static View*    Poping_frontView_ = NULL;
static Timer    Poping_timer_ = NULL;
Fluid* popingnode_last_notSharedOpt_ = NULL;

void PopingNode_setFrontView(View* frontView) {
    Poping_frontView_ = frontView;
}

void PopingNode_checkAll_callback_(void* unused) {
    if(!Poping_frontView_) goto terminate_callbacks;
    if(Poping_frontView_->n.flags & flag_toDelete_) {
        printwarning("Front view deleted and not updated in PopingNode.");
        Poping_frontView_ = NULL;
        goto terminate_callbacks;
    }
    bool stillPopingActive = false;
    Node* child = Poping_frontView_->n._firstChild;
    while(child) {
        Node* toDelete = NULL;
        PopingNode_* const pn = node_asPopingNodeOpt_(child);
        if(!pn) goto go_next;
        stillPopingActive = true;
        if(pn->callbackOpt && (pn->n.flags & flag_show)) pn->callbackOpt(&pn->f, &pn->countdown);
        if(!countdown_isRinging(&pn->countdown)) goto go_next;
        if(pn->n.flags & flag_show)
            node_tree_close(child);
        else // Déjà fermer -> effacer.
            toDelete = child;
    go_next:
        child = child->_littleBro;
        if(toDelete) {
            node_throwToGarbage_(toDelete);
        }
    }
    if(stillPopingActive) return;
terminate_callbacks:
    timer_cancel(&Poping_timer_);
}

void PopingNode_newAdded_(void) {
    if(!Poping_frontView_) { printerror("No front view."); return; }
    if(Poping_timer_) return;
    timer_scheduled(&Poping_timer_, 1, true, NULL, PopingNode_checkAll_callback_);
}

// MARK: - Instance PopingNode

void popingnode_close_(Node* n) {
    PopingNode_* pn = (PopingNode_*)n;
    pn->countdown.ringTimeMS = 500;
    if(n == &popingnode_last_notSharedOpt_->n)
        popingnode_last_notSharedOpt_ = NULL;
    countdown_start(&pn->countdown);
    // Déréférencer
    if(pn->refererOpt) *pn->refererOpt = NULL;
}
void PopingNode_spawn(PopingNode_ **const refererOpt, float x, float y, float width, float height,
                      float timeSec, PopingInfo const popInfo, void (*callBackOpt)(Fluid*,Countdown*)) {
    popingnode_last_notSharedOpt_ = NULL;
    if(Poping_frontView_ == NULL) { printerror("PopingNode not init."); return; }
    PopingNode_ *pn = coq_callocTyped(PopingNode_);
    // Super inits
    node_init(&pn->n, &Poping_frontView_->n, x, y, width, height, 0, 0);
    fluid_init(&pn->f, 0.f);
    fluid_popIn(&pn->f, popInfo);
    // Init as poping
    pn->n._type |= node_type_poping;
    pn->n.closeOpt = popingnode_close_;
    pn->countdown.ringTimeMS = (int64_t)(timeSec * 1000.f);
    countdown_start(&pn->countdown);
    pn->callbackOpt = callBackOpt;
    pn->refererOpt = refererOpt;
    if(pn->refererOpt) *pn->refererOpt = pn; // i.e. on a aussi *refererOpt = pn.

    PopingNode_newAdded_();
    popingnode_last_notSharedOpt_ = &pn->f;
}
void popingnode_last_open(void) {
    node_tree_openAndShow(&popingnode_last_notSharedOpt_->n);
}
void popingnoderef_cancel(PopingNode_** const popingref) {
    if(!popingref) {
        return;
    }
    PopingNode_* const poping = *popingref;
    if(poping == NULL) {
        return;
    }
    if(popingref != poping->refererOpt) {
        printerror("timerRef is not the timer referer.");
        return;
    }
    *popingref = NULL;  // (déréférencer)
    if(poping->n.flags & flag_show)  // Mettre le countdown à zéro -> PopingNode sera fermé puis effacé...
        poping->countdown.ringTimeMS = 0;
}

void PopingNode_spawnOver(Node* const nodeOver, PopingNode_** refererOpt, float width_rel, float height_rel,
                                float timeSec, PopingInfo const popInfo, void (*callBackOpt)(Fluid*,Countdown*)) {
    Box parent_box = node_hitboxInParent(nodeOver, NULL);
    float height = height_rel * 2*parent_box.Dy;
    float width =  width_rel  * 2*parent_box.Dy; // (La référence reste la hauteur du parent.)
    float x = parent_box.c_x;
    float y = parent_box.c_y;

    PopingNode_spawn(refererOpt, x, y, width, height, timeSec, popInfo, callBackOpt);
}

void popingnode_last_checkForScreenSpilling(void) {
    guard_let(Fluid*, f, popingnode_last_notSharedOpt_, , )
    Node* const n = &f->n;
    Box const n_b0 = node_hitbox(n); // (box dans parent)
    Box n_b = n_b0;
    // Trouver la root.
    Root* root = NULL;
    for(Node const* p = n->_parent; p; p = p->_parent) {
        root = node_asRootOpt(p);
        // Déjà au niveau de la root ? Hitbox ok -> sort.
        if(root) break;
        // Sortir la hitbox du parent et aller au parent suivant...
        n_b = box_referentialOut(n_b, node_referential(p)); 
    }
    if(!root) { printerror("No root."); return; }
    float dx = 0;
    float dy = 0;
    // Débordement à droite...
    if(n_b.c_x + n_b.Dx > 0.5*root->n.w) {
        dx = ( 0.5*root->n.w - (n_b.c_x + n_b.Dx)) * (n_b0.Dx/n_b.Dx); // (négatif)
    }
    // Débordement à gauche...
    if(n_b.c_x - n_b.Dx < -0.5*root->n.w) {
        dx = (-0.5*root->n.w - (n_b.c_x - n_b.Dx)) * (n_b0.Dx/n_b.Dx); // (positif)
    }
    // Débordement en haut
    if(n_b.c_y + n_b.Dy > 0.5*root->n.h) {
        dy = ( 0.5*root->n.h - (n_b.c_y + n_b.Dy)) * (n_b0.Dy/n_b.Dy); // (négatif)
    }
    // Débordement en bas
    if(n_b.c_y - n_b.Dy < -0.5*root->n.h) {
        dy = (-0.5*root->n.h - (n_b.c_y - n_b.Dy)) * (n_b0.Dy/n_b.Dy); // (positif)
    }
    // Replacer x, y dans l'écran.
    if(dx || dy)
        fluid_setXY(f, (Vector2){{n->x + dx, n->y + dy}});
}

// MARK: - PopDisk, exemple de PopingNode : disk de progres qui s'autodetruit -
void popdisk_updateCallback_(Fluid* f, Countdown* cd) {
    float elapsedSec = chrono_elapsedSec(&cd->c);
    float deltaT = (float)cd->ringTimeMS * 0.001f;
    Drawable* const fan = node_asDrawableOpt(f->n._firstChild);
    if(!fan) { printerror("No drawable in popdisk."); return; }
    if(elapsedSec < deltaT + 0.10f && mesh_isReadyToEdit(fan->_mesh)) {
        float ratio = fminf(elapsedSec / deltaT, 1.f);
        mesh_fan_update(fan->_mesh, ratio);
        return;
    }
}
void   PopDisk_spawnOverAndOpen(Node *const nodeOverOpt, PopingNode_ **const refererOpt,
                   uint32_t const pngId, uint32_t const tile, float const deltaT,
                   float x, float y, float twoDyRel) {
    // Ajuster la positon relativement au noeud de reférence.
    if(nodeOverOpt) {
        Box parent_box = node_hitboxInParent(nodeOverOpt, NULL);
        twoDyRel = twoDyRel * 2*parent_box.Dy;  // (La référence reste la hauteur du parent.)
        x += parent_box.c_x;
        y += parent_box.c_y;
    }
    PopingNode_spawn(refererOpt, x, y, twoDyRel, twoDyRel, deltaT, popinginfo_default, popdisk_updateCallback_);
    Fluid* const f = popingnode_last_notSharedOpt_;
    if(!f) return;
    // Structure
    Drawable* const fan = coq_callocTyped(Drawable);
    node_init(&fan->n, &f->n, 0, 0, twoDyRel, twoDyRel, 0, 0);
    drawable_init(fan, Texture_sharedImage(pngId), Mesh_createFan(), 0, twoDyRel);
    // Open
    popingnode_last_open();
}

// MARK: - Sparkles
/*-- Feux d'artifices --------------------------------------------*/
#define SPARKLES_N_ 12
// MARK: - DrawableMulti de la Sparkles ------------

typedef struct DrawableMultiSparkles_ {
    union {
        Node          n;
        Drawable      d;
        DrawableMulti dm;
    };
    int64_t             t0;
    Vector2             p0s[SPARKLES_N_];
    Vector2             p1s[SPARKLES_N_];
} DrawableMultiSparkles_;
// Override...
void drawablemultiSparkle_updateIUs_(Node* const n) {
    DrawableMultiSparkles_* dmsp = (DrawableMultiSparkles_*)n;
    float const show = drawable_updateShow(&dmsp->d);
    if(!show) return;
    const Matrix4* const pm = node_parentModel(n);
    float deltaT = (float)(ChronoApp_elapsedMS() - dmsp->t0)*0.001f;
    float alpha = float_smoothOut(deltaT, 5.f);
    Vector2 const scl = {{ dmsp->n.sx * show, dmsp->n.sy * show }};
    withIUsToEdit_beg(iusEdit, &dmsp->dm.iusBuffer)
    const Vector2  *p0 = dmsp->p0s, *p1 = dmsp->p1s;
    for(; iusEdit.iu < iusEdit.end; iusEdit.iu++, p0++, p1++) {
        iusEdit.iu->show = show;
        Matrix4* m = &iusEdit.iu->model;
        // Petite translation sur la parent-matrix en fonction de la particule.
        Vector2 const pos = vector2_add(vector2_times(*p0, alpha), vector2_times(*p1, 1.f - alpha));
        m->v0.v = pm->v0.v * scl.x;
        m->v1.v = pm->v1.v * scl.y;
        m->v2 =   pm->v2;
        m->v3 = (Vector4) {{
            pm->v3.x + pm->v0.x * pos.x + pm->v1.x * pos.y,
            pm->v3.y + pm->v0.y * pos.x + pm->v1.y * pos.y,
            pm->v3.z + pm->v0.z * pos.x + pm->v1.z * pos.y,
            pm->v3.w,
        }};
    }
    withIUsToEdit_end(iusEdit)
}
DrawableMultiSparkles_* DrawableMultiSparkles_create_(Node* parent, float const height, Texture* tex) {
    DrawableMultiSparkles_* dmsp = coq_callocTyped(DrawableMultiSparkles_);
    node_init(&dmsp->n, parent, 0, 0, height, height, 0, 0);
    drawable_init(&dmsp->d, tex, Mesh_drawable_sprite, 0, height);
    drawablemulti_init(&dmsp->dm, SPARKLES_N_, NULL);
    dmsp->n.renderer_updateInstanceUniforms = drawablemultiSparkle_updateIUs_;
    dmsp->t0 = ChronoApp_elapsedMS();
    
    // Init des per instance uniforms
    Vector2* p0 = dmsp->p0s;
    Vector2* p1 = dmsp->p1s;
    TextureDims const td = dmsp->d.texr.dims;
    withIUsToEdit_beg(iusEdit, &dmsp->dm.iusBuffer)
    for(; iusEdit.iu < iusEdit.end; iusEdit.iu++, p0++, p1++) {
        iusEdit.iu->color = Drawable_renderIU_defaultColor;
        iusEdit.iu->uvRect = texturedims_uvRectOfTile(td, rand(), 0);
        *p0 = (Vector2) {{ rand_floatAt(0, 0.25f*height), rand_floatAt(0, 0.25f*height) }};
        *p1 = (Vector2) {{ rand_floatAt(0, 3.00f*height), rand_floatAt(0, 3.00f*height) }};
    }
    iusEdit.init = true;
    withIUsToEdit_end(iusEdit)
    return dmsp;
}


static Texture* Sparkle_tex_ = NULL;
static uint32_t Sparkle_soundId_ = 0;
void Sparkle_init(Texture* sparkleTex, uint32_t sparkleSoundId) {
    Sparkle_tex_ = sparkleTex;
    Sparkle_soundId_ = sparkleSoundId;
}

void Sparkle_spawnAtAndOpen(float xabs, float yabs, float delta, Texture* texOpt) {
    if(!texOpt && !Sparkle_tex_) { printerror("No texture for sparkles."); return; }
    PopingInfo info = {
        {{ rand_floatAt(0, 0.25*delta), rand_floatAt(0, 0.25*delta), 0, 0 }},
        {{ rand_floatAt(0, 0.50*delta), rand_floatAt(0, 0.50*delta), 0, 0 }},
        10, 25, 0, 0,
    };
    PopingNode_spawn(NULL, xabs, yabs, delta, delta, 0.6, info, NULL);
    Fluid* const f = popingnode_last_notSharedOpt_;
    if(!f) return;
    Texture* tex = texOpt ? texOpt : Sparkle_tex_;
    DrawableMultiSparkles_create_(&f->n, delta, tex);
    Sound_play(Sparkle_soundId_, 1.f, 0, 1.f);
    node_tree_openAndShow(&f->n);
}
// Convenience constructor.
void Sparkle_spawnOverAndOpen(Node* nd, float deltaRatio) {
    Box box = node_hitboxInParent(nd, NULL);
    float delta = deltaRatio * box.Dy;
    Sparkle_spawnAtAndOpen(box.c_x, box.c_y, delta, NULL);
}

// MARK: - PopMessage
/*-- PopMessage : Message temporaire. ----------------------------------*/
void PopMessage_spawnAtAndOpen(float xabs, float yabs, float twoDxOpt, float twoDy,
                        float timeSec, uint32_t framePngId,
                        StringGlyphedInit str, FramedStringParams params)
{
    PopingNode_spawn(NULL, xabs, yabs, twoDxOpt, twoDy, timeSec, popinginfo_default, NULL);
    Fluid* const f = popingnode_last_notSharedOpt_;
    if(!f) return;
    // Structure
    node_addFramedString(&f->n, framePngId, str, params);
    popingnode_last_checkForScreenSpilling();
    node_tree_openAndShow(&f->n);
}

void PopMessage_spawnOverAndOpen(Node* n, float widthOpt_rel, float height_rel,
                            float timeSec, uint32_t framePngId,
                            StringGlyphedInit str, FramedStringParams params)
{
    PopingNode_spawnOver(n, NULL, widthOpt_rel, height_rel, timeSec, popinginfo_default, NULL);
    Fluid* const f = popingnode_last_notSharedOpt_;
    if(!f) return;
    f->n.w = f->n.h * 10;
    // Structure
    node_addFramedString(&f->n, framePngId, str, params);
    popingnode_last_checkForScreenSpilling();
    node_tree_openAndShow(&f->n);
}
