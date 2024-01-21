//
//  my_enums.c
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//
#include "my_particules.h"
#include "my_enums.h"

ParticulesPool* particulespool_create(uint32_t count, float twoDx, float twoDy,
                                      float r0, float brownian, float repulsion) {
    ParticulesPool* const pp = coq_malloc(sizeof(ParticulesPool) + sizeof(Particule) * (count - 1));
    pp->count = count;
    pp->deltaX = 0.5f*twoDx;
    pp->deltaY = 0.5f*twoDy;
    pp->r0 = r0;
    pp->brownian = brownian;
    pp->repulsion = repulsion;
    Particule* p = pp->particules;
    Particule* const end = &pp->particules[count];
    while(p < end) {
        p->pos = rand_vector2_inBox((Box) {{ 0.f, 0.f, 0.45f*twoDx, 0.45f*twoDy }});
        p->vit = rand_vector2_ofNorm(0.1f*brownian);
        p++;
    }
    return pp;
}

void           particulespool_update(ParticulesPool* pp, float const deltaT) {
    // Init
    Particule* const end = &pp->particules[pp->count];
    for(Particule* p = pp->particules; p < end; p++) {
        p->acc = vector2_zeros;
    }
    // Repulsion entre particules
    for(Particule* p = pp->particules; p < end; p++) {
        for(Particule* p2 = p + 1; p2 < end; p2++) {
            // Position relative
            Vector2 v = {{ p->pos.x - p2->pos.x, p->pos.y - p2->pos.y }};
            float   v_n = sqrtf(v.x*v.x + v.y*v.y);
            // Force entre les deux.
            Vector2 a;
            if(v_n <= 0.5*pp->r0) {
                a = rand_vector2_ofNorm(4.f*pp->repulsion);
            } else {
                float num = pp->repulsion*pp->r0*pp->r0;
                a = (Vector2) {{ num * v.x / powf(v_n, 3.f), num * v.y / powf(v_n, 3.f) }};
            }
            p ->acc.x += a.x; p ->acc.y += a.y;
            p2->acc.x -= a.x; p2->acc.y -= a.y;
        }
        // Repulsion des bords (puit de potentiel), mouvement "brownien" et friction.
        Vector2 brown = rand_vector2_ofNorm(pp->brownian);
        float vit_n = sqrtf(p->vit.x*p->vit.x + p->vit.y*p->vit.y);
        p->acc.x += -(5.f*pp->repulsion + 3.f)*powf(p->pos.x/pp->deltaX, 5.f) + brown.x - 1.0f*p->vit.x*vit_n;
        p->acc.y += -(5.f*pp->repulsion + 3.f)*powf(p->pos.y/pp->deltaY, 5.f) + brown.y - 1.0f*p->vit.y*vit_n;
        // Nouvelle vitesse et position
        p->pos.x += p->vit.x*deltaT + 0.5f*p->acc.x*deltaT*deltaT;
        p->pos.y += p->vit.y*deltaT + 0.5f*p->acc.y*deltaT*deltaT;
        p->vit.x += p->acc.x*deltaT;
        p->vit.y += p->acc.y*deltaT;
    }
}



/*-- structure... --*/

typedef struct {
    union {
        Node   n;
        Fluid  f;
//        Button b;
    };
} PartNode_;

//void partnode_action_(Button* b) {
//    Sparkle_spawnOver(&b->n, 1.f);
//}
PartNode_* PartNode_create(Node* ref, Root* root, Vector2 pos, float rayon) {
    PartNode_* pn = Node_createEmptyOfType_(node_type_nf_particule, sizeof(PartNode_), 0, ref, 0);
    pn->n.x = pos.x;   pn->n.y = pos.y;
    pn->n.w = 2*rayon; pn->n.h = 2*rayon;
    fluid_init_(&pn->f, 20);
    // Struct
    Drawable_createImage(&pn->n, png_disks, 0.f, 0.f, 2.f*rayon, 0);
    drawable_last_setTile(rand() % 13, 0);
    return pn;
}
PartNode_* node_asPartNodeOpt_(Node* n) {
    if(n->_type & node_type_flag_particule)
        return (PartNode_*)n;
    return NULL;
}

static const float partpool_width_ =    4.f;
static const float partpool_height_ =   2.5f;
static const float partpool_count_ =    100;
static const float partpool_r0_ =       0.10f;
static const float partpool_brownian_ = 5.1f;
static const float partpool_repuls_ =   2.5f;

void ppnode_callback_(Node* nd) {
    PPNode* ppn = (PPNode*)nd;
    particulespool_update(ppn->pp, Chrono_UpdateDeltaTMS*0.001f);
    Particule* p = ppn->pp->particules;
    Particule* const end = &ppn->pp->particules[ppn->pp->count];
    Node* n = ppn->n.firstChild;
    while(p < end && n) {
        PartNode_* pn = node_asPartNodeOpt_(n);
        n = n->littleBro;
        if(!pn) continue;
        fluid_setXY(&pn->f, p->pos);
        p++;
    }
}
void ppnode_open_(Node* nd) {
    PPNode* ppn = (PPNode*)nd;
    timer_scheduled(&ppn->timer, 1, true, nd, ppnode_callback_);
}
void ppnode_close_(Node* nd) {
    PPNode* ppn = (PPNode*)nd;
    timer_cancel(&ppn->timer);
}
void ppnode_deinit_(Node* nd) {
    PPNode* ppn = (PPNode*)nd;
    coq_free(ppn->pp);
}
PPNode* PPNode_create(Node* ref, Root* root) {
    PPNode* ppn = Node_createEmptyOfType_(node_type_node, sizeof(PPNode), 0, ref, 0);
    ppn->n.w = partpool_width_;
    ppn->n.h = partpool_height_;
    // Init as ParticulePool
    ppn->pp = particulespool_create(partpool_count_, partpool_width_, partpool_height_,
                                    partpool_r0_, partpool_brownian_, partpool_repuls_);
    ppn->n.openOpt =   ppnode_open_;
    ppn->n.closeOpt =  ppnode_close_;
    ppn->n.deinitOpt = ppnode_deinit_;
    // Struct
    Frame_create(&ppn->n, 0.5f, 0.1f*partpool_height_, 0.f, 0.f, png_frame_gray_back, frame_option_getSizesFromParent);
    for(int i = 0; i < partpool_count_; i++) {
        PartNode_create(&ppn->n, root, ppn->pp->particules[i].pos, partpool_r0_);
    }

    return ppn;
}

