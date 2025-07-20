//
//  node_drawable_multi.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2024-03-31.
//

#include "node_drawable_multi.h"

#include "../utils/util_base.h"

void drawablemulti_deinit(Node* n) {
    DrawableMulti* dm = (DrawableMulti*)n;
    // drawable_deinit_
    meshref_releaseAndNull(&dm->d._mesh);
    textureref_releaseAndNull(&dm->d.texr);
    // DrawableMulti : liberer le buffer.
    iusbuffer_deinit(&dm->iusBuffer);
}
void drawablemulti_init(DrawableMulti* dm, uint32_t const maxInstanceCount, InstanceUniforms const*const defaultIUOpt) {
    dm->n._type |= node_type_drawMulti;
    dm->n.deinitOpt = drawablemulti_deinit;  // (override drawable deinit)
    iusbuffer_init(&dm->iusBuffer, maxInstanceCount, defaultIUOpt);
}



// Fonction par défaut pour l'affichage des instances du drawable multi. -> Un tilling carré...
// Devrait être overridée.
//void drawablemulti_updateModelsDefault_(Node* const n) {
//    DrawableMulti* const dm = (DrawableMulti*)n;
//    float const show = drawable_getShow(&dm->d);
//    if(show < 0.001f) { // Rien à afficher...
//        n->renderIU.flags &= ~renderflag_toDraw;
//        return;
//    }
//    n->renderIU.flags |= renderflag_toDraw;
//    const Matrix4* const pm = node_parentModel(n);
//    IUsBufferToEdit const iusEdit = iusbuffer_rendering_retainIUsToEdit(dm->iusBuffer);
//    uint32_t const count = (uint32_t)ceilf(sqrtf(iusEdit.count));
//    float const delta = 0.5*(float)(count-1);
//    float const pop = (dm->n.flags & flag_drawablePoping) ? show : 1.f;
//    Vector3 const scl = dm->n.scales;
//    Vector3 const pos0 = {{ -delta * scl.x + dm->n.x, -delta * scl.y + dm->n.y, 0 }};
//    uint32_t tex_m = dm->d.texDims.m;
//    uint32_t tex_n = dm->d.texDims.n;
//    // Boucle sur les piu.
//    uint32_t i = 0;
//    for(InstanceUniforms* iu = iusEdit.iusBeg; iu < iusEdit.iusEnd; iu++, i++) {
//        iu->uvRect.o_x =  (i % tex_m) * iu->uvRect.w;
//        iu->uvRect.o_y = ((i / tex_m) % tex_n) * iu->uvRect.h;
//        iu->show = show;
//        Matrix4* m = &iu->model;
//        float pos_x = pos0.x + scl.x*(float)          (i%count);
//        float pos_y = pos0.y + scl.y*(float)(uint32_t)(i/count);
//        m->v0.v = pm->v0.v * scl.x * pop;
//        m->v1.v = pm->v1.v * scl.y * pop;
//        m->v2.v = pm->v2.v * scl.z * pop;
//        m->v3.v = pm->v3.v + pm->v0.v * pos_x + pm->v1.v * pos_y;
//    }
//    iusbuffer_releaseIUs(dm->iusBuffer, iusEdit.count);
//}
