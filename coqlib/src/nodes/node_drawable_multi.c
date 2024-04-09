//
//  node_drawable_multi.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2024-03-31.
//

#include "node_drawable_multi.h"

#include "../utils/utils_base.h"
#include "../graphs/graph_colors.h"

void drawablemulti_deinit_(Node* n) {
    DrawableMulti* dm = (DrawableMulti*)n;
    if(!mesh_isShared(dm->d._mesh))
        mesh_destroyAndNull(&dm->d._mesh);
    textureref_releaseAndNull_(&dm->d._tex);
    piusbuffer_deinit(&dm->piusBuffer);
}
void drawablemulti_init_(DrawableMulti* dm, uint32_t maxInstanceCount) {
    dm->n.deinitOpt = drawablemulti_deinit_;  // (override drawable deinit)
    dm->_maxInstanceCount = maxInstanceCount;
    dm->currentInstanceCount = maxInstanceCount;
    size_t bufferSize = sizeof(PerInstanceUniforms) * maxInstanceCount;
    piusbuffer_init(&dm->piusBuffer, bufferSize);
    dm->updateModels = drawablemulti_defaultUpdateModels_;
    PerInstanceUniforms* piu = dm->piusBuffer.pius;
    PerInstanceUniforms* const end = &dm->piusBuffer.pius[dm->_maxInstanceCount];
    // Préset des piu (visible par defaut)
    while(piu < end) {
        piu->color = color4_white;
        piu->show = 1.f;
        piu ++;
    }
}

DrawableMulti* DrawableMulti_create(Node* const refOpt,
                          Texture* tex, Mesh* const mesh, uint32_t maxInstanceCount,
                          float x, float y, float twoDy,
                          flag_t flags, uint8_t node_place) {
    if(maxInstanceCount < 1) { printerror("No insantce count ?"); maxInstanceCount = 2; }
    size_t size = sizeof(DrawableMulti); // + sizeof(PerInstanceUniforms) * (maxInstanceCount - 1);
    DrawableMulti *dm = coq_calloc(1, size);
    node_init_(&dm->n, refOpt, x, y, twoDy, twoDy, node_type_nd_multi, flags, node_place);
    if(!(tex->flags & tex_flag_png_shared)) {
        printerror("Not a png."); tex = Texture_sharedImageByName("coqlib_the_cat");
    }
    drawable_init_(&dm->d, tex, mesh, 0, twoDy);
    drawable_updateDims_(&dm->d);
    drawablemulti_init_(dm, maxInstanceCount);
    
    return dm;
}
                          
DrawableMulti*  node_asDrawableMultiOpt(Node* n) {
    if(n->_type & node_type_flag_drawMulti) return (DrawableMulti*)n;
    return NULL;
}

// Fonction par défaut pour l'affichage des instances du drawable multi. -> Un tilling carré...
void    drawablemulti_defaultUpdateModels_(DrawableMulti* const dm, const Matrix4* const pm) {
    uint32_t const n = (uint32_t)ceilf(sqrtf(dm->currentInstanceCount));
    float const delta = 0.5*(float)(n-1);
    float const show = dm->n._piu.show;
    float const pop = (dm->n.flags & flag_poping) ? show : 1.f;
    Vector2 const scl = dm->n.scales;
    Vector3 const pos0 = {{ -delta * scl.x + dm->n.x, -delta * scl.y + dm->n.y, 0 }};
    uint32_t tex_m = dm->d._tex->m;
    uint32_t tex_n = dm->d._tex->n;
    // Boucle sur les piu.
    uint32_t i = 0;
    PerInstanceUniforms* piu = dm->piusBuffer.pius;
    PerInstanceUniforms* const end = &dm->piusBuffer.pius[dm->_maxInstanceCount];
    while(piu < end) {
        piu->i = i % tex_m;
        piu->j = (i / tex_m) % tex_n;
        piu->show = show;
        Matrix4* m = &piu->model;
        float pos_x = pos0.x + scl.x*(float)(i%n);
        float pos_y = pos0.y + scl.y*(float)(i/n);
        m->v0.v = pm->v0.v * scl.x * pop;
        m->v1.v = pm->v1.v * scl.y * pop;
        m->v2 =   pm->v2;
        m->v3 = (Vector4) {{
            pm->v3.x + pm->v0.x * pos_x + pm->v1.x * pos_y,
            pm->v3.y + pm->v0.y * pos_x + pm->v1.y * pos_y,
            pm->v3.z + pm->v0.z * pos_x + pm->v1.z * pos_y,
            pm->v3.w,
        }};
        i ++;
        piu ++;
    }
}
