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
void   piusbuffer_init(PIUsBuffer* piusbuffer, size_t size) {
    id<MTLBuffer> buffer_mtl = [MTL_device_ newBufferWithLength:size
                                    options:MTLResourceCPUCacheModeDefaultCache];
    piusbuffer->mtlBuffer_cptr = CFBridgingRetain(buffer_mtl);
    piusbuffer->size = size;
    piusbuffer->pius = [buffer_mtl contents];
//    printdebug("set piusbuffer %p, %zu %s", piusbuffer->pius, piusbuffer->size, [[buffer_mtl debugDescription] UTF8String]);
    buffer_mtl = nil;
}
// Superflu ?
//void   piusbuffer_setDataAt(PIUsBuffer* piusbuffer, const void *newData, size_t size, size_t offset) {
//    id<MTLBuffer> mtlBuffer = (__bridge id<MTLBuffer>)(piusbuffer->mtlBuffer_cptr);
//    memcpy([mtlBuffer contents] + offset, newData, size);
//}
/// Libère l'espace du buffer (et array de piu si nécessaire)
void   piusbuffer_deinit(PIUsBuffer* piusbuffer) {
    CFRelease(piusbuffer->mtlBuffer_cptr);
    piusbuffer->pius = NULL;
    piusbuffer->mtlBuffer_cptr = NULL;
    piusbuffer->size = 0;
}
id<MTLBuffer>  piusbuffer_asMTLBuffer(const PIUsBuffer* piusbuffer) {
    return (__bridge id<MTLBuffer>)piusbuffer->mtlBuffer_cptr;
}

