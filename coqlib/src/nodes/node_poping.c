//
//  pop_disk.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-28.
//

#include "nodes/node_poping.h"

#include "nodes/node_squirrel.h"
#include "nodes/node_tree.h"
#include "nodes/node_root.h"
#include "coq_sound.h"

#pragma mark - Poping Base
static View*    Poping_frontView_ = NULL;
static Texture* Poping_tex_ = NULL;
static uint32_t Poping_soundId_ = 0;

static const PopingInfo Poping_defaultInfo_ = {
    { 0.0, 0.0, -0.4, -0.2},
    { 0.0, 1.0,  0.0,  0.0},
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
#define SPARKLES_N_ 8

void Sparkle_spawnAt(float xabs, float yabs, float delta, Texture* texOpt) {
    if(!Poping_tex_) { printerror("Poping not init."); return; }
    PopingInfo info = {
        {rand_floatAt(0, 0.25*delta), rand_floatAt(0, 0.25*delta), 0, 0},
        {rand_floatAt(0, 0.50*delta), rand_floatAt(0, 0.50*delta), 0, 0},
        10, 25, 0, 0,
    };
    PopingNode* spk = PopingNode_spawn(NULL, xabs, yabs, delta, delta, 0, 0.6, &info);
    Texture* tex = texOpt ? texOpt : Poping_tex_;
    uint32_t MN = texture_mn(tex);
    for(int i = 0; i < SPARKLES_N_; i ++) {
        Fluid* part = Fluid_create(&spk->n,
             rand_floatAt(0, 0.1*delta), rand_floatAt(0, 0.1*delta), 1, 1, 5, 0, 0);
        Drawable* d = Drawable_createImageGeneral(&part->n, tex, mesh_sprite, 0, 0, 0, 0.4f*delta, 0, flag_poping, 0);
        drawable_setTile(d, rand() % MN, 0);
        fl_set(&part->x, rand_floatAt(0, delta));
        fl_set(&part->y, rand_floatAt(0, delta));
    }
    Sound_play(Poping_soundId_, 1.f, 0, 1.f);
    node_tree_openAndShow(&spk->n);
    
    // Despawn après 0.6 s.
    timer_scheduled(&spk->timer, 600, false, &spk->n, node_tree_close);
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
