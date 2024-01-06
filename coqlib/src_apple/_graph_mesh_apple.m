//
//  graph_mesh_apple.m
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-29.
//

#import <Foundation/Foundation.h>

#include "_graph__apple.h"

typedef struct _Mesh {
    size_t    vertices_size;      // La taille en bytes de l'array vertices.
    uint32_t  vertex_count;       // Le nombre de vertex.
    uint32_t  index_count;        // 0 si triangle strip (pas besoin d'indices de vertex).
    uint16_t  primitive_type;
    uint16_t  cull_mode;
    bool      isShared;
    id<MTLBuffer> indicesBuffer;   // Buffer Metal (optionel). Suffisant, pas besoin du indicesOpt.
    //    id<MTLBuffer>    verticesBuffer; // Besoin ?
    //    uint16_t* indicesOpt;    //  Pas besoin...
//    Vertex*    vertices;
    Vertex    vertices[1];         // Array des vertex. A LA FIN, fait varier la taille.
} Mesh;

static id<MTLDevice> _mesh_device = nil;

static Vertex _mesh_sprite_vertices[4] = {
    {-0.5, 0.5, 0, 0,0, 0,0,1},
    {-0.5,-0.5, 0, 0,1, 0,0,1},
    { 0.5, 0.5, 0, 1,0, 0,0,1},
    { 0.5,-0.5, 0, 1,1, 0,0,1},
};
Mesh*  mesh_sprite = NULL;

void Mesh_init(id<MTLDevice> const device) {
    _mesh_device = device;
    
    // Init de la sprite.
    mesh_sprite = Mesh_createEmpty(_mesh_sprite_vertices, 4, NULL, 0,
          mesh_primitive_triangleStrip, mesh_cullMode_none, true);
}

Mesh*  Mesh_createEmpty(const Vertex* const verticesOpt, uint32_t vertexCount,
                        const uint16_t* const indicesOpt, uint32_t indexCount,
                        enum MeshPrimitiveType primitive_type,
                        enum MeshCullMode cull_mode, bool isShared) {
    if(_mesh_device == nil) {
        printerror("Mesh not init.");
        return NULL;
    }
    size_t mesh_size = sizeof(Mesh) + sizeof(Vertex) * (vertexCount - 1);
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
        mesh->indicesBuffer = [_mesh_device
                               newBufferWithBytes:indicesOpt
                               length:indicesSize options:0];
    } else {
        if(indicesOpt || indexCount)
            printwarning("Missing indices array or indexCount.");
        mesh->indicesBuffer = nil;
    }
    if(verticesOpt) {
        memcpy(mesh->vertices, verticesOpt, mesh->vertices_size);
    }
    // Besoin de buffer pour les vertices ? Non a priori...
//    if(bufferedVertices) {
//        mesh->verticesBuffer = [_mesh_device
//                                 newBufferWithBytes:mesh->bm.vertices
//                                 length:mesh->bm.vertices_size options:0];
//    }
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
Vertex*                mesh_vertices(Mesh* mesh) {
    return mesh->vertices;
}

id<MTLBuffer>  mesh_MTLIndicesBuffer(Mesh* mesh) {
    return mesh->indicesBuffer;
}
uint32_t mesh_indexCount(Mesh* mesh) {
    return mesh->index_count;
}

void   mesh_destroy(Mesh* meshToDelete) {
    meshToDelete->indicesBuffer = NULL;
//    meshToDelete->verticesBuffer = nil;
    coq_free(meshToDelete);  // (free aussi les vertices)
}
