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
    if(mesh->flags & mesh_flag_metal_bufferInit) {
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
        *(const void**)&mesh->indicesMTLBufferOpt = CFBridgingRetain(mtlIndicesBuffer);
    } else if(indicesOpt || mesh->maxIndexCount) {
        printwarning("Missing indices array or indexCount.");
    }
    // Utiliser un Metal buffer pour les vertices...
    if(mesh->flags & mesh_flag_metal_useVerticesMTLBuffer) {
        id<MTLBuffer> mtlVerticesBuffer;
        if(verticesOpt) {
            mtlVerticesBuffer = [device newBufferWithBytes:verticesOpt 
                                        length:mesh->verticesSize options:0];
        } else {
            mtlVerticesBuffer = [device newBufferWithLength:mesh->verticesSize options:0];
        }
        *(const void**)&mesh->verticesMTLBufferOpt = CFBridgingRetain(mtlVerticesBuffer);
    }
    // Deuxième vertices buffer ?
    if(mesh->flags & mesh_flag_metal_useVerticesDoubleMTLBuffer) {
        id<MTLBuffer> mtlVerticesBuffer;
        if(verticesOpt) {
            mtlVerticesBuffer = [device newBufferWithBytes:verticesOpt 
                                        length:mesh->verticesSize options:0];
        } else {
            mtlVerticesBuffer = [device newBufferWithLength:mesh->verticesSize options:0];
        }
        *(const void**)&mesh->vertices2MTLBufferOpt = CFBridgingRetain(mtlVerticesBuffer);
    }
    mesh->flags |= mesh_flag_metal_bufferInit;
}
void     mesh_engine_deinit_(Mesh* const mesh) {
    if(mesh->verticesMTLBufferOpt) {
        CFRelease(mesh->verticesMTLBufferOpt); 
        *(const void**)&mesh->verticesMTLBufferOpt = NULL;
    }
    if(mesh->vertices2MTLBufferOpt) {
        CFRelease(mesh->vertices2MTLBufferOpt); 
        *(const void**)&mesh->vertices2MTLBufferOpt = NULL;
    }
    if(mesh->indicesMTLBufferOpt) {
        CFRelease(mesh->indicesMTLBufferOpt); 
        *(const void**)&mesh->indicesMTLBufferOpt = NULL;
    }
    mesh->flags &= ~mesh_flag_metal_bufferInit;
}
void     mesh_render_tryToUpdateVerticesAndIndiceCount(Mesh *const mesh) {
    // Besoin de mise à jour ?
    if((mesh->flags & (mesh_flag_mutable|mesh_flag__needUpdate)) != (mesh_flag_mutable|mesh_flag__needUpdate)) return;
    // Cas pas de buffer
    if(mesh->flags & mesh_flag_metal_mutableDoubleVertices) {
        if(!mesh->verticesReadOpt) { printerror("No second vertices."); return; }
        memcpy(mesh->verticesReadOpt, mesh->verticesEdit, mesh->verticesSize);
    }
    // Cas avec buffer
    else {
        if(!mesh->vertices2MTLBufferOpt || !mesh->verticesMTLBufferOpt) {
            printerror("Missing vertices buffer."); return;
        }
        // Copie dans le buffer non actif et swap.
        id<MTLBuffer> mtlVerticesBuffer;
        if(mesh->flags & mesh_flag_metal_isFirstVertMTLBufferActive) {
            mtlVerticesBuffer= (__bridge id<MTLBuffer>)mesh->vertices2MTLBufferOpt;
        } else {
            mtlVerticesBuffer= (__bridge id<MTLBuffer>)mesh->verticesMTLBufferOpt;
        }
        memcpy([mtlVerticesBuffer contents], mesh->verticesEdit, mesh->verticesSize);
        mesh->flags ^= mesh_flag_metal_isFirstVertMTLBufferActive;
    }
    if(mesh->newIndexCountOpt) {
        uint_initConst(&mesh->actualIndexCount, uminu(mesh->newIndexCountOpt, mesh->maxIndexCount));
        mesh->newIndexCountOpt = 0;
    }
    // Ok, fini d'updater
    mesh->flags &= ~mesh_flag__needUpdate; 
}

MeshToDraw mesh_render_getMeshToDraw(Mesh const*const mesh) {
    void const* verticesMTLBuffer;
    if(mesh->vertices2MTLBufferOpt && !(mesh->flags & mesh_flag_metal_isFirstVertMTLBufferActive))
        verticesMTLBuffer = mesh->vertices2MTLBufferOpt;
    else
        verticesMTLBuffer = mesh->verticesMTLBufferOpt;
    return (MeshToDraw) {
        .vertexCount =    mesh->vertexCount,
        .verticesSize =   mesh->verticesSize,
        .indexCount =     mesh->actualIndexCount,
//        .primitive_type = mesh->primitive_type,
        .cull_mode =      mesh->cull_mode,
        .metal_verticesOpt =          mesh->verticesReadOpt,
        .metal_verticesMTLBufferOpt = verticesMTLBuffer,
        .metal_indicesMTLBufersOpt =  mesh->indicesMTLBufferOpt,
        .metal_primitiveType = mesh->primitive_type, // (correspondance directe pour Metal)
    };
}
