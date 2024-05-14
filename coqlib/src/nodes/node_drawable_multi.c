//
//  node_drawable_multi.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2024-03-31.
//

#include "node_drawable_multi.h"

#include "../utils/util_base.h"
#include "../graphs/graph_colors.h"

void drawablemulti_deinit_(Node* n) {
    DrawableMulti* dm = (DrawableMulti*)n;
    if(!mesh_isShared(dm->d._mesh))
        mesh_destroyAndNull(&dm->d._mesh);
    textureref_releaseAndNull_(&dm->d._tex);
    piusbuffer_deinit_(&dm->piusBuffer);
}
void drawablemulti_init_(DrawableMulti* dm, uint32_t maxInstanceCount) {
    dm->n.deinitOpt = drawablemulti_deinit_;  // (override drawable deinit)
    piusbuffer_init_(&dm->piusBuffer, maxInstanceCount);
    dm->n.updateModel = drawablemulti_updateModelsDefault_;
    PerInstanceUniforms* piu = dm->piusBuffer.pius;
    PerInstanceUniforms* const end = &dm->piusBuffer.pius[dm->piusBuffer.actual_count];
    while(piu < end) {
        *piu = dm->n._piu;
        piu ++;
    }
}

DrawableMulti* DrawableMulti_create(Node* const refOpt,
                          Texture* tex, Mesh* const mesh, uint32_t maxInstanceCount,
                          float x, float y, float twoDy,
                          flag_t flags, uint8_t node_place) {
    if(maxInstanceCount < 1) { printerror("No insantce count ?"); maxInstanceCount = 2; }
    size_t size = sizeof(DrawableMulti); // + sizeof(PerInstanceUniforms) * (maxInstanceCount - 1);
    DrawableMulti* dm = coq_calloc(1, size);
    node_init_(&dm->n, refOpt, x, y, twoDy, twoDy, node_type_nd_multi, flags, node_place);
    if(!(tex->flags & tex_flag_png)) {
        printerror("Not a png."); tex = Texture_sharedImageByName("coqlib_the_cat");
    }
    drawable_init_(&dm->d, tex, mesh, 0, twoDy, 0);
    drawablemulti_init_(dm, maxInstanceCount);
    
    return dm;
}
                          
DrawableMulti*  node_asDrawableMultiOpt(Node* n) {
    if(n->_type & node_type_flag_drawMulti) return (DrawableMulti*)n;
    return NULL;
}

// Fonction par défaut pour l'affichage des instances du drawable multi. -> Un tilling carré...
// Devrait être overridée.
Drawable* drawablemulti_updateModelsDefault_(Node* const n) {
    DrawableMulti* const dm = (DrawableMulti*)n;
    float const show = smtrans_setAndGetIsOnSmooth(&dm->d.trShow, (n->flags & flag_show) != 0);
    if(show < 0.001f)  // Rien à afficher...
        return NULL;
    const Node* const parent = n->_parent;
    if(!parent) { printwarning("DrawableMulti without parent."); return NULL; }
    const Matrix4* const pm = &parent->_piu.model;
    
    uint32_t const count = (uint32_t)ceilf(sqrtf(dm->piusBuffer.actual_count));
    float const delta = 0.5*(float)(count-1);
    float const pop = (dm->n.flags & flag_poping) ? show : 1.f;
    Vector2 const scl = dm->n.scales;
    Vector3 const pos0 = {{ -delta * scl.x + dm->n.x, -delta * scl.y + dm->n.y, 0 }};
    uint32_t tex_m = dm->d._tex->m;
    uint32_t tex_n = dm->d._tex->n;
    // Boucle sur les piu.
    uint32_t i = 0;
    PerInstanceUniforms*       piu =  dm->piusBuffer.pius;
    PerInstanceUniforms* const end = &dm->piusBuffer.pius[dm->piusBuffer.actual_count];
    while(piu < end) {
        piu->u0 =  (i % tex_m) * piu->Du;
        piu->v0 = ((i / tex_m) % tex_n) * piu->Dv;
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
        i ++;
        piu ++;
    }
    return &dm->d;
}
