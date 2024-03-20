//
//  graph_mesh.h
//  Mesh : ensemble de vertex pour affichage de surfaces.
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef COQ_GRAPH_MESH_H
#define COQ_GRAPH_MESH_H

#include "maths/math_base.h"

/*-- Vertex --------------------*/
typedef struct {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
} Vertex;

/*-- Mesh ----------------------*/
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

/*-- Infos d'une mesh. (implementation partiellement cachée, depend de l'OS.) --*/
typedef struct Mesh_ Mesh;

/// La "sprite". Mesh partagée par la plupart des surfaces.
/// Init lors du Mesh_init().
extern Mesh* mesh_sprite;
Mesh*  Mesh_createEmpty(const Vertex* verticesOpt,  uint32_t vertexCount,
                        const uint16_t* indicesOpt, uint32_t indexCount,
                        enum MeshPrimitiveType primitive_type,
                        enum MeshCullMode cull_mode, bool isShared);
void   mesh_destroyAndNull(Mesh** const meshToDelete);
Mesh*  Mesh_createHorizontalBar(void);
Mesh*  Mesh_createVerticalBar(void);
Mesh*  Mesh_createFrame(void);
Mesh*  Mesh_createFan(void);
void   mesh_fan_update(Mesh* fan, float ratio);
Mesh*  Mesh_createPlot(float* xs, float* ys, uint32_t count, float delta, float ratio);
Mesh*  Mesh_createPlotGrid(float xmin, float xmax, float xR, float deltaX,
                           float ymin, float ymax, float yR, float deltaY,
                           float lineWidthRatio);



/*-- Getters --*/
uint32_t mesh_indexCount(Mesh* mesh);
uint32_t mesh_vertexCount(Mesh* mesh);
size_t   mesh_verticesSize(Mesh* mesh);
/// Référence des vertices pour mise à jour des positions.
Vertex*  mesh_vertices(Mesh* mesh);
/// Une fois modifiées signaler que les vertices devront être updatés
/// (juste pour OpenGL).
void     mesh_needToUpdateVertices(Mesh* mesh);
enum MeshPrimitiveType  mesh_primitiveType(Mesh* mesh);
enum MeshCullMode       mesh_cullMode(Mesh* mesh);
bool     mesh_isShared(Mesh* mesh);




#endif /* graph_mesh_h */
