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

// Type de cull mode. Telles que dans Metal, voir MTLRenderCommandEncoder.h.
// (Comme OpenGL aussi...)
enum MeshCullMode {
    mesh_cullMode_none = 0,
    mesh_cullMode_front = 1,
    mesh_cullMode_back = 2,
};

/// Type de primitives. Telles que dans Metal.
/// (C'est aussi la même chose pour DirectX,
/// par contre pour OpenGL il faut faire un petit mapping.)
enum MeshPrimitiveType {
    meshprimitive_point = 0,
    meshprimitive_line = 1,
    meshprimitive_lineStrip = 2,
    meshprimitive_triangle = 3,
    meshprimitive_triangleStrip = 4,
};
// De "MTLRenderCommandEncoder.h" :
//typedef NS_ENUM(NSUInteger, MTLPrimitiveType) {
//    MTLPrimitiveTypePoint = 0,
//    MTLPrimitiveTypeLine = 1,
//    MTLPrimitiveTypeLineStrip = 2,
//    MTLPrimitiveTypeTriangle = 3,
//    MTLPrimitiveTypeTriangleStrip = 4,
//} API_AVAILABLE(macos(10.11), ios(8.0));

// Structure d'init d'une mesh.
typedef struct MeshInit {
    const void*     verticesOpt;
    uint32_t        vertexCount;
    uint32_t        vertexSizeOpt;
    const uint16_t* indicesOpt;
    uint32_t        indexCountOpt;
    enum MeshPrimitiveType primitiveType;
    enum MeshCullMode      cullMode;
    uint32_t        flags;
} MeshInit;
/// Init d'une mesh. Par défaut la taille d'un vertex est sizeof(Vertex) == 32 bytes. Si taille custom setter vertexSizeOpt.
Mesh*    Mesh_create(MeshInit initInfo);
void     meshref_init(Mesh*const* meshRef, Mesh* initValue);
/// Libérer la mesh (non shared seulement. Pour libérer une shared, le faire manuellement avec `mesh_engine_deinit_`...)
void     meshref_render_releaseAndNull(Mesh*const* meshRef);

// MARK: - Edition
typedef struct MeshToEdit {
    void*          v;   // Liste de vertex (init à `beg`). Typiquement un pointeur `Vertex*`.
    void*const     beg; // Début de la liste de vertex.
    void*const     end; // Fin de la liste de vertex.
    uint32_t       indexCount;
    uint32_t const vertexSize;
    Mesh*const     _mesh;
} MeshToEdit;
//bool       mesh_isReadyToEdit(Mesh const* mesh);
/// Obtenir la référence aux vertices pour édition (NULL si non mutable)
MeshToEdit mesh_retainToEditOpt(Mesh* mesh);
/// Fini d'éditer, flager pour que la thread de rendering met à jour le vertex buffer.
void       meshtoedit_release(MeshToEdit meshEdit);
#define withMeshToEdit_beg(meshEdit, mesh, VertexType, vertices) \
            { MeshToEdit meshEdit = mesh_retainToEditOpt(mesh);  \
            if(meshEdit.v) { VertexType* vertices = meshEdit.v; \
            if(sizeof(VertexType) != meshEdit.vertexSize) { printerror("Bad Vertex size."); }
#define withMeshToEdit_end(meshEdit) meshtoedit_release(meshEdit); } \
            else { printerror("Cannot edit mesh."); } }

// MARK: - Dessin (par renderer)
// Info pour dessiner par le renderer.
typedef struct MeshToDraw {
    uint32_t vertexCount;
    size_t   verticesSize;
    uint32_t indexCount;
    uint16_t cull_mode; // (Même enum pour Metal et OpenGL)
    // Buffers spécifiques à l'engine
    union {
        // Metal
        struct {
            void const* metal_verticesOpt; // Vertex directement.
            // Vertex dans un Metal Buffers, convertir avec `(__bridge id<MTLBuffer>)`
            void const* metal_verticesMTLBufferOpt;
            void const* metal_indicesMTLBufersOpt;
            uint32_t    metal_primitiveType;
        };
        // OpenGL : index des buffers
        struct {
            uint32_t glVertexArrayId;   // VAO
            // (Juste le VAO est nécessaire...)
            uint32_t glPrimitiveType;
        };
    };
} MeshToDraw;
void       mesh_render_checkMeshInit(Mesh* mesh);
void       mesh_render_checkMeshUpdate(Mesh* mesh);
MeshToDraw mesh_render_getToDraw(Mesh const* mesh);

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
// Exemple de d'autres structs de vertex possibles...
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

// MARK: - Exemples de meshes usuelles pratiques. Utilisent la structure `Vertex` ci-haut.
Mesh*  Mesh_createHorizontalBar(void);
Mesh*  Mesh_createVerticalBar(void);
Mesh*  Mesh_createFrame(void);
void   mesh_frame_setCenterRatios(Mesh* frame, float ratioX, float ratioY);
Mesh*  Mesh_createFan(void);
void   mesh_fan_update(Mesh* fan, float ratio);
Mesh*  Mesh_createPlot(float* xs, float* ys, uint32_t count, float delta, float ratio);
Mesh*  Mesh_createPlotGrid(float xmin, float xmax, float xR, float deltaX,
                           float ymin, float ymax, float yR, float deltaY,
                           float lineWidthRatio);
/// Importation d'un fichier 3D .obj.
/// Pas très optimale... Version simple : liste de triangle non indexé.       
Mesh*  Mesh_createFromObjFile(char const* path);

// Flags d'une mesh, seulement needUpdate peut changer.
enum {
    // Si shared, `meshref_releaseAndNull` ne dealloc pas la mesh. Il faut caller explicitement `mesh_engine_deinit_` + `free`.
    mesh_flag_shared =          0x0001,  // Pas besoin de libérer lors du release (on n'est pas l'owner).
    mesh_flag_mutable =         0x0002,  // On peut éditer les vertex (si absent init sans le champ `vertices`)
    mesh_flag_firstCustomFlag = 0x1000,
};

// MARK: - Private stuff...
/// "privé". Libère les resouces (OpenGL/Metal) de la mesh. Utiliser a priori `meshref_releaseAndNull`.
void     mesh_render_deinit_(Mesh* mesh);

#endif /* graph_mesh_h */
