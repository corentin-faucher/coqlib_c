//
//  graph_mesh_apple.m
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-29.
//

#import <Foundation/Foundation.h>

#include "graph__apple.h"
#include "utils/utils_base.h"

static id<MTLDevice> MTL_device_ = nil;

#pragma mark - Testing... Metal Buffer (pour uniforms) -----------------------

typedef struct UniformBuffer {
    id<MTLBuffer> mtlBuffer;
} UniformBuffer;

UniformBuffer* UniformBuffer_create(size_t size) {
    // A priori, on ne fait qu'écrire dans le buffer ?
    UniformBuffer* ub = coq_calloc(1, sizeof(UniformBuffer));
    ub->mtlBuffer = [MTL_device_ newBufferWithLength:size options:MTLResourceOptionCPUCacheModeWriteCombined];
    return ub;
};
void   uniformbuffer_setDataAt(UniformBuffer* ub, const void *newData, size_t size, size_t offset) {
    memcpy([ub->mtlBuffer contents] + offset, newData, size);
}
void   uniformbufferref_destroyAndNull(UniformBuffer** const ubToDeleteRef) {
    if(*ubToDeleteRef == NULL) return;
    (*ubToDeleteRef)->mtlBuffer = nil;
    coq_free(*ubToDeleteRef);
    *ubToDeleteRef = NULL;
}
id<MTLBuffer>  uniformbuffer_MTLBuffer(UniformBuffer* ub) {
    return ub->mtlBuffer;
}

// Ou bien... (semble mieux...)

const void* MTLBuffer_createAndGetCPointer(size_t size) {
    return CFBridgingRetain([MTL_device_ newBufferWithLength:size options:MTLResourceOptionCPUCacheModeWriteCombined]);
}
void mtlbufferCptr_setDataAt(const void* mtlBufferCPtr, const void *newData, size_t size, size_t offset) {
    id<MTLBuffer> mtlBuffer = (__bridge id<MTLBuffer>)(mtlBufferCPtr);
    memcpy([mtlBuffer contents] + offset, newData, size);
}
void mtlbufferCPtrRef_releaseAndNull(const void** mtlBufferRef) {
    if(*mtlBufferRef == NULL) return;
    CFRelease(*mtlBufferRef);
    *mtlBufferRef = NULL;
}
id<MTLBuffer> mtlbufferCPtr_asMTLBuffer(const void* mtlBufferCPtr) {
    return (__bridge id<MTLBuffer>)mtlBufferCPtr;
}

#pragma mark - Mesh ------------------------------------------------------

typedef struct Mesh_ {
    size_t        vertices_size;      // La taille en bytes de l'array vertices.
    uint32_t      vertex_count;       // Le nombre de vertex.
    uint32_t      index_count;        // 0 si triangle strip (pas besoin d'indices de vertex).
    uint16_t      primitive_type;
    uint16_t      cull_mode;
    bool          isShared;
    id<MTLBuffer> indicesBufferOpt;   // Buffer Metal des indices (optionel).
    id<MTLBuffer> verticesBuffer;     // (Suffisant, pas besoin d'array vertices.)
//    Vertex        vertices[1];         // Array des vertex. A LA FIN, fait varier la taille.
} Mesh;



static Vertex _mesh_sprite_vertices[4] = {
    {-0.5, 0.5, 0, 0,0, 0,0,1},
    {-0.5,-0.5, 0, 0,1, 0,0,1},
    { 0.5, 0.5, 0, 1,0, 0,0,1},
    { 0.5,-0.5, 0, 1,1, 0,0,1},
};
Mesh*  mesh_sprite = NULL;

void Mesh_init(id<MTLDevice> const device) {
    MTL_device_ = device;
    
    // Init de la sprite.
    mesh_sprite = Mesh_createEmpty(_mesh_sprite_vertices, 4, NULL, 0,
          mesh_primitive_triangleStrip, mesh_cullMode_none, true);
}

Mesh*  Mesh_createEmpty(const Vertex* const verticesOpt, uint32_t vertexCount,
                        const uint16_t* const indicesOpt, uint32_t indexCount,
                        enum MeshPrimitiveType primitive_type,
                        enum MeshCullMode cull_mode, 
                        bool isShared) {
    if(MTL_device_ == nil) {
        printerror("Mesh not init.");
        return NULL;
    }
    size_t mesh_size = sizeof(Mesh); // + sizeof(Vertex) * (vertexCount - 1);
    size_t vertices_size = vertexCount * sizeof(Vertex);
    Mesh* mesh = coq_calloc(1, mesh_size);
    
    mesh->vertex_count =  vertexCount;
    mesh->vertices_size = vertices_size;
    mesh->index_count =   indexCount;
    mesh->primitive_type = primitive_type;
    mesh->cull_mode = cull_mode;
    mesh->isShared = isShared;
    
    if(indexCount && indicesOpt) {
        size_t indicesSize = indexCount * sizeof(uint16_t);
        mesh->indicesBufferOpt = [MTL_device_
                               newBufferWithBytes:indicesOpt
                               length:indicesSize options:0];
    } else {
        if(indicesOpt || indexCount)
            printwarning("Missing indices array or indexCount.");
        mesh->indicesBufferOpt = nil;
    }
    if(verticesOpt) {
        mesh->verticesBuffer = [MTL_device_ newBufferWithBytes:verticesOpt 
                                        length:mesh->vertices_size options:0];
    } else {
        mesh->verticesBuffer = [MTL_device_ newBufferWithLength:mesh->vertices_size options:0];
    }
    return mesh;
}

uint32_t mesh_vertexCount(Mesh* mesh) {
    return mesh->vertex_count;
}
size_t   mesh_verticesSize(Mesh* mesh) {
    return mesh->vertices_size;
}
enum MeshPrimitiveType mesh_primitiveType(Mesh* mesh) {
    return mesh->primitive_type;
}
enum MeshCullMode      mesh_cullMode(Mesh* mesh) {
    return mesh->cull_mode;
}
bool     mesh_isShared(Mesh* mesh) {
    return mesh->isShared;
}
Vertex*        mesh_vertices(Mesh* mesh) {
    return [mesh->verticesBuffer contents];
    
}
void mesh_needToUpdateVertices(Mesh* mesh) {
    // pass (pas besoin avec Metal, on prend directement l'array de vertices.)
}

id<MTLBuffer>  mesh_MTLIndicesBufferOpt(Mesh* mesh) {
    return mesh->indicesBufferOpt;
}
id<MTLBuffer>  mesh_MTLVerticesBuffer(Mesh* mesh) {
    return mesh->verticesBuffer;
}
uint32_t mesh_indexCount(Mesh* mesh) {
    return mesh->index_count;
}

void   mesh_destroyAndNull(Mesh** const meshToDeleteRef) {
    if(*meshToDeleteRef == NULL) return;
    (*meshToDeleteRef)->indicesBufferOpt = nil;
    (*meshToDeleteRef)->verticesBuffer = nil;
    coq_free(*meshToDeleteRef);  // (free aussi les vertices)
    *meshToDeleteRef = NULL;
}
