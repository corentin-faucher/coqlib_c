//
//  graph_iusbuffer.c
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-15.
//

#include "graph_iusbuffer.h"
#include "graph__metal.h"
#include "../coq__buildConfig.h"
#include "../utils/util_base.h"
//#import <MetalKit/MetalKit.h>

/// Init du buffer.
void       iusbuffer_init(IUsBuffer*const iusbuffer, uint32_t maxCount, InstanceUniforms const*const defaultIUOpt) 
{
    if(iusbuffer->iusOpt || iusbuffer->_mtlBufferOptA || iusbuffer->_mtlBufferOptB) {
        printerror("Arleady init."); return;
    }
    if(maxCount < 2) {
        if(IUsBuffer_warningMaxCount) printwarning("IUsBuffer with maxCount < 2.");
        maxCount = 2;
    }
    size_t const max_size = maxCount * sizeof(InstanceUniforms);
    memset(iusbuffer, 0, sizeof(IUsBuffer));
    size_initConst(&iusbuffer->actual_count, 0);
    size_initConst(&iusbuffer->actual_size, 0);
    size_initConst(&iusbuffer->max_count, maxCount);
    InstanceUniforms *iusA = NULL, *iusB = NULL;
    // Cas simple, pas besoin de metal buffer (envoie direct d'un array de IUs).
    if(max_size < IUsBuffer_sizeForBuffer) {
        *(InstanceUniforms**)&iusbuffer->iusOpt = coq_callocSimpleArray(maxCount, InstanceUniforms);
        iusA = (InstanceUniforms*)iusbuffer->iusOpt;
    }
    // Cas double buffer de MTLBuffer
    else {
        id<MTLBuffer> mtlBufferA = [CoqGraph_metal_device newBufferWithLength:max_size
                                    options:MTLResourceCPUCacheModeDefaultCache];
        id<MTLBuffer> mtlBufferB = [CoqGraph_metal_device newBufferWithLength:max_size
                                    options:MTLResourceCPUCacheModeDefaultCache];
        *(const void**)&iusbuffer->_mtlBufferOptA = CFBridgingRetain(mtlBufferA);
        *(const void**)&iusbuffer->_mtlBufferOptB = CFBridgingRetain(mtlBufferB);
        *(const void**)&iusbuffer->mtlBufferOpt = iusbuffer->_mtlBufferOptA;
        iusA = [mtlBufferA contents];
        iusB = [mtlBufferB contents];
        mtlBufferA = nil;
        mtlBufferB = nil;
    }
    // Préremplissage du buffer.
    if(defaultIUOpt) {
        InstanceUniforms *const end = &iusA[maxCount];
        for(InstanceUniforms* iu = iusA; iu < end; iu++) { *iu = *defaultIUOpt; }
        if(iusB) memcpy(iusB, iusA, max_size);
    }
}
/// Libère l'espace du buffer (et array de piu si nécessaire)
void   iusbuffer_deinit(IUsBuffer*const iusbuffer) {
    if(iusbuffer->_mtlBufferOptA) {
        CFRelease(iusbuffer->_mtlBufferOptA);
        *(const void**)&iusbuffer->_mtlBufferOptA = NULL;
    }
    if(iusbuffer->_mtlBufferOptB) {
        CFRelease(iusbuffer->_mtlBufferOptB);
        *(const void**)&iusbuffer->_mtlBufferOptB = NULL;
    }
    if(iusbuffer->iusOpt) {
        coq_free((InstanceUniforms*)iusbuffer->iusOpt);
        *(InstanceUniforms**)&iusbuffer->iusOpt = NULL;
    }
}

IUsToEdit iusbuffer_retainIUsToEdit(IUsBuffer *const iusBuffer) {
    if(!iusBuffer) { printerror("No iusBuffer."); return (IUsToEdit) {}; }
    if(iusBuffer->_editing) { printerror("Already editing."); return (IUsToEdit) {}; }
    // Cas rien à éditer...
    if(!iusBuffer->max_count) {
        return (IUsToEdit) {};
    }
    iusBuffer->_editing = true;
    // Cas simple sans buffer metal (juste l'array ius)
    if(iusBuffer->iusOpt) {
        InstanceUniforms *const beg = (InstanceUniforms*)iusBuffer->iusOpt;
        return (IUsToEdit) {
            .beg =  beg,
            .end = &beg[iusBuffer->max_count],
            .iu =   beg,
            ._iusBuffer = iusBuffer,
        };
    }
    if(!iusBuffer->_mtlBufferOptA) { printerror("Buffer not init."); return (IUsToEdit) {}; }
    // Sinon, on édite le MTLBuffer qui n'est pas actif...
    bool const AisLive = iusBuffer->mtlBufferOpt == iusBuffer->_mtlBufferOptA;
    void const*const mtlBufferToEdit = AisLive ?
        iusBuffer->_mtlBufferOptB : iusBuffer->_mtlBufferOptA;;
    InstanceUniforms *const beg = [(__bridge id<MTLBuffer>)mtlBufferToEdit contents];
    return (IUsToEdit) {
        .beg =  beg,
        .end = &beg[iusBuffer->max_count],
        .iu =   beg,
        ._iusBuffer = iusBuffer,
    };
}

void  iustoedit_release(IUsToEdit const edited) {
    IUsBuffer *const iusBuffer = edited._iusBuffer;
    if(!iusBuffer->_editing) { printwarning("Not editing."); return; }
    iusBuffer->_editing = false;
    size_t newCount = edited.iu - edited.beg;
    if(newCount > iusBuffer->max_count) {
        printerror("Overflow buffer size. newCount = %zu.", newCount);
        newCount = iusBuffer->max_count;
    }
    size_initConst(&iusBuffer->actual_count, newCount);
    size_initConst(&iusBuffer->actual_size, newCount*sizeof(InstanceUniforms));
    // (Cas simple array de IU, rien à faire)
    if(!iusBuffer->_mtlBufferOptA)
        return;
    bool const AisLive = iusBuffer->mtlBufferOpt == iusBuffer->_mtlBufferOptA;
    void const*const mtlBufferLive = AisLive ?
        iusBuffer->_mtlBufferOptA : iusBuffer->_mtlBufferOptB;
    void const*const mtlBufferEdited = AisLive ?
        iusBuffer->_mtlBufferOptB : iusBuffer->_mtlBufferOptA;
    if(edited.init) {
        // Si init du buffer on copie le buffer édité dans l'autre copie.
        InstanceUniforms* const src = [(__bridge id<MTLBuffer>)mtlBufferEdited contents];
        InstanceUniforms* const dst = [(__bridge id<MTLBuffer>)mtlBufferLive contents];
        size_t const bufferSize = iusBuffer->max_count * sizeof(InstanceUniforms);
        memcpy(dst, src, bufferSize);
    }
    // Swap (celui qui vient d'être édité sera le prochain "live")
    *(const void**)&iusBuffer->mtlBufferOpt = mtlBufferEdited;
}

