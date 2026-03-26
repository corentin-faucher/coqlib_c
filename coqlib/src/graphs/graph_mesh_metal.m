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

void     mesh_render_deinit_(Mesh* const mesh) {
    if(mesh->indicesOpt) {
        coq_free(mesh->indicesOpt);
        mesh->indicesOpt = NULL;
    }
    if(mesh->mtl_verticesBuffer0Opt) {
        CFRelease(mesh->mtl_verticesBuffer0Opt); 
        *(const void**)&mesh->mtl_verticesBuffer0Opt = NULL;
    }
    if(mesh->mtl_verticesBuffer1Opt) {
        CFRelease(mesh->mtl_verticesBuffer1Opt); 
        *(const void**)&mesh->mtl_verticesBuffer1Opt = NULL;
    }
    if(mesh->mtl_indicesBufferOpt) {
        CFRelease(mesh->mtl_indicesBufferOpt); 
        *(const void**)&mesh->mtl_indicesBufferOpt = NULL;
    }
}

void       mesh_render_checkMeshInit(Mesh* mesh) {
    if(mesh->mtl_bufferInit) return;
    mesh->mtl_bufferInit = true;
    // Buffer des indices
    if(mesh->maxIndexCount && mesh->indicesOpt) {
        size_t indicesSize = mesh->maxIndexCount * sizeof(uint16_t);
        id<MTLBuffer> mtlIndicesBuffer = 
            [CoqMtl_device newBufferWithBytes:mesh->indicesOpt
                                length:indicesSize options:0];
        *(const void**)&mesh->mtl_indicesBufferOpt = CFBridgingRetain(mtlIndicesBuffer);
        coq_free(mesh->indicesOpt);
        mesh->indicesOpt = NULL;
    } 
    else if(mesh->indicesOpt || mesh->maxIndexCount) {
        printwarning("Missing indices array or indexCount.");
    }
    // (pass si pas besoin de buffer)
    if(mesh->mtl_vertices1Opt) return;
    // Utiliser un Metal buffer pour les vertices...
    id<MTLBuffer> mtlVerticesBuffer0 = 
        [CoqMtl_device newBufferWithBytes:mesh->vertices0 
                               length:mesh->verticesSize options:0];
    *(const void**)&mesh->mtl_verticesBuffer0Opt = CFBridgingRetain(mtlVerticesBuffer0);
    if(mesh->flags & mesh_flag_mutable) {
        id<MTLBuffer> mtlVerticesBuffer1 = 
            [CoqMtl_device newBufferWithBytes:mesh->vertices0 
                                length:mesh->verticesSize options:0];
        *(const void**)&mesh->mtl_verticesBuffer1Opt = CFBridgingRetain(mtlVerticesBuffer1);
    }
}

void       mesh_render_checkMeshUpdate(Mesh* mesh) {
    if(!mesh->verticesEdited) return;
    float*const edited = mesh->verticesEdited;
    mesh->verticesEdited = NULL;
    if(!(mesh->flags & mesh_flag_mutable)) {
        printerror("Non mutable mesh with edited vertices.");
        return;
    }
    if(mesh->newIndexCountOpt) {
        uint_initConst(&mesh->actualIndexCount, uminu(mesh->newIndexCountOpt, mesh->maxIndexCount));
        mesh->newIndexCountOpt = 0;
    }
    if(!mesh->mtl_bufferInit) {
        printwarning("Buffer still not init?");
        return;
    }
    // Cas pas de buffer
    if(mesh->mtl_vertices1Opt) {
        // (pass, c'est beau comme ça, les vertex sont prêts pour être lu.)
        goto swap_01;
    }
    // Cas avec buffer
    if(!mesh->mtl_verticesBuffer1Opt || !mesh->mtl_verticesBuffer0Opt) {
        printerror("Missing vertices buffer.");
        return;
    }
    // Copie dans le buffer non actif et swap.
    memcpy([mesh->mtl_readBuffer1 ? 
                (__bridge id<MTLBuffer>)mesh->mtl_verticesBuffer0Opt :
                (__bridge id<MTLBuffer>)mesh->mtl_verticesBuffer1Opt contents], 
           edited, mesh->verticesSize);
swap_01:
    mesh->mtl_readBuffer1 = !mesh->mtl_readBuffer1;
}

MeshToDraw mesh_render_getToDraw(Mesh const* mesh) {
    void const* verticesToReadOpt = NULL;
    void const* verticesBufferToReadOpt = mesh->mtl_verticesBuffer0Opt;
    if(mesh->mtl_vertices1Opt) {
        verticesToReadOpt = mesh->mtl_readBuffer1 ?
            mesh->mtl_vertices1Opt : mesh->vertices0; 
    } else {
        if(mesh->mtl_verticesBuffer1Opt && mesh->mtl_readBuffer1) {
            verticesBufferToReadOpt = mesh->mtl_verticesBuffer1Opt;
        }
        if(!verticesBufferToReadOpt) printerror("No vertices buffer.");
    }
    return (MeshToDraw) {
        .vertexCount =    mesh->vertexCount,
        .verticesSize =   mesh->verticesSize,
        .indexCount =     mesh->actualIndexCount,
        .cull_mode =      mesh->cull_mode,
        .metal_verticesOpt =          verticesToReadOpt,
        .metal_verticesMTLBufferOpt = verticesBufferToReadOpt,
        .metal_indicesMTLBufersOpt =  mesh->mtl_indicesBufferOpt,
        .metal_primitiveType = mesh->primitive_type, // (correspondance directe pour Metal)
    };
}

