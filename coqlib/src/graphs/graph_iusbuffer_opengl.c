//
//  graph_iusbuffer_opengl.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2025-03-11.
//

#include "graph_iusbuffer.h"
#include "../coq__buildConfig.h"
#include "../utils/util_base.h"

typedef struct IUsBuffer {
    size_t const       max_count;
    size_t             actual_count;
    size_t const       iusSize;
    bool               editing;
//    const void* const  mtlBufferOptA_cptr;  // Buffer Metal
//    const void* const  mtlBufferOptB_cptr;
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
    IUsBuffer *const iusbuffer = coq_callocArray(IUsBuffer, InstanceUniforms, maxCount);
    size_initConst(&iusbuffer->max_count, maxCount);
    size_initConst(&iusbuffer->iusSize, iusSize);
    iusbuffer->actual_count = actualCount;
    InstanceUniforms *const end = &iusbuffer->ius[maxCount];
    if(defaultIUOpt) for(InstanceUniforms* iu = iusbuffer->ius; iu < end; iu++) {
        *iu = *defaultIUOpt;
    }
    return iusbuffer;
}
/// Libère l'espace du buffer (et array de piu si nécessaire)
void   iusbufferref_releaseAndNull(IUsBuffer** iusref) {
    if(!iusref) { printerror("No ius ref."); return; }
    IUsBuffer *const ius = *iusref;
    *iusref = NULL;
    coq_free(ius);
}

IUsToEdit   iusbuffer_retainIUsToInit(IUsBuffer *iusBuffer) {
    if(iusBuffer->editing) { printwarning("Already editing."); }
    iusBuffer->editing = true;
//    iusBuffer->initEdit = true;
    return (IUsToEdit) {
        .beg =  iusBuffer->ius,
        .end = &iusBuffer->ius[iusBuffer->max_count],
        .iu =   iusBuffer->ius,
        ._iusBuffer = iusBuffer,
    };
}
IUsToEdit iusbuffer_rendering_retainIUsToEdit(IUsBuffer *const iusBuffer) {
    if(iusBuffer->editing) { printerror("Already editing."); return (IUsToEdit) {}; }
    iusBuffer->editing = true;
    return (IUsToEdit) {
        .beg =  iusBuffer->ius,
        .end = &iusBuffer->ius[iusBuffer->max_count],
        .iu =   iusBuffer->ius,
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
}

IUsToDraw   iusbuffer_rendering_getToDraw(IUsBuffer const* iusBuffer) {
    if(iusBuffer->editing) {
        printerror("Editing not released.");
        return (IUsToDraw) { };
    }
    return (IUsToDraw) {
        .count =  iusBuffer->actual_count,
        .size =   iusBuffer->actual_count * sizeof(InstanceUniforms),
        .iusOpt = iusBuffer->ius,
    };
}
