//
//  node_drawable_multi.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2024-03-31.
//

#include "node_drawable_multi.h"

#include "utils/utils_base.h"

void drawablemulti_deinit_(Node* n) {
    DrawableMulti* dm = (DrawableMulti*)n;
    if(!mesh_isShared(dm->d._mesh))
        mesh_destroyAndNull(&dm->d._mesh);
    textureref_releaseAndNull_(&dm->d._tex);
    mtlbufferCPtrRef_releaseAndNull(&dm->_piusBufferCptr);
}

DrawableMulti* DrawableMulti_create(Node* const refOpt,
                          Texture* tex, Mesh* const mesh, uint32_t maxInstanceCount,
                          float x, float y, float twoDy,
                          flag_t flags, uint8_t node_place) {
    if(maxInstanceCount < 1) { printerror("No insantce count ?"); maxInstanceCount = 2; }
    size_t size = sizeof(DrawableMulti); // + sizeof(PerInstanceUniforms) * (maxInstanceCount - 1);
    DrawableMulti *dm = coq_calloc(1, size);
    node_init_(&dm->n, refOpt, x, y, twoDy, twoDy, node_type_leaf_drawable, flags, node_place);
    if(!texture_isSharedPng(tex)) {
        printerror("Not a png."); tex = Texture_sharedImageByName("coqlib_the_cat");
    }
    drawable_init_(&dm->d, tex, mesh, 0, twoDy);
    drawable_updateDims_(&dm->d);
    
    dm->n.deinitOpt = drawablemulti_deinit_;
    dm->_maxInstanceCount = maxInstanceCount;
    size_t bufferSize = sizeof(PerInstanceUniforms) * maxInstanceCount;
    dm->_piusBufferCptr = MTLBuffer_createAndGetCPointer(bufferSize);
    
    return dm;
}
                          
DrawableMulti*  node_asDrawableMultiOpt(Node* n) {
    if(n->_type & node_type_flag_drawMulti) return (DrawableMulti*)n;
    return NULL;
}

//void drawblemulti_updateBuffer(DrawableMulti* dm) {
//    size_t useSize = sizeof(PerInstanceUniforms) * dm->currentInstanceCount;
//    uniformbuffer_setDataAt(dm->_piusBuffer, dm->pius, useSize, 0);
//}
