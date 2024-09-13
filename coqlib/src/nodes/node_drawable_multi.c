//
//  node_drawable_multi.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2024-03-31.
//

#include "node_drawable_multi.h"

#include "../utils/util_base.h"
#include "../graphs/graph_colors.h"

// Fonction par défaut pour l'affichage des instances du drawable multi. -> Un tilling carré...
// Devrait être overridée.
void drawablemulti_updateModelsDefault_(Node* const n) {
    DrawableMulti* const dm = (DrawableMulti*)n;
    float const show = smtrans_setAndGetValue(&dm->d.trShow, (n->flags & flag_show) != 0);
    if(show < 0.001f) { // Rien à afficher...
        n->flags &= ~flag_drawableActive;
        return;
    }
    n->flags |= flag_drawableActive;
    const Matrix4* const pm = node_parentModel(n);
    
    uint32_t const count = (uint32_t)ceilf(sqrtf(dm->iusBuffer.actual_count));
    float const delta = 0.5*(float)(count-1);
    float const pop = (dm->n.flags & flag_poping) ? show : 1.f;
    Vector2 const scl = dm->n.scales;
    Vector3 const pos0 = {{ -delta * scl.x + dm->n.x, -delta * scl.y + dm->n.y, 0 }};
    uint32_t tex_m = dm->d._tex->m;
    uint32_t tex_n = dm->d._tex->n;
    // Boucle sur les piu.
    uint32_t i = 0;
    InstanceUniforms* const end = &dm->iusBuffer.ius[dm->iusBuffer.actual_count];
    for(InstanceUniforms* piu = dm->iusBuffer.ius; piu < end; piu++, i++) {
        piu->uvRect.o_x =  (i % tex_m) * piu->uvRect.w;
        piu->uvRect.o_y = ((i / tex_m) % tex_n) * piu->uvRect.h;
        piu->show = show;
        Matrix4* m = &piu->model;
        float pos_x = pos0.x + scl.x*(float)(i%count);
        float pos_y = pos0.y + scl.y*(float)(i/count);
        m->v0.v = pm->v0.v * scl.x * pop;
        m->v1.v = pm->v1.v * scl.y * pop;
        m->v2 =   pm->v2;
        m->v3 = (Vector4) {{
            pm->v3.x + pm->v0.x * pos_x + pm->v1.x * pos_y,
            pm->v3.y + pm->v0.y * pos_x + pm->v1.y * pos_y,
            pm->v3.z + pm->v0.z * pos_x + pm->v1.z * pos_y,
            pm->v3.w,
        }};
    }
}
void drawablemulti_deinit_(Node* n) {
    DrawableMulti* dm = (DrawableMulti*)n;
    if(!dm->d._mesh->isShared) {
        mesh_deinit(dm->d._mesh);
        dm->d._mesh = NULL;
    }
    textureref_releaseAndNull_(&dm->d._tex);
    piusbuffer_deinit_(&dm->iusBuffer);
}
void drawablemulti_init_(DrawableMulti* dm, uint32_t maxInstanceCount) {
    dm->n.deinitOpt = drawablemulti_deinit_;  // (override drawable deinit)
    piusbuffer_init_(&dm->iusBuffer, maxInstanceCount);
    dm->n.updateModel = drawablemulti_updateModelsDefault_;
}

DrawableMulti* DrawableMulti_create(Node* const refOpt,
                          Texture* tex, Mesh* const mesh, uint32_t maxInstanceCount,
                          float x, float y, float twoDy,
                          flag_t flags, uint8_t node_place) {
    if(maxInstanceCount < 1) { printerror("No insantce count ?"); maxInstanceCount = 2; }
    DrawableMulti* dm = coq_callocTyped(DrawableMulti);
    node_init(&dm->n, refOpt, x, y, 1, 1, node_type_nd_multi, flags, node_place);
    drawable_init(&dm->d, tex, mesh, 0, twoDy);
    drawablemulti_init_(dm, maxInstanceCount);
    
    return dm;
}
                          
DrawableMulti*  node_asDrawableMultiOpt(Node* n) {
    if(n->_type & node_type_flag_drawMulti) return (DrawableMulti*)n;
    return NULL;
}

