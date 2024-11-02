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
        n->_iu.render_flags &= ~renderflag_toDraw;
        return;
    }
    n->_iu.render_flags |= renderflag_toDraw;
    const Matrix4* const pm = node_parentModel(n);
    
    uint32_t const count = (uint32_t)ceilf(sqrtf(dm->iusBuffer.actual_count));
    float const delta = 0.5*(float)(count-1);
    float const pop = (dm->n.flags & flag_drawablePoping) ? show : 1.f;
    Vector2 const scl = dm->n.scales;
    Vector3 const pos0 = {{ -delta * scl.x + dm->n.x, -delta * scl.y + dm->n.y, 0 }};
    uint32_t tex_m = dm->d._tex->m;
    uint32_t tex_n = dm->d._tex->n;
    // Boucle sur les piu.
    uint32_t i = 0;
    InstanceUniforms* const end = &dm->iusBuffer.ius[dm->iusBuffer.actual_count];
    for(InstanceUniforms* iu = dm->iusBuffer.ius; iu < end; iu++, i++) {
        iu->draw_uvRect.o_x =  (i % tex_m) * iu->draw_uvRect.w;
        iu->draw_uvRect.o_y = ((i / tex_m) % tex_n) * iu->draw_uvRect.h;
        iu->show = show;
        Matrix4* m = &iu->model;
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
void drawablemulti_deinit(Node* n) {
    DrawableMulti* dm = (DrawableMulti*)n;
    // (Drawable super)
    if(!dm->d._mesh->isShared) {
        mesh_engine_deinit(dm->d._mesh);
        dm->d._mesh = NULL;
    }
    textureref_releaseAndNull_(&dm->d._tex);
    // Release buffer.
    iusbuffer_engine_deinit_(&dm->iusBuffer);
}
void drawablemulti_init(DrawableMulti* dm, uint32_t maxInstanceCount) {
    dm->n._type |= node_type_flag_drawMulti;
    if(maxInstanceCount < 2) { printerror("maxInstances < 2 ?"); maxInstanceCount = 2; }
    dm->n.deinitOpt = drawablemulti_deinit;  // (override drawable deinit)
    iusbuffer_engine_init_(&dm->iusBuffer, maxInstanceCount);
    dm->n.renderer_updateInstanceUniforms = drawablemulti_updateModelsDefault_;
}

                          
DrawableMulti*  node_asDrawableMultiOpt(Node* n) {
    if(n->_type & node_type_flag_drawMulti) return (DrawableMulti*)n;
    return NULL;
}

