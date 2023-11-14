//
//  graph_mesh.h
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef graph_mesh_h
#define graph_mesh_h

#include "utils.h"

/*-- Vertex --------------------*/
typedef struct {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
} Vertex;

/*-- Mesh ----------------------*/
// Type de primitives. Telles que dans Metal, voir MTLRenderCommandEncoder.h.
enum MeshPrimitiveType {
    mesh_primitive_point = 0,
    mesh_primitive_line = 1,
    mesh_primitive_lineStrip = 2,
    mesh_primitive_triangle = 3,
    mesh_primitive_triangleStrip = 4,
};
enum MeshCullMode {
    mesh_cullMode_none = 0,
    mesh_cullMode_front = 1,
    mesh_cullMode_back = 2,
};

/*-- Infos d'une mesh. (implementation partiellement cachée, depend de l'OS.) --*/
typedef struct _Mesh Mesh;

/// La "sprite". Mesh partagée par la plupart des surfaces.
/// Init lors du Mesh_init().
extern Mesh* mesh_sprite;
Mesh*  Mesh_createEmpty(const Vertex* verticesOpt, uint vertexCount,
                        const uint16_t* indicesOpt, uint indexCount,
                        enum MeshPrimitiveType primitive_type,
                        enum MeshCullMode cull_mode, Bool isShared);
void   mesh_destroy(Mesh* meshToDelete);
Mesh*  Mesh_createBar(void);
Mesh*  Mesh_createFrame(void);
Mesh*  Mesh_createFan(void);
void   mesh_fan_update(Mesh* fan, float ratio);



/*-- Getters --*/
uint32_t mesh_indexCount(Mesh* mesh);
uint32_t mesh_vertexCount(Mesh* mesh);
size_t   mesh_verticesSize(Mesh* mesh);
Vertex*  mesh_vertices(Mesh* mesh);
enum MeshPrimitiveType mesh_primitiveType(Mesh* mesh);
enum MeshCullMode      mesh_cullMode(Mesh* mesh);
Bool     mesh_isShared(Mesh* mesh);




#endif /* graph_mesh_h */
