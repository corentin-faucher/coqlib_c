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
            const void* const indicesMTLBufferOpt;
            const void* const verticesMTLBufferOpt;
            const void* const vertices2MTLBufferOpt;
        };
        // OpenGl, IDs des vertex buffer object et vertex array object.
        struct {
            uint32_t _glVertexArrayId;   // VAO
            uint32_t _glVertexBufferId;  // VBO
            uint32_t _glIndicesBufferId; // VBO des indices
            // TODO: Double VertexBuffer comme Metal ? avec `mesh_flag__withDoubleVertBuffer` 
            // pour fluidité lors d'éditions de vertex...
//            uint32_t _glVertexBuffer2Id;  // VBO
        };
    };
    // Liste des vertex gardé en mémoire pour l'édition (si mutable, si non mutable -> vide)
    float *const verticesReadOpt; // Deuxième moitié de vertices (cas doubleVertices)
    float        verticesEdit[1];
} Mesh;

#endif
