//
//  graph_mesh_private.h
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-16.
//
#ifndef COQ_GRAPH_MESH_PRIVATE_H
#define COQ_GRAPH_MESH_PRIVATE_H

#include <stdint.h>
#include <stddef.h>

typedef struct Mesh {
    uint32_t const vertexCount;      // Le nombre de vertex.
    uint32_t const maxIndexCount;     // 0 si triangle strip (pas besoin d'indices de vertex).
    uint32_t const actualIndexCount;  // Nombre de vertex à dessiner. A priori, actualIndexCount == maxIndexCount,
                                      // on dessine tout les vertex dans la liste `indices`.
    uint16_t const primitive_type;
    uint16_t const cull_mode;
    uint32_t const vertexSize;    // Taille d'un vertex (par défault sizeof(Vertex), voir plus bas)
    size_t   const verticesSize;  // vertexSize * vertexCount
    uint32_t       flags;
    uint32_t       newIndexCountOpt; // Lors d'une mise à jour de vertex, le nouveau compte de vertex.
    union {
        // Metal, pointeurs des Objective-C MTLBuffer, 
        // à caster avec `(__bridge id<MTLBuffer>)`.
        struct {
            const void* const mtl_verticesBuffer0Opt;
            const void* const mtl_verticesBuffer1Opt;
            const void* const mtl_indicesBufferOpt;
            float*const       mtl_vertices1Opt;
            bool mtl_bufferInit;
            bool mtl_readBuffer1;
        };
        // OpenGl, IDs des vertex buffer object et vertex array object.
        struct {
            uint32_t glVertexArrayId;   // VAO
            uint32_t glVertexBuffer0Id;  // VBO
//            uint32_t glVertexBuffer1Id; // Double buffer superflu pour OpenGL ?
            uint32_t glIndicesBufferId; // VBO des indices
        };
    };
    // Liste des vertex gardé en mémoire pour l'édition (si mutable, si non mutable -> vide)
//    float *const verticesReadOpt; // Deuxième moitié de vertices (cas doubleVertices)
    uint16_t*    indicesOpt;
    float*       verticesEdited;
//    float*const  vertices1Opt;
    float        vertices0[1];
} Mesh;

extern bool Mesh_engineIsMetal_;

enum {
// Flags "privé"
    mesh_flag__editing =         0x0020, // (privé. En train d'éditer les vertices.)
//    mesh_flag__mutableNoBuffer = 0x0080, // Peu de vertices, pas besoinde buffer pour les vertices.
    mesh_flags__privates =       0x0FF0,
};


#endif
