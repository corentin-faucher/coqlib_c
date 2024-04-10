//
//  graph_piusbuffer_apple.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-07.
//

#include "graphs/graph_base.h"
#include "graph__apple.h"
#include "utils/utils_base.h"

#pragma mark - Version Metal des Uniform Buffer -----------------------

/// Création du buffer. 
void   piusbuffer_init_(PIUsBuffer* piusbuffer, uint32_t count) {
    size_t size = count * sizeof(PerInstanceUniforms);
    id<MTLBuffer> buffer_mtl = [MTL_device_ newBufferWithLength:size
                                    options:MTLResourceCPUCacheModeDefaultCache];
    *piusbuffer = (PIUsBuffer) {
        count, count, size,
        [buffer_mtl contents], CFBridgingRetain(buffer_mtl)
    };
    buffer_mtl = nil;
}

/// Libère l'espace du buffer (et array de piu si nécessaire)
void   piusbuffer_deinit_(PIUsBuffer* piusbuffer) {
    CFRelease(piusbuffer->_mtlBuffer_cptr);
    *piusbuffer = (PIUsBuffer) { 0 };
}

id<MTLBuffer>  piusbuffer_asMTLBuffer(const PIUsBuffer* piusbuffer) {
    return (__bridge id<MTLBuffer>)piusbuffer->_mtlBuffer_cptr;
}

