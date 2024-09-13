//
//  graph_mesh_apple.m
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-29.
//

#import <Foundation/Foundation.h>

#include "graph__apple.h"
#include "util_base.h"

#pragma mark - Metal method pour les meshes ------------------------------------------------------

/// Référence des vertices pour mise à jour des positions.
/// Caller `mesh_releaseVertices` une fois fini.
Vertex*  mesh_engine_retainVertices(Mesh* mesh) {
    id<MTLBuffer> mtlVerticesBuffer = (__bridge id<MTLBuffer>)mesh->_mtlVerticesBuffer_cptr;
    return [mtlVerticesBuffer contents];
    // (Avec Metal, on édite directement l'array de vertices)
}
/// Fini d'éditer les vertex.
void     mesh_engine_releaseVertices(Mesh* mesh) {
    // pass (seulement pour OpenGL)
}

void     mesh_engine_initBuffers_(Mesh* const mesh, const Vertex* const verticesOpt, const uint16_t* const indicesOpt) {
    if(mesh->_mtlVerticesBuffer_cptr) {
        printerror("Vertices buffer already init.");
        return;
    }
    id<MTLDevice> device = CoqGraph_getMTLDevice();
    if(!device) { printerror("Device not init."); return; }
    if(mesh->index_count && indicesOpt) {
        size_t indicesSize = mesh->index_count * sizeof(uint16_t);
        id<MTLBuffer> mtlIndicesBuffer = [device newBufferWithBytes:indicesOpt
                                                      length:indicesSize options:0];
        *(const void**)&mesh->_mtlIndicesBufferOpt_cptr = CFBridgingRetain(mtlIndicesBuffer);
    } else {
        if(indicesOpt || mesh->index_count)
            printwarning("Missing indices array or indexCount.");
        *(const void**)&mesh->_mtlIndicesBufferOpt_cptr = NULL;
    }
    id<MTLBuffer> mtlVerticesBuffer;
    size_t const verticesSize = mesh->vertex_count * sizeof(Vertex);
    if(verticesOpt) {
        mtlVerticesBuffer = [device newBufferWithBytes:verticesOpt 
                                        length:verticesSize options:0];
    } else {
        mtlVerticesBuffer = [device newBufferWithLength:verticesSize options:0];
    }
    *(const void**)&mesh->_mtlVerticesBuffer_cptr = CFBridgingRetain(mtlVerticesBuffer);
}
void     mesh_deinit(Mesh* const mesh) {
    CFRelease(mesh->_mtlVerticesBuffer_cptr); 
    *(const void**)&mesh->_mtlVerticesBuffer_cptr = NULL;
    if(mesh->_mtlIndicesBufferOpt_cptr) {
        CFRelease(mesh->_mtlIndicesBufferOpt_cptr); 
        *(const void**)&mesh->_mtlIndicesBufferOpt_cptr = NULL;
    }
}

id<MTLBuffer>  mesh_MTLIndicesBufferOpt(Mesh* mesh) {
    return (__bridge id<MTLBuffer>)mesh->_mtlIndicesBufferOpt_cptr;
}
id<MTLBuffer>  mesh_MTLVerticesBuffer(Mesh* mesh) {
    return (__bridge id<MTLBuffer>)mesh->_mtlVerticesBuffer_cptr;
}

