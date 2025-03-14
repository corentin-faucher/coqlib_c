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
    // Le rest est "privé"
    uint32_t const _vertexSize;       // Taille d'un vertex (par défault sizeof(Vertex), voir plus bas)
    size_t   const _verticesSize;     // vertexSize * vertexCount
    uint32_t       _flags;
    uint32_t       _newIndexCountOpt; // Lors d'une mise à jour de vertex, le nouveau compte de vertex.
//    float *const   _verticesReadOpt;  // Les vertices pour la lecture (si on n'utilise pas _mtlVerticesBufferOpt_cptr)
    union {
        // Metal
        struct {
            const void* const _mtlIndicesBufferOpt_cptr;
            const void* const _mtlVerticesBufferOpt_cptr;
            const void* const _mtlVerticesBuffer2Opt_cptr;
        };
        // OpenGl
        struct {
            uint32_t _glVertexArrayId;   // VAO
            uint32_t _glVertexBufferId;  // VBO
            uint32_t _glIndicesBufferId; // VBO des indices
        };
    };
    // Liste des vertex gardé en mémoire pour l'édition (si mutable, si non mutable -> vide)
    float *const _verticesReadOpt; // Deuxième moitié de vertices (cas doubleVertices)
    float        _verticesEdit[1];
} Mesh;

#endif
