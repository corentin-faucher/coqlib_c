//
//  my_enums.c
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//
#include "my_particules.h"
#include "my_enums.h"
#include "utils/utils_base.h"
#include "graphs/graph_colors.h"

#define PART_BOUNCE 0.5  // Rebond entre 0 et 1 (si > 1 -> gain d'énergie...)
static const uint32_t partpool_count_ =     100;
static const float    partpool_width_ =     4.f;
static const float    partpool_height_ =   2.5f;
static const float    partpool_r0_ =      0.03f;
static const float    partpool_brownian_ = 3.1f;
static const float    partpool_repuls_ =   1.5f;

#pragma mark - Particule Pool ------------------------------

Vector2 particule_evalPos(Particule* p, float dT, bool zeroOne, bool print) {
    if(print) {
//        printdebug("%p EVAL dT %f pos 1 %f %f, pos 0 %f %f, vit 1 %f %f, vit 0 %f %f.",
//             p, dT, p->pos1.x, p->pos1.y, p->pos0.x, p->pos0.y,
//             p->vit1.x, p->vit1.y, p->vit0.x, p->vit0.y);
    }
    if(zeroOne) {
        return (Vector2) {{ p->pos1.x + p->vit1.x * dT, p->pos1.y + p->vit1.y * dT }};
    } else {
        return (Vector2) {{ p->pos0.x + p->vit0.x * dT, p->pos0.y + p->vit0.y * dT }};
    }
}

ParticulesPool* ParticulesPool_create(uint32_t count, float twoDx, float twoDy,
                                      float r0, float brownian, float repulsion) {
    ParticulesPool* const pp = coq_calloc(1, sizeof(ParticulesPool) + sizeof(Particule) * (count - 1));
    pp->count = count;
    pp->deltaX = 0.5f*twoDx;
    pp->deltaY = 0.5f*twoDy;
    pp->r0 = r0;
    pp->brownian = brownian;
    pp->repulsion = repulsion;
    pp->t0 = -1;
    Particule* p = pp->particules;
    Particule* const end = &pp->particules[count];
    while(p < end) {
        p->pos0 = rand_vector2_inBox((Box) {{ 0.f, 0.f, 0.45f*twoDx, 0.45f*twoDy }});
        p->pos1 = p->pos0;
        p->vit0 = rand_vector2_ofNorm(0.1f*brownian);
        p->vit1 = p->vit0;
        p->_vit = p->vit0;
        p++;
    }
    return pp;
}
void            particulespool_update(ParticulesPool* pp) {
    // Delta T, constant à 50 ms = 0.05 s.
    float const deltaT = (float)Chrono_UpdateDeltaTMS * 0.001f;  // (en sec, mieux d'avoir les vitesse/accélération en m/s, m/s2...?)
//    int64_t time = ChronoApp_elapsedMS();
//    int64_t deltaTMS = time - pp->last_time;
//    deltaTMS = (deltaTMS > Chrono_UpdateDeltaTMS * 1.1f) ? 100 : (deltaTMS < 5 ? 5 : deltaTMS);
//    pp->last_time = time;
    
    // Fixer la nouvelle position (à l'aide des calcul du tick précédent) 
    // et init acc pour calcul de nouvelle vitesse (actif durant jusqu'au next tick). 
    Particule* const end = &pp->particules[pp->count];
    bool update0 = pp->t0 < pp->t1;
    if(update0) for(Particule* p = pp->particules; p < end; p++) {
        p->pos0.x = p->pos1.x + p->vit1.x * deltaT;
        p->pos0.y = p->pos1.y + p->vit1.y * deltaT;
        p->_posi = p->pos0;  // (posi pourrait changer à cause des collisions)
        p->_acc = vector2_zeros;
    } else for(Particule* p = pp->particules; p < end; p++) {
        p->pos1.x = p->pos0.x + p->vit0.x * deltaT;
        p->pos1.y = p->pos0.y + p->vit0.y * deltaT;
        p->_posi = p->pos1;
        p->_acc = vector2_zeros;
    }
    
    // Calcul de l'accélération, à la nouvelle position.
    for(Particule* p = pp->particules; p < end; p++) {
        for(Particule* p2 = p + 1; p2 < end; p2++) {
            // Position relative (distance vectoriel et scalaire)
            Vector2 d = update0 ? (Vector2){{ p->pos0.x - p2->pos0.x, p->pos0.y - p2->pos0.y }} 
                                : (Vector2){{ p->pos1.x - p2->pos1.x, p->pos1.y - p2->pos1.y }}; 
            float   d_n = sqrtf(d.x*d.x + d.y*d.y);
            // Force entre les deux.
            Vector2 a;
            if(d_n <= 0.5*pp->r0) {
                a = rand_vector2_ofNorm(4.f*pp->repulsion);
            } else {
                float num = pp->repulsion*pp->r0*pp->r0;
                a = (Vector2) {{ num * d.x / powf(d_n, 3.f), num * d.y / powf(d_n, 3.f) }};
            }
            p ->_acc.x += a.x; p ->_acc.y += a.y;
            p2->_acc.x -= a.x; p2->_acc.y -= a.y;
        }
        // Repulsion des bords (puit de potentiel), mouvement "brownien" et friction.
        Vector2 brown = rand_vector2_ofNorm(pp->brownian);
        Vector2 vit = p->_vit;  // Récupération de la vitesse finale estimé précédemment (maintenant vitesse présente)
        float vit_n = vector2_norm(vit);
        p->_acc.x += -(5.f*pp->repulsion + 3.f)*powf(p->_posi.x/pp->deltaX, 5.f) + brown.x - 1.0f*vit.x*vit_n;
        p->_acc.y += -(5.f*pp->repulsion + 3.f)*powf(p->_posi.y/pp->deltaY, 5.f) + brown.y - 1.0f*vit.y*vit_n;
        // Vitesse final -> vitesse moyenne sur prochain tick pour collisions.
        p->_vit.x += 0.5*deltaT*p->_acc.x;
        p->_vit.y += 0.5*deltaT*p->_acc.y;
        // Position finale estimé (pour détecter les collision)
        p->_posf.x = p->_posi.x + p->_vit.x*deltaT;
        p->_posf.y = p->_posi.y + p->_vit.y*deltaT;
    }
    // Detection des (futurs) collisions
    // Ici les collisions détectés sont durant la next `tick`.
    // Pour comprendre, il faut faire un dessin ! :)
    // -> Deux boules de rayon R...
    float _4R2 = 4*(pp->r0 * pp->r0);
    for(Particule *a = pp->particules; a < end; a++) {
        for(Particule* b = a + 1; b < end; b++) {
            // Distance finale a priori (a par rapport à b)
            Vector2 df = vector2_minus(a->_posf, b->_posf);
            float df2 = vector2_norm2(df);
            // Pas de collision ? (On néglige les cas peu probables de frottement dans l'intervalle dT)
            if(df2 >= _4R2) continue;
            // Distance initiale
            Vector2 d0 =  vector2_minus(a->_posi, b->_posi);
            float d02 = vector2_norm2(d0);
            Vector2 vm =  vector2_mean(a->_vit, b->_vit); // Vitesse commune (ou moyenne)
            Vector2 var = vector2_minus(a->_vit, vm);     // Vitesse de rapprochement/eloignement (vit de a relelative au point de collision)
            float var2 =  vector2_norm2(var);
            float beta =  vector2_dot(d0, var);
            // Instant de la collision
            float tc = 0.f;
            if(var2 == 0) { printwarning("var2 == 0"); } 
            // Cas particulier où il y avait déjà collision, on ajoute artificiellement +1 de vitesse d'éloignement.
            if(d02 > _4R2) {
                float deter = beta*beta - var2*(d02 - _4R2);
                if(deter <= 0) { printwarning("det <= 0."); continue; }
                tc = (-beta - sqrt(deter)) / (2.f * var2);
                if(tc < 0.f) { printwarning("tc < 0."); tc = 0; }
                if(tc > deltaT) { printwarning("tc > deltaT"); continue; }
            }
            // Positions à la collision
            Vector2 ac = vector2_add(a->_posi, vector2_times(a->_vit, tc));
            Vector2 bc = vector2_add(b->_posi, vector2_times(b->_vit, tc));
            Vector2 dc = vector2_minus(ac, bc);
            // Composantes de la vitesse relative par rapport à la surface de collision.
            float var_dc = vector2_dot(var, dc);
            float dc2 =    vector2_norm2(dc);
            if(dc2 == 0) { printerror("dc2 == 0 ?"); }
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
            // Vitesse après collision
            Vector2 vac = vector2_add(  vm, vector2_add(varl_new,  vart));
            Vector2 vbc = vector2_minus(vector2_minus(vm, varl_new), vart);
            a->_vit = vac;
            b->_vit = vbc;
            // Réestimation de la position finale
            a->_posf = vector2_add(ac, vector2_times(vac, deltaT - tc));
            b->_posf = vector2_add(bc, vector2_times(vbc, deltaT - tc));
            // Nouvelle position initiale (virtuel, i.e. sans collision)
            a->_posi = vector2_add(ac, vector2_times(vac, -tc));
            b->_posi = vector2_add(bc, vector2_times(vbc, -tc));
        }
    }
    // Il reste à setter la future vitesse finale et vitesse moyenne sur next tick.
    for(Particule* p = pp->particules; p < end; p++) {
        // Vitesse moyenne -> Vitesse final
        p->_vit.x += 0.5*deltaT*p->_acc.x;
        p->_vit.y += 0.5*deltaT*p->_acc.y;
        // Vitesse d'interpolation (variation linéaire entre pos0 et posf).
        if(update0) {
            p->vit0.x = (p->_posf.x - p->pos0.x) / deltaT;
            p->vit0.y = (p->_posf.y - p->pos0.y) / deltaT;
        } else {
            p->vit1.x = (p->_posf.x - p->pos1.x) / deltaT;
            p->vit1.y = (p->_posf.y - p->pos1.y) / deltaT;
        }
//        if(p == pp->particules) {
//            printdebug("%p pos 1 %f %f, pos 0 %f %f, vit 1 %f %f, vit 0 %f %f.",
//             p, p->pos1.x, p->pos1.y, p->pos0.x, p->pos0.y,
//             p->vit1.x, p->vit1.y, p->vit0.x, p->vit0.y);
//        }
    }
    // Tout est setté, on peut faire le switch.
    if(update0) {
        pp->t0 = ChronoApp_elapsedMS(); // (nouveau temps le plus récent)
    } else {
        pp->t1 = ChronoApp_elapsedMS();
    }
}


#pragma mark - DrawableMulti de la particule pool ------------

typedef struct DrawableMultiPP_ {
    union {
        Node          n;
        Drawable      d;
        DrawableMulti dm;
    };
    ParticulesPool*   pp;
} DrawableMultiPP_;
// Override...
void              drawablemultiPP_updateModels_(DrawableMulti* const dm, const Matrix4* const pm) {
    DrawableMultiPP_* dmpp = (DrawableMultiPP_*)dm;
    // Mode 0/1, DeltaT, w/h.
    bool zeroOne = dmpp->pp->t1 > dmpp->pp->t0;
    float deltaT = (float)(ChronoApp_elapsedMS() - (zeroOne ? dmpp->pp->t1 : dmpp->pp->t0))*0.001f;
    Vector2 const scl = dm->n.scales;
    // Boucle sur les particules
    Particule* p = dmpp->pp->particules;
    PerInstanceUniforms* piu = dm->piusBuffer.pius;
    PerInstanceUniforms* const end = &dm->piusBuffer.pius[dm->piusBuffer.actual_count];
    while(piu < end) {
        Matrix4* m = &piu->model;
        // Petite translation sur la parent-matrix en fonction de la particule.
        Vector2 const pos = particule_evalPos(p, deltaT, zeroOne, piu == dm->piusBuffer.pius);
        if(isnan(pos.x) || isnan(pos.y)) { printerror("Nan value ..."); }
        m->v0.v = pm->v0.v * scl.x;
        m->v1.v = pm->v1.v * scl.y;
        m->v2 =   pm->v2;
        
        m->v3 = (Vector4) {{
            pm->v3.x + pm->v0.x * pos.x + pm->v1.x * pos.y,
            pm->v3.y + pm->v0.y * pos.x + pm->v1.y * pos.y,
            pm->v3.z + pm->v0.z * pos.x + pm->v1.z * pos.y,
            pm->v3.w,
        }};
        piu++; p++;
    }
    // Setter le Uniform buffer.
//    mtlbufferCptr_setDataAt(dm->_piusBufferCptr, dmpp->pius, sizeof(PerInstanceUniforms)*dm->currentInstanceCount, 0);
}
DrawableMultiPP_* DrawableMultiPP_create(Node* parent, ParticulesPool* pp) {
    DrawableMultiPP_* dmpp = coq_calloc(1, sizeof(DrawableMultiPP_));
    node_init_(&dmpp->n, parent, 0, 0, 2.35*partpool_r0_, 2.35*partpool_r0_, node_type_nd_multi, 0, 0);
    Texture* const tex = Texture_sharedImage(png_disks);
    drawable_init_(&dmpp->d, tex, mesh_sprite, 0, 2.35*partpool_r0_);
    drawable_updateDims_(&dmpp->d);
    drawablemulti_init_(&dmpp->dm, partpool_count_);
    dmpp->dm.updateModels = drawablemultiPP_updateModels_;
    dmpp->pp = pp;
    // Init des per instance uniforms (juste setter une tile.
    PerInstanceUniforms* piu =  dmpp->dm.piusBuffer.pius;
    PerInstanceUniforms* end = &dmpp->dm.piusBuffer.pius[partpool_count_];
    while(piu < end) {
        *piu = piu_default;
        uint32_t tile = rand() % 12;
        piu->i = tile % tex->m;   piu->j = (tile / tex->m) % tex->n;
        piu++;
    }
    return dmpp;
}


#pragma mark - Particule pool node --------------

void ppnode_callback_(Node* nd) {
    PPNode* ppn = (PPNode*)nd;
    
    particulespool_update(ppn->pp);
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
    node_init_(&ppn->n, ref, 0, 0,  partpool_width_, partpool_height_, node_type_node, 0, 0);
    // Init as ParticulePool
    ppn->pp = ParticulesPool_create(partpool_count_, partpool_width_, partpool_height_,
                                    partpool_r0_, partpool_brownian_, partpool_repuls_);
    ppn->n.openOpt =   ppnode_open_;
    ppn->n.closeOpt =  ppnode_close_;
    ppn->n.deinitOpt = ppnode_deinit_;
    // Struct
    Frame_create(&ppn->n, 0.5f, 0.1f*partpool_height_, 0.f, 0.f, png_frame_gray_back, frame_option_getSizesFromParent);
    DrawableMultiPP_create(&ppn->n, ppn->pp);

    return ppn;
}

