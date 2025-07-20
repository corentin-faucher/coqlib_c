//
//  graph_iusbuffer_opengl.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2025-03-11.
//

#include "graph_iusbuffer.h"
#include "../coq__buildConfig.h"
#include "../utils/util_base.h"

/// Init du buffer.
void   iusbuffer_init(IUsBuffer* iusbuffer, uint32_t maxCount, InstanceUniforms const* defaultIUOpt)
{
    if(iusbuffer->iusOpt) {
        printerror("Arleady init."); return;
    }
    if(maxCount < 2) {
        if(IUsBuffer_warningMaxCount) printwarning("IUsBuffer with maxCount < 2.");
        maxCount = 2;
    }
    size_t const iusSize = maxCount * sizeof(InstanceUniforms);
    memset(iusbuffer, 0, sizeof(IUsBuffer));
    size_initConst(&iusbuffer->actual_count, maxCount);
    size_initConst(&iusbuffer->actual_size, iusSize);
    size_initConst(&iusbuffer->max_count, maxCount);
    *(InstanceUniforms**)&iusbuffer->iusOpt = coq_callocSimpleArray(maxCount, InstanceUniforms);
    InstanceUniforms*const beg = (InstanceUniforms*)iusbuffer->iusOpt;
    InstanceUniforms*const end = &beg[maxCount];
    if(defaultIUOpt) for(InstanceUniforms* iu = beg; iu < end; iu++) {
        *iu = *defaultIUOpt;
    }
}
/// Libère l'espace du buffer
void   iusbuffer_deinit(IUsBuffer* iusbuffer) 
{
    if(iusbuffer->iusOpt) {
        coq_free((InstanceUniforms*)iusbuffer->iusOpt);
        *(InstanceUniforms**)&iusbuffer->iusOpt = NULL;
    }
}

IUsToEdit iusbuffer_retainIUsToEdit(IUsBuffer *const iusBuffer) {
    if(iusBuffer->_editing) { 
        printwarning("Already editing.");
        return (IUsToEdit) {};
    }
    if(!iusBuffer->iusOpt) { printerror("Buffer not init."); return (IUsToEdit) {}; }
    iusBuffer->_editing = true;
    InstanceUniforms *const beg = (InstanceUniforms*)iusBuffer->iusOpt;
    return (IUsToEdit) {
        .beg =  beg,
        .end = &beg[iusBuffer->max_count],
        .iu =   beg,
        ._iusBuffer = iusBuffer,
    };
}
// Ok, finit d'éditer, prêt pour drawing.
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
}

