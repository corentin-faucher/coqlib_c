//
//  graph_mesh.h
//  Mesh : ensemble de vertex pour affichage de surfaces.
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef COQ_GRAPH_MESH_H
#define COQ_GRAPH_MESH_H

#include "../maths/math_base.h"

// Structure "privée"
typedef struct Mesh Mesh;

// Info pour dessiner par le renderer.
typedef struct MeshToDraw {
    uint32_t vertexCount;
    size_t   verticesSize;
    uint32_t indexCount;
    uint16_t primitive_type;
    uint16_t cull_mode;
    void const* verticesOpt;
    union {
        struct {  // Metal, convertir avec `(__bridge id<MTLBuffer>)`
            void const* metal_verticesBufferOpt_cptr;
            void const* metal_indicesBufersOpt_cptr;
        };
        struct {  // OpenGL index...
            uint32_t glVertexArrayId;   // VAO
            uint32_t glVertexBufferId;  // VBO
            uint32_t glIndicesBufferId; // VBO des indices
        };
    };
} MeshToDraw;
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
// Flags d'une mesh, seulement needUpdate peut changer.
enum {
    // Si shared, `meshref_releaseAndNull` ne dealloc pas la mesh. Il faut caller explicitement `mesh_engine_deinit_` + `free`.
    mesh_flag_shared =          0x0001,  // Pas besoin de libérer lors du release (on n'est pas l'owner).
    mesh_flag_mutable =         0x0002,  // On peut éditer les vertex (si absent init sans le champ `vertices`)
    mesh_flag__editing =        0x0004, // (privé. En train d'éditer les vertices.)
    mesh_flag__needUpdate =     0x0008, // (privé. Besoin de mettre à jour les vertices (doit être mutable))
    mesh_flag__bufferInit =     0x0010, // (privé. Buffer (Metal/OpenGL) de la mesh est setté (mesh init))
    mesh_flag__withDoubleVertices =   0x0020, // Mutable avec double _vertices.
    mesh_flag__withVerticesBuffer =   0x0040,
    mesh_flag__withDoubleVertBuffer = 0x0080,
    mesh_flag__firstActive =    0x0100,
    mesh_flags__privates =      0x01FC,
    mesh_flag_firstCustomFlag = 0x0200,
};
// Structure temporaire pour créer une mesh.
typedef struct MeshInit {
    const void* verticesOpt;
    uint32_t    vertexCount;
    uint32_t    vertexSizeOpt;
    const uint16_t* indicesOpt;
    uint32_t    indexCountOpt;
    enum MeshPrimitiveType primitive_type;
    enum MeshCullMode      cull_mode;
    uint32_t    flags;
} MeshInit;

/// Init d'une mesh. Par défaut la taille d'un vertex est sizeof(Vertex) == 32 bytes. Si taille custom setter vertexSizeOpt.
Mesh*    Mesh_create(MeshInit initInfo);
/// Libérer la mesh (non shared seulement. Pour libérer une shared, le faire manuellement avec `mesh_engine_deinit_`...)
void     meshref_releaseAndNull(Mesh** meshRef);

// MARK: - Getter
/// Obtenir la référence aux vertices pour édition (NULL si non mutable)
void*    mesh_retainVerticesOpt(Mesh* mesh);
/// Fin des vertex éditable (NULL si non mutable)
void*    mesh_verticesEndOpt(Mesh *mesh);
/// Fini d'éditer, flager pour que la thread de rendering met à jour le vertex buffer.
void     mesh_releaseVertices(Mesh *mesh, uint32_t newIndiceCountOpt);

// MARK: - Engine specific (Metal or OpenGL)
/// Update des vertices dans mesh->vertices vers le buffer Metal/OpenGL (pour mesh mutable)
void       mesh_render_tryToUpdateVerticesAndIndiceCount(Mesh *mesh);
MeshToDraw mesh_render_getMeshToDraw(Mesh const* mesh);

/// "privé". Libère les resouces (OpenGL/Metal) de la mesh. Utiliser a priori `meshref_releaseAndNull`.
void     mesh_engine_deinit_(Mesh* mesh);
void     mesh_engine_initBuffers_(Mesh* const mesh, const void* verticesOpt, const uint16_t* const indicesOpt);

// MARK: - Meshes Globales
/// Init de la "sprite". Mesh partagée par la plupart des Drawable Node...
/// Customizable, a priori centrée en (0,0) de dimensions (1, 1), i.e. [-0.5, 0.5] x [-0.5, 0.5].
/// Pour second/final pass du renderer (Quad [-1, 1] x [-1, 1]). (Customizable)
void   Mesh_initDefaultMeshes_(MeshInit const*const drawableSpriteInitOpt,
                               MeshInit const*const renderingQuadInitOpt);
extern Mesh* Mesh_drawable_sprite;
extern Mesh* Mesh_rendering_quad;

// MARK: - Default Vertex structure
/// Le vertex par défaut a :
///  - Position (x,y,z) ;
///  - coord uv ;
///  - vecteur normal (x, y, z), e.g. (0, 0, 1).
///  A priori un vertex est de taille 8 floats / 32 bytes.
///  Si on veut on peut définir ses propres vertex et mesh avec vertex custom,
///  e.g. (pos, color).
typedef __attribute__((aligned(16))) union {
    float f_arr[8];
    struct { // Vertex par défaut...
        Vector3 pos;
        Vector2 uv;
        Vector3 norm;
    };
} Vertex;
/// Vecteur normal usuel (vers les z positifs)
extern const Vector3 mesh_defNorm; // = { 0, 0, 1};

// Exemple de d'autre struct de vertex...
//    struct { // Position + couleur 4 floats.
//        Vector3 pos;
//        float   _unused;
//        Vector4 color;
//    };
//    struct { // Position + uv + couleur 3 floats
//        Vector3 pos;
//        Vector2 uv;
//        Vector3 light;
//    };



// MARK: - Meshes usuelles pratiques (Utilise la structure `Vertex` ci-haut)
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
