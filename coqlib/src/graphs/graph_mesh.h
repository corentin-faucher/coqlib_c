//
//  graph_mesh.h
//  Mesh : ensemble de vertex pour affichage de surfaces.
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef COQ_GRAPH_MESH_H
#define COQ_GRAPH_MESH_H

#include "../maths/math_base.h"

/// - Vertex -
/// Le verteux peut être soit :
///  - avec coord uv et vecteur normal (pos, uv, norm) ;
///  - avec une couleur par vertex (pos, color).
typedef union {
    float f_arr[8];
    struct {
        Vector3 pos;
        Vector2 uv;
        Vector3 norm;
    };
    struct {
        Vector3 pos_;
        float   unused_;
        Vector4 color;
    };
//    struct {
//        float x, y, z;
//        union {
//            struct {
//                float u, v;
//                float nx, ny, nz;
//            };
//            struct {
//                float w;
//                float col_r, col_g, col_b, col_a;
//            };
//        };        
//    };
} Vertex;

/// Vecteur normal usuel (vers les z positifs)
extern const Vector3 mesh_defNorm; // = { 0, 0, 1};

/*-- Mesh ----------------------*/

typedef struct Mesh {
    uint32_t const vertex_count;       // Le nombre de vertex.
    uint32_t const index_count;        // 0 si triangle strip (pas besoin d'indices de vertex).
    uint16_t       primitive_type;
    uint16_t       cull_mode;
    bool const     isShared;          // Le noeud drawable n'est pas l'owner de la mesh (pas besoin de libérer)
    bool const     isColor;           // Vertex avec couleur au lieu de mapping uv de texture et vecteur normal.
    union {
        // Metal
        struct {
            const void* const _mtlIndicesBufferOpt_cptr; // Buffer Metal des indices (optionel).
            const void* const _mtlVerticesBuffer_cptr;   // (Suffisant, pas besoin de garder l'array vertices.)
        };
        // OpenGl
        struct {
            Vertex  *_verticesCopy;      // On garde une copie éditable. (écrit par la thread d'events et lu par la thread d'affichage)
            bool     _needToUpdateVertices;
            uint32_t _glVertexArrayId;   // VAO
            uint32_t _glVertexBufferId;  // VBO
            uint32_t _glIndicesBufferId; // VBO des indices
        };
    };   
} Mesh;

// Type de primitives. Telles que dans Metal, voir MTLRenderCommandEncoder.h.
enum MeshPrimitiveType {
#ifdef WITH_OPENGL
    mesh_primitive_point = 0,
    mesh_primitive_line = 1,
    mesh_primitive_lineStrip = 3,
    mesh_primitive_triangle = 4,
    mesh_primitive_triangleStrip = 5,
#else // METAL
    mesh_primitive_point = 0,
    mesh_primitive_line = 1,
    mesh_primitive_lineStrip = 2,
    mesh_primitive_triangle = 3,
    mesh_primitive_triangleStrip = 4,
#endif
};
// Type de cull mode. Telles que dans Metal, voir MTLRenderCommandEncoder.h.
enum MeshCullMode {
    mesh_cullMode_none = 0,
    mesh_cullMode_front = 1,
    mesh_cullMode_back = 2,
};

/// Init d'une mesh.
void   mesh_init(Mesh* mesh, const Vertex* const verticesOpt, uint32_t vertexCount,
            const uint16_t* const indicesOpt, uint32_t indexCount,
            enum MeshPrimitiveType primitive_type, enum MeshCullMode cull_mode, bool isShared);
#pragma mark - Engine specific (Metal or OpenGL) -----------------
void   mesh_engine_deinit(Mesh* mesh);
/// Référence des vertices pour mise à jour des positions.
/// Caller `mesh_releaseVertices` une fois fini.
Vertex*  mesh_engine_retainVertices(Mesh* mesh);
/// Fini d'éditer les vertex.
void     mesh_engine_releaseVertices(Mesh* mesh);
void     mesh_engine_initBuffers_(Mesh* const mesh, const Vertex* const verticesOpt, const uint16_t* const indicesOpt);

#pragma mark - Global -
/// La "sprite". Mesh partagée par la plupart des surfaces.
/// (Centrée en (0,0) de dimensions (1, 1), i.e. [-0.5, 0.5] x [-0.5, 0.5].)
extern Mesh  mesh_sprite;
// Pour final pass du renderer (Quad [-1, 1] x [-1, 1]).
extern Mesh  mesh_shaderQuad_;
/// Init des meshes de base : `mesh_sprite` et `mesh_shaderQuad_`.
/// (privé -> déjà callé par `CoqGraph_init...`)
void   Mesh_init_(void);

#pragma mark - Meshes usuelles pratiques ------------------------

Mesh*  Mesh_createHorizontalBar(void);
Mesh*  Mesh_createVerticalBar(void);
Mesh*  Mesh_createFrame(void);
Mesh*  Mesh_createFan(void);
void   mesh_fan_update(Mesh* fan, float ratio);
Mesh*  Mesh_createPlot(float* xs, float* ys, uint32_t count, float delta, float ratio);
Mesh*  Mesh_createPlotGrid(float xmin, float xmax, float xR, float deltaX,
                           float ymin, float ymax, float yR, float deltaY,
                           float lineWidthRatio);





#endif /* graph_mesh_h */
