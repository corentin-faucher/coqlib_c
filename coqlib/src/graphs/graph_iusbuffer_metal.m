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

typedef struct IUsBuffer {
    size_t const       max_count;
    size_t             actual_count;
    size_t const       iusSize;
    bool               editing, initEdit, AisLive;
    const void* const  mtlBufferOptA_cptr;  // Buffer Metal
    const void* const  mtlBufferOptB_cptr;
    InstanceUniforms   ius[1];
} IUsBuffer;

/// Création du buffer.
IUsBuffer* IUsBuffer_create(uint32_t maxCount, InstanceUniforms const*const defaultIUOpt) {
    uint32_t const actualCount = maxCount;
    if(maxCount < 2) {
        if(IUsBuffer_warningMaxCount) printwarning("IUsBuffer with maxCount < 2.");
        maxCount = 2;
    }
    size_t const iusSize = maxCount * sizeof(InstanceUniforms);
    bool const withBuffer = iusSize >= IUsBuffer_sizeForBuffer;
    IUsBuffer *const iusbuffer = withBuffer ? coq_callocTyped(IUsBuffer) :
        coq_callocArray(IUsBuffer, InstanceUniforms, maxCount); // (Sinon juste dans ius)
    size_initConst(&iusbuffer->max_count, maxCount);
    size_initConst(&iusbuffer->iusSize, iusSize);
    iusbuffer->actual_count = actualCount;
    InstanceUniforms *iuA, *iuB;
    if(withBuffer) {
        id<MTLBuffer> mtlBufferA = [CoqGraph_metal_device newBufferWithLength:iusSize
                                    options:MTLResourceCPUCacheModeDefaultCache];
        id<MTLBuffer> mtlBufferB = [CoqGraph_metal_device newBufferWithLength:iusSize
                                    options:MTLResourceCPUCacheModeDefaultCache];
        *(const void**)&iusbuffer->mtlBufferOptA_cptr = CFBridgingRetain(mtlBufferA);
        *(const void**)&iusbuffer->mtlBufferOptB_cptr = CFBridgingRetain(mtlBufferB);
        iuA = [mtlBufferA contents];
        iuB = [mtlBufferB contents];
        mtlBufferA = nil;
        mtlBufferB = nil;
    } else {
        iuA = iusbuffer->ius;
        iuB = NULL;
    }
    // Fill
    if(defaultIUOpt) {
        InstanceUniforms *const end = &iuA[maxCount];
        for(InstanceUniforms* iu = iuA; iu < end; iu++) { *iu = *defaultIUOpt; }
        if(iuB) memcpy(iuB, iuA, iusSize);
    }
    return iusbuffer;
}
/// Libère l'espace du buffer (et array de piu si nécessaire)
void   iusbufferref_releaseAndNull(IUsBuffer** iusref) {
    if(!iusref) { printerror("No ius ref."); return; }
    IUsBuffer *const ius = *iusref;
    *iusref = NULL;
    if(!ius) return; // (ok pas grave, déjà release)
    if(ius->mtlBufferOptA_cptr) {
        CFRelease(ius->mtlBufferOptA_cptr);
        *(const void**)&ius->mtlBufferOptA_cptr = NULL;
    }
    if(ius->mtlBufferOptB_cptr) {
        CFRelease(ius->mtlBufferOptB_cptr);
        *(const void**)&ius->mtlBufferOptB_cptr = NULL;
    }
    coq_free(ius);
}

IUsToEdit   iusbuffer_retainIUsToInit(IUsBuffer *iusBuffer) {
    if(iusBuffer->editing) { printwarning("Already editing."); }
    iusBuffer->editing = true;
    iusBuffer->initEdit = true;
    if(!iusBuffer->mtlBufferOptA_cptr)
        return (IUsToEdit) {
            .beg =  iusBuffer->ius,
            .end = &iusBuffer->ius[iusBuffer->max_count],
            .iu =      iusBuffer->ius,
            ._iusBuffer = iusBuffer,
        };
    InstanceUniforms *const beg = [(__bridge id<MTLBuffer>)iusBuffer->mtlBufferOptA_cptr contents];
    return (IUsToEdit) {
        .beg =  beg,
        .end = &beg[iusBuffer->max_count],
        .iu =       beg,
        ._iusBuffer = iusBuffer,
    };
}
IUsToEdit iusbuffer_rendering_retainIUsToEdit(IUsBuffer *const iusBuffer) {
    if(iusBuffer->editing) { printerror("Already editing."); return (IUsToEdit) {}; }
    iusBuffer->editing = true;
    if(!iusBuffer->mtlBufferOptA_cptr)
        return (IUsToEdit) {
            .beg =  iusBuffer->ius,
            .end = &iusBuffer->ius[iusBuffer->max_count],
            .iu =      iusBuffer->ius,
            ._iusBuffer = iusBuffer,
        };
    // On édite celui qui n'est pas actif...
    id<MTLBuffer> mtlBuffer = (__bridge id<MTLBuffer>)(
        (iusBuffer->AisLive) ? iusBuffer->mtlBufferOptB_cptr : iusBuffer->mtlBufferOptA_cptr 
    );
    InstanceUniforms *const beg = [mtlBuffer contents];
    return (IUsToEdit) {
        .beg =  beg,
        .end = &beg[iusBuffer->max_count],
        .iu =      beg,
        ._iusBuffer = iusBuffer,
    };
}
// Ok, finit d'éditer, prêt pour drawing.
void  iustoedit_release(IUsToEdit const edited) {
    IUsBuffer *const iusBuffer = edited._iusBuffer;
    if(!iusBuffer->editing) { printwarning("Not editing."); return; }
    iusBuffer->editing = false;
    size_t newCount = edited.iu - edited.beg;
    if(newCount > iusBuffer->max_count) {
        printerror("Overflow buffer size. newCount = %zu.", newCount);
        newCount = iusBuffer->max_count;
    }
    iusBuffer->actual_count = newCount;
    if(!iusBuffer->mtlBufferOptA_cptr)
        return;
    if(iusBuffer->initEdit) {
        iusBuffer->initEdit = false;
        // A l'init copier A dans B.
        id<MTLBuffer> mtlBufferA = (__bridge id<MTLBuffer>)iusBuffer->mtlBufferOptA_cptr;
        id<MTLBuffer> mtlBufferB = (__bridge id<MTLBuffer>)iusBuffer->mtlBufferOptB_cptr;
        InstanceUniforms* const iuA = [mtlBufferA contents];
        InstanceUniforms* const iuB = [mtlBufferB contents];
        memcpy(iuB, iuA, iusBuffer->iusSize);
    }
    // Swap (celui qui vient d'être édité sera le prochain "live")
    iusBuffer->AisLive = !iusBuffer->AisLive;
}

IUsToDraw   iusbuffer_rendering_getToDraw(IUsBuffer const* iusBuffer) {
    if(iusBuffer->editing) {
        printerror("Editing not released.");
        return (IUsToDraw) { };
    }
    if(!iusBuffer->mtlBufferOptA_cptr) {
        return (IUsToDraw) {
            .count =  iusBuffer->actual_count,
            .size =   iusBuffer->actual_count * sizeof(InstanceUniforms),
            .iusOpt = iusBuffer->ius,
        };
    } else {
        return (IUsToDraw) {
            .count = iusBuffer->actual_count,
            .size =  iusBuffer->actual_count * sizeof(InstanceUniforms),
            .metal_bufferOpt = iusBuffer->AisLive ? iusBuffer->mtlBufferOptA_cptr : iusBuffer->mtlBufferOptB_cptr,
        };
    }
}
