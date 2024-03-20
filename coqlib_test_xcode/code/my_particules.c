//
//  my_enums.c
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//
#include "my_particules.h"
#include "my_enums.h"

#define PART_BOUNCE 1.0  // Rebond entre 0 et 1

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
    // Mise à jour de l'accélération, vitesse, position (a priori)
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
        // Last position et vitesse moyenne du déplacement (pour collision)
        p->vit_mean.x = p->vit.x + 0.5*deltaT*p->acc.x;
        p->vit_mean.y = p->vit.y + 0.5*deltaT*p->acc.y;
        p->pos_last = p->pos;
        // Nouvelle vitesse et position
        p->pos.x += p->vit.x*deltaT + 0.5f*p->acc.x*deltaT*deltaT;
        p->pos.y += p->vit.y*deltaT + 0.5f*p->acc.y*deltaT*deltaT;
        p->vit.x += p->acc.x*deltaT;
        p->vit.y += p->acc.y*deltaT;
    }
    // Detection des collisions
    float _4R2 = 4*(pp->r0 * pp->r0);
    for(Particule *a = pp->particules; a < end; a++) {
        for(Particule* b = a + 1; b < end; b++) {
            // Distance finale a priori (a par rapport à b)
            Vector2 df = vector2_minus(a->pos, b->pos);
            float df2 = vector2_norm2(df);
            // Collision ?
            if(df2 >= _4R2) continue;
            // Distance initiale
            Vector2 d0 =  vector2_minus(a->pos_last, b->pos_last);
            float d02 = vector2_norm2(d0);
            Vector2 vm =  vector2_mean(a->vit_mean, b->vit_mean);
            Vector2 var = vector2_minus(a->vit_mean, vm);
            float var2 =  vector2_norm2(var);
            float beta =  vector2_dot(d0, var);
            // Temps de collision
            float tc = 0.f;
            // Cas particulier où il y avait déjà collision, on ajoute artificiellement +1 de vitesse d'éloignement.
            if(d02 > _4R2) {
                float deter = beta*beta - var2*(d02 - _4R2);
                if(deter <= 0) { printwarning("det <= 0."); continue; }
                tc = (-beta - sqrt(deter)) / (2.f * var2);
                if(tc < 0.f) { printwarning("tc < 0."); tc = 0; }
                if(tc > deltaT) { printwarning("tc > deltaT"); continue; }
            }
            // Positions à la collision
            Vector2 ac = vector2_add(a->pos_last, vector2_times(a->vit_mean, tc));
            Vector2 bc = vector2_add(b->pos_last, vector2_times(b->vit_mean, tc));
            Vector2 dc = vector2_minus(ac, bc);
            // Composantes de la vitesse relative par rapport à la surface de collision.
            float var_dc = vector2_dot(var, dc);
            float dc2 =    vector2_norm2(dc);
            Vector2 varl = vector2_times(dc, var_dc / dc2);// vector2_projOn(var, dc);
            Vector2 vart = vector2_minus(var, varl);
            float bounce_factor = (var_dc < 0) ? -PART_BOUNCE : 1.f;  // Rebond qui si var et dc sont opposés.
            Vector2 varl_new = vector2_times(varl, bounce_factor);
            // Ajout de répulsion si overlapping (dans direction b->a)
            if(d02 < 0.95*_4R2) {
                float varl_new2 = vector2_norm2(varl_new);
                if(varl_new2 < _4R2) {
                    varl_new = vector2_add(varl_new, vector2_times(dc, sqrtf(_4R2)/sqrtf(dc2)));
                }
            }
            // Vitesse après collision et finale
            Vector2 vac = vector2_add(  vm, vector2_add(varl_new,  vart));
            Vector2 vbc = vector2_minus(vector2_minus(vm, varl_new), vart);
            a->vit = vector2_add(vac, vector2_times(a->acc, 0.5*deltaT));
            b->vit = vector2_add(vbc, vector2_times(b->acc, 0.5*deltaT));
            // Position finale
            a->pos = vector2_add(ac, vector2_times(vac, deltaT - tc));
            b->pos = vector2_add(bc, vector2_times(vbc, deltaT - tc));
            // Nouvelle position initiale et vit (pour multi-colision)
            a->vit_mean = vac;
            b->vit_mean = vbc;
            a->pos_last = vector2_add(ac, vector2_times(vac, -tc));
            b->pos_last = vector2_add(bc, vector2_times(vbc, -tc));
        }
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
PartNode_* PartNode_create(Node* ref, Vector2 pos, float rayon) {
    PartNode_* pn = coq_calloc(1, sizeof(PartNode_));
    node_init_(&pn->n, ref, pos.x, pos.y, 2*rayon, 2*rayon, node_type_nf_particule, sizeof(PartNode_), 0, 0);
    fluid_init_(&pn->f, 100);
    // Struct
    Drawable_createImage(&pn->n, png_disks, 0.f, 0.f, 2.35f*rayon, 0);
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
static const float partpool_count_ =    60;
static const float partpool_r0_ =       0.1f;
static const float partpool_brownian_ = 3.1f;
static const float partpool_repuls_ =   1.5f;

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
PPNode* PPNode_create(Node* ref) {
    PPNode* ppn = coq_calloc(1, sizeof(PPNode));
    node_init_(&ppn->n, ref, 0, 0, partpool_width_, partpool_height_, node_type_node, sizeof(PPNode), 0, 0);
    // Init as ParticulePool
    ppn->pp = particulespool_create(partpool_count_, partpool_width_, partpool_height_,
                                    partpool_r0_, partpool_brownian_, partpool_repuls_);
    ppn->n.openOpt =   ppnode_open_;
    ppn->n.closeOpt =  ppnode_close_;
    ppn->n.deinitOpt = ppnode_deinit_;
    // Struct
    Frame_create(&ppn->n, 0.5f, 0.1f*partpool_height_, 0.f, 0.f, png_frame_gray_back, frame_option_getSizesFromParent);
    for(int i = 0; i < partpool_count_; i++) {
        PartNode_create(&ppn->n, ppn->pp->particules[i].pos, partpool_r0_);
    }

    return ppn;
}

