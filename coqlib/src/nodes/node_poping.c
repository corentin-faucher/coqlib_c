//
//  pop_disk.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-28.
//

#include "node_poping.h"

#include "node_squirrel.h"
#include "node_tree.h"
#include "node_root.h"
#include "node_drawable_multi.h"
#include "../coq_sound.h"

#pragma mark - Poping Base
static View*    Poping_frontView_ = NULL;
static Texture* Poping_tex_ = NULL;
static uint32_t Poping_soundId_ = 0;

static const PopingInfo Poping_defaultInfo_ = {
    {{ 0.0, 0.0, -0.4, -0.2 }},
    {{ 0.0, 1.0,  0.0,  0.0 }},
    10, 25, 10, 25,
};


void popingnode_close_(Node* n) {
    PopingNode* pn = (PopingNode*)n;
    timer_cancel(&pn->timer);
    timer_scheduled(&pn->timer, 500, false, n, node_tree_throwToGarbage);
}
void PopingNode_init(View* frontView, const char* pngNameOpt, uint32_t soundId) {
    Poping_frontView_ = frontView;
    Poping_tex_ = Texture_sharedImageByName(pngNameOpt ? pngNameOpt : "coqlib_sparkle_stars");
    Poping_soundId_ = soundId;
}
void PopingNode_setTexture(uint32_t pngId) {
    Poping_tex_ = Texture_sharedImage(pngId);
}
PopingNode* PopingNode_spawn(Node* refOpt, float x, float y, float width, float height, size_t structSizeOpt,
                                  float timeSec, const PopingInfo* popInfoOpt) {
    if(Poping_frontView_ == NULL) { printerror("PopingNode not init."); }
    Node* parent = refOpt ? refOpt : (Node*)Poping_frontView_;
    size_t size = (structSizeOpt > sizeof(PopingNode)) ? structSizeOpt : sizeof(PopingNode);
    const PopingInfo* info = popInfoOpt ? popInfoOpt : &Poping_defaultInfo_;

    PopingNode *pn = coq_calloc(1, size);
    // Init as node
    node_init_(&pn->n, parent, x, y, width, height, node_type_n_fluid, 0, 0);
    pn->n.closeOpt = popingnode_close_;
    // Init as Fluid
    fluid_init_(&pn->f, 0.f);
    fluid_popIn(&pn->f, *info);
    // Close callback
    timer_scheduled(&pn->timer, (int64_t)(timeSec*1000.f), false, &pn->n, node_tree_close);

    return pn;
}
PopingNode* PopingNode_spawnOver(Node* const nodeOver, float width_rel, float height_rel, size_t structSizeOpt,
                                float timeSec, const PopingInfo* popInfoOpt, bool inFrontScreen) {
    float x = 0;
    float y = 0;
    float width, height;
    if(inFrontScreen) {
        Box parent_box = node_hitBoxInParentReferential(nodeOver, NULL);
        height = height_rel * 2*parent_box.Dy;
        width =  width_rel  * 2*parent_box.Dy; // (La référence reste la hauteur du parent.)
        x = parent_box.c_x;
        y = parent_box.c_y;
    } else {
        height = height_rel * nodeOver->h;
        width =  width_rel * nodeOver->h;
    }

    return PopingNode_spawn(inFrontScreen ? NULL : nodeOver, x, y, width, height, structSizeOpt, timeSec, popInfoOpt);
}

void popingnode_checkForScreenSpilling(PopingNode* pn) {
    // Retrouver la root
    Squirrel sq;
    Node* n = &pn->n;
    sq_init(&sq, n, sq_scale_deltas);
    Root* root = NULL;
    while(sq_goUpPS(&sq)) {
        root = node_asRootOpt(sq.pos);
        if(root) break;
    }
    if(!root) { printerror("No root."); return; }
    float dx = 0;
    float dy = 0;
    // Débordement à droite...
    if(sq.v.x + sq.s.x > 0.5*root->n.w) {
        dx = ( 0.5*root->n.w - (sq.v.x + sq.s.x)) * (node_deltaX(n)/sq.s.x); // (négatif)
    }
    // Débordement à gauche...
    if(sq.v.x - sq.s.x < -0.5*root->n.w) {
        dx = (-0.5*root->n.w - (sq.v.x - sq.s.x)) * (node_deltaX(n)/sq.s.x); // (positif)
    }
    // Débordement en haut
    if(sq.v.y + sq.s.y > 0.5*root->n.h) {
        dy = ( 0.5*root->n.h - (sq.v.y + sq.s.y)) * (node_deltaY(n)/sq.s.y); // (négatif)
    }
    // Débordement en bas
    if(sq.v.y - sq.s.y < -0.5*root->n.h) {
        dy = (-0.5*root->n.h - (sq.v.y - sq.s.y)) * (node_deltaY(n)/sq.s.y); // (positif)
    }
    // Replacer x, y dans l'écran.
    if(dx || dy)
        fluid_setXY(&pn->f, (Vector2){{n->x + dx, n->y + dy}});
}


#pragma mark - PopDisk
/*-- PopDisk disk de progres qui s'autodetruit --------------------------------------------*/
typedef struct PopDisk {
    union {
        Node        n;
        PopingNode pn;
    };
    Timer*    updateTimer;
    PopDisk** referer;
    Chrono    chrono;
    float     deltaT;
    Drawable* fan;
} PopDisk;

// Overwriting du close.
void popdisk_close_(Node* n) {
    PopDisk* pop = (PopDisk*)n;
    timer_cancel(&pop->updateTimer);
    if(pop->referer) *pop->referer = (PopDisk*)NULL;
    // super
    popingnode_close_(n);
}
void popdisk_updateCallback_(Node* nd) {
    PopDisk* pd = (PopDisk*)nd;
    float elapsedSec = chrono_elapsedSec(&pd->chrono);
    if(elapsedSec < pd->deltaT + 0.10f) {
        float ratio = fminf(elapsedSec / pd->deltaT, 1.f);
        mesh_fan_update(pd->fan->_mesh, ratio);
        return;
    }
}
void   PopDisk_spawn(Node* const refOpt, PopDisk** const refererOpt,
                   uint32_t pngId, uint32_t tile, float deltaT,
                   float x, float y, float twoDy) {
    PopDisk* pop = (PopDisk*)PopingNode_spawn(refOpt, x, y, twoDy, twoDy, sizeof(PopDisk), deltaT, NULL);
    // Init as PopDisk
    pop->n.closeOpt = popdisk_close_;
    pop->deltaT = deltaT;
    chrono_start(&pop->chrono);
    timer_scheduled(&pop->updateTimer, 50, true, &pop->n, popdisk_updateCallback_);
    pop->referer = refererOpt;
    if(refererOpt) *refererOpt = pop;
    // Structure
    pop->fan = Drawable_createImageGeneral(&pop->n, Texture_sharedImage(pngId), Mesh_createFan(), 0, 0, 0, twoDy, 0, 0, 0);
    drawable_setTile(pop->fan, tile, 0);
    // Open
    node_tree_openAndShow(&pop->n);
}
void   popdisk_cancel(PopDisk** popRef) {
    if(*popRef == NULL) return;
    if(popRef != (*popRef)->referer) {
        printerror("PopDisk ref is not the popdisk...");
        return;
    }
    node_tree_close((Node*)*popRef);
}

#pragma mark - Sparkles
/*-- Feux d'artifices --------------------------------------------*/
#define SPARKLES_N_ 12
#pragma mark - DrawableMulti de la Sparkles ------------

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
void              drawablemultiSparkle_updateModels_(DrawableMulti* const dm, const Matrix4* const pm) {
    DrawableMultiSparkles_* dmsp = (DrawableMultiSparkles_*)dm;
    
    float deltaT = (float)(ChronoApp_elapsedMS() - dmsp->t0)*0.001f;
    float alpha = float_smoothOut(deltaT, 5.f);
    Vector2 const scl = {{ dm->n.sx * dm->n._piu.show, dm->n.sy * dm->n._piu.show }};
    PerInstanceUniforms* piu = dmsp->dm.piusBuffer.pius;
    PerInstanceUniforms* const end = &dmsp->dm.piusBuffer.pius[dmsp->dm._maxInstanceCount];
    const Vector2* p0 = dmsp->p0s;
    const Vector2* p1 = dmsp->p1s;
    while(piu < end) {
        Matrix4* m = &piu->model;
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
        piu++; p0++; p1++;
    }
}
DrawableMultiSparkles_* DrawableMultiSparkles_create_(Node* parent, float const height, Texture* tex) {
    DrawableMultiSparkles_* dmsp = coq_calloc(1, sizeof(DrawableMultiSparkles_));
    node_init_(&dmsp->n, parent, 0, 0, height, height, node_type_nd_multi, 0, 0);
    drawable_init_(&dmsp->d, tex, mesh_sprite, 0, height);
    drawable_updateDims_(&dmsp->d);
    drawablemulti_init_(&dmsp->dm, SPARKLES_N_);
    dmsp->dm.updateModels = drawablemultiSparkle_updateModels_;
    dmsp->t0 = ChronoApp_elapsedMS();
    // Init des per instance uniforms
    PerInstanceUniforms* piu = dmsp->dm.piusBuffer.pius;
    Vector2* p0 = dmsp->p0s;
    Vector2* p1 = dmsp->p1s;
    PerInstanceUniforms* end = &dmsp->dm.piusBuffer.pius[SPARKLES_N_];
    while(piu < end) {
        *piu = piu_default;
        uint32_t tile = rand() % tex->m*tex->n;
        piu->i = tile % tex->m;   piu->j = (tile / tex->m);
        *p0 = (Vector2) {{ rand_floatAt(0, 0.25f*height), rand_floatAt(0, 0.25f*height) }};
        *p1 = (Vector2) {{ rand_floatAt(0, 3.00f*height), rand_floatAt(0, 3.00f*height) }};
        piu++; p0++; p1++;
    }
    return dmsp;
}

void Sparkle_spawnAt(float xabs, float yabs, float delta, Texture* texOpt) {
    if(!Poping_tex_) { printerror("Poping not init."); return; }
    PopingInfo info = {
        {{ rand_floatAt(0, 0.25*delta), rand_floatAt(0, 0.25*delta), 0, 0 }},
        {{ rand_floatAt(0, 0.50*delta), rand_floatAt(0, 0.50*delta), 0, 0 }},
        10, 25, 0, 0,
    };
    PopingNode* spk = PopingNode_spawn(NULL, xabs, yabs, delta, delta, 0, 0.6, &info);
    Texture* tex = texOpt ? texOpt : Poping_tex_;
    DrawableMultiSparkles_create_(&spk->n, delta, tex);
    Sound_play(Poping_soundId_, 1.f, 0, 1.f);
    node_tree_openAndShow(&spk->n);
}
// Convenience constructor.
void Sparkle_spawnOver(Node* nd, float deltaRatio) {
    Box box = node_hitBoxInParentReferential(nd, NULL);
    float delta = deltaRatio * box.Dy;
    Sparkle_spawnAt(box.c_x, box.c_y, delta, NULL);
}

#pragma mark - PopMessage
/*-- PopMessage : Message temporaire. ----------------------------------*/
void PopMessage_spawnAt(float xabs, float yabs, float twoDxOpt, float twoDy, float timeSec, uint32_t framePngId, StringDrawable str, FramedStringParams params) {
    PopingNode* pm = PopingNode_spawn(NULL, xabs, yabs, twoDxOpt, twoDy, 0, timeSec, NULL);
    // Structure
    node_addFramedString(&pm->n, framePngId, str, params);
    popingnode_checkForScreenSpilling(pm);
    node_tree_openAndShow(&pm->n);
}

void PopMessage_spawnOver(Node* n, float widthOpt_rel, float height_rel, float timeSec, uint32_t framePngId, StringDrawable str, FramedStringParams params, bool inFrontScreen) {
    PopingNode* pm = PopingNode_spawnOver(n, widthOpt_rel, height_rel, 0, timeSec, NULL, inFrontScreen);
    pm->n.w = pm->n.h * 10;
    // Structure
    node_addFramedString(&pm->n, framePngId, str, params);
    popingnode_checkForScreenSpilling(pm);
    node_tree_openAndShow(&pm->n);
}
