//
//  graph_mesh_apple.m
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-29.
//

#import <Foundation/Foundation.h>

#include "graph__metal.h"
#include "graph_mesh_private.h"
#include "util_base.h"

// MARK: - Metal method pour les meshes
#define Mesh_verticesSecond (float*)((char*)mesh->_vertices + mesh->_verticesSize)

void     mesh_engine_initBuffers_(Mesh* const mesh, const void* const verticesOpt, const uint16_t* const indicesOpt) {
    if(mesh->_flags & mesh_flag__bufferInit) {
        printwarning("Vertices buffer already init.");
        return;
    }
    id<MTLDevice> device = CoqGraph_metal_device;
    if(!device) { printerror("Device not init."); return; }
    // Buffer des indices
    if(mesh->maxIndexCount && indicesOpt) {
        size_t indicesSize = mesh->maxIndexCount * sizeof(uint16_t);
        id<MTLBuffer> mtlIndicesBuffer = [device newBufferWithBytes:indicesOpt
                                                      length:indicesSize options:0];
        *(const void**)&mesh->_mtlIndicesBufferOpt_cptr = CFBridgingRetain(mtlIndicesBuffer);
    } else if(indicesOpt || mesh->maxIndexCount) {
        printwarning("Missing indices array or indexCount.");
    }
    // Utiliser un buffer pour les vertices...
    if(mesh->_flags & mesh_flag__withVerticesBuffer) {
        id<MTLBuffer> mtlVerticesBuffer;
        if(verticesOpt) {
            mtlVerticesBuffer = [device newBufferWithBytes:verticesOpt 
                                        length:mesh->_verticesSize options:0];
        } else {
            mtlVerticesBuffer = [device newBufferWithLength:mesh->_verticesSize options:0];
        }
        *(const void**)&mesh->_mtlVerticesBufferOpt_cptr = CFBridgingRetain(mtlVerticesBuffer);
    }
    // Deuxième vertices buffer ?
    if(mesh->_flags & mesh_flag__withDoubleVertBuffer) {
        id<MTLBuffer> mtlVerticesBuffer;
        if(verticesOpt) {
            mtlVerticesBuffer = [device newBufferWithBytes:verticesOpt 
                                        length:mesh->_verticesSize options:0];
        } else {
            mtlVerticesBuffer = [device newBufferWithLength:mesh->_verticesSize options:0];
        }
        *(const void**)&mesh->_mtlVerticesBuffer2Opt_cptr = CFBridgingRetain(mtlVerticesBuffer);
    }
    mesh->_flags |= mesh_flag__bufferInit;
}
void     mesh_engine_deinit_(Mesh* const mesh) {
    if(!(mesh->_flags & mesh_flag__bufferInit)) { return; }
    if(mesh->_mtlVerticesBufferOpt_cptr) {
        CFRelease(mesh->_mtlVerticesBufferOpt_cptr); 
        *(const void**)&mesh->_mtlVerticesBufferOpt_cptr = NULL;
    }
    if(mesh->_mtlVerticesBuffer2Opt_cptr) {
        CFRelease(mesh->_mtlVerticesBuffer2Opt_cptr); 
        *(const void**)&mesh->_mtlVerticesBuffer2Opt_cptr = NULL;
    }
    if(mesh->_mtlIndicesBufferOpt_cptr) {
        CFRelease(mesh->_mtlIndicesBufferOpt_cptr); 
        *(const void**)&mesh->_mtlIndicesBufferOpt_cptr = NULL;
    }
}
void     mesh_render_tryToUpdateVerticesAndIndiceCount(Mesh *const mesh) {
    // Besoin de mise à jour ?
    if((mesh->_flags & (mesh_flag_mutable|mesh_flag__needUpdate)) != (mesh_flag_mutable|mesh_flag__needUpdate)) return;
    // Cas pas de buffer
    if(mesh->_flags & mesh_flag__withDoubleVertices) {
        if(!mesh->_verticesReadOpt) { printerror("No second vertices."); return; }
        memcpy(mesh->_verticesReadOpt, mesh->_verticesEdit, mesh->_verticesSize);
    }
    // Cas avec buffer
    else {
        if(!mesh->_mtlVerticesBuffer2Opt_cptr || !mesh->_mtlVerticesBufferOpt_cptr) {
            printerror("Missing vertices buffer."); return;
        }
        // Copie dans le buffer non actif et swap.
        id<MTLBuffer> mtlVerticesBuffer;
        if(mesh->_flags & mesh_flag__firstActive) {
            mtlVerticesBuffer= (__bridge id<MTLBuffer>)mesh->_mtlVerticesBuffer2Opt_cptr;
        } else {
            mtlVerticesBuffer= (__bridge id<MTLBuffer>)mesh->_mtlVerticesBufferOpt_cptr;
        }
        memcpy([mtlVerticesBuffer contents], mesh->_verticesEdit, mesh->_verticesSize);
        mesh->_flags ^= mesh_flag__firstActive;
    }
    if(mesh->_newIndexCountOpt) {
        uint_initConst(&mesh->actualIndexCount, uminu(mesh->_newIndexCountOpt, mesh->maxIndexCount));
        mesh->_newIndexCountOpt = 0;
    }
    // Ok, fini d'updater
    mesh->_flags &= ~mesh_flag__needUpdate; 
}

MeshToDraw mesh_render_getMeshToDraw(Mesh const*const mesh) {
    void const* verticesBuffer_cptr;
    if(mesh->_mtlVerticesBuffer2Opt_cptr && !(mesh->_flags & mesh_flag__firstActive))
        verticesBuffer_cptr = mesh->_mtlVerticesBuffer2Opt_cptr;
    else
        verticesBuffer_cptr = mesh->_mtlVerticesBufferOpt_cptr;
    return (MeshToDraw) {
        .vertexCount =    mesh->vertexCount,
        .verticesSize =   mesh->_verticesSize,
        .indexCount =     mesh->actualIndexCount,
        .primitive_type = mesh->primitive_type,
        .cull_mode =      mesh->cull_mode,
        .verticesOpt =       mesh->_verticesReadOpt,
        .metal_verticesBufferOpt_cptr = verticesBuffer_cptr,
        .metal_indicesBufersOpt_cptr =  mesh->_mtlIndicesBufferOpt_cptr,
    };
}
