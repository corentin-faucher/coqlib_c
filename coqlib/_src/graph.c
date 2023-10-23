//
//  Mesh.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include <stdlib.h>
#include "graph.h"
#include "utils.h"

static Vertex _mesh_sprite_vertices[4] = {
    {-0.5, 0.5, 0, 0,0, 0,0,1},
    {-0.5,-0.5, 0, 0,1, 0,0,1},
    { 0.5, 0.5, 0, 1,0, 0,0,1},
    { 0.5,-0.5, 0, 1,1, 0,0,1},
};
static Vertex _mesh_bar_vertices[8] = {
    {-0.5000, 0.5, 0,  0.000,0,  0,0,1},
    {-0.5000,-0.5, 0,  0.000,1,  0,0,1},
    {-0.1667, 0.5, 0,  0.333,0,  0,0,1},
    {-0.1667,-0.5, 0,  0.333,1,  0,0,1},
    { 0.1667, 0.5, 0,  0.667,0,  0,0,1},
    { 0.1667,-0.5, 0,  0.667,1,  0,0,1},
    { 0.5000, 0.5, 0,  1.000,0,  0,0,1},
    { 0.5000,-0.5, 0,  1.000,1,  0,0,1},
};
static Vertex _mesh_frame_vertices[] = {
    {-0.5000, 0.5000, 0,  0.000,0.000,  0,0,1},
    {-0.5000, 0.1667, 0,  0.000,0.333,  0,0,1},
    {-0.5000,-0.1667, 0,  0.000,0.667,  0,0,1},
    {-0.5000,-0.5000, 0,  0.000,1.000,  0,0,1},
    {-0.1667, 0.5000, 0,  0.333,0.000,  0,0,1},
    {-0.1667, 0.1667, 0,  0.333,0.333,  0,0,1},
    {-0.1667,-0.1667, 0,  0.333,0.667,  0,0,1},
    {-0.1667,-0.5000, 0,  0.333,1.000,  0,0,1},
    { 0.1667, 0.5000, 0,  0.667,0.000,  0,0,1},
    { 0.1667, 0.1667, 0,  0.667,0.333,  0,0,1},
    { 0.1667,-0.1667, 0,  0.667,0.667,  0,0,1},
    { 0.1667,-0.5000, 0,  0.667,1.000,  0,0,1},
    { 0.5000, 0.5000, 0,  1.000,0.000,  0,0,1},
    { 0.5000, 0.1667, 0,  1.000,0.333,  0,0,1},
    { 0.5000,-0.1667, 0,  1.000,0.667,  0,0,1},
    { 0.5000,-0.5000, 0,  1.000,1.000,  0,0,1},
};
static mesh_index_t _mesh_frame_indices[] = {
    0, 1, 4,    1, 5, 4,
    1, 2, 5,    2, 6, 5,
    2, 3, 6,    3, 7, 6,
    4, 5, 8,    5, 9, 8,
    5, 6, 9,    6, 10, 9,
    6, 7, 10,   7, 11, 10,
    8, 9, 12,   9, 13, 12,
    9, 10, 13,  10, 14, 13,
    10, 11, 14, 11, 15, 14,
};

Mesh*  mesh_sprite = NULL;

Mesh*  Mesh_createEmpty(uint vertexCount, uint indexCount,
                        enum MeshPrimitiveType primitive_type,
                        enum MeshCullMode cull_mode) {
    Mesh* mesh;
    size_t mesh_size = sizeof(Mesh) + sizeof(Vertex) * (vertexCount - 1);
    mesh = calloc(1, mesh_size);
    mesh->vertex_count = vertexCount;
    mesh->vertices_size = vertexCount * sizeof(Vertex);
    mesh->index_count = indexCount;
    mesh->primitive_type = primitive_type;
    mesh->cull_mode = cull_mode;
    if(indexCount)
        mesh->indices = calloc(indexCount, sizeof(mesh_index_t));
    return mesh;
}
Mesh*  Mesh_createBar(void) {
    Mesh* bar = Mesh_createEmpty(8, 0, mesh_primitive_triangleStrip, mesh_cullMode_none);
    mesh_fillVertices(bar, _mesh_bar_vertices, 8);
    return bar;
}
Mesh*  Mesh_createFrame(void) {
    Mesh* frame = Mesh_createEmpty(16, 54, mesh_primitive_triangle, mesh_cullMode_none);
    mesh_fillVertices(frame, _mesh_frame_vertices, 16);
    frame->indices = _mesh_frame_indices;
    return frame;
}

void   mesh_destroy(Mesh* meshToDelete) {
    if(meshToDelete->indices)
        free(meshToDelete->indices);
    free(meshToDelete);
}

void   Mesh_init(void) {
    mesh_sprite = Mesh_createEmpty(4, 0, mesh_primitive_triangleStrip, mesh_cullMode_none);
    printdebug("Mesh sprite %p.", mesh_sprite);
    mesh_fillVertices(mesh_sprite, _mesh_sprite_vertices, 4);
}
void   mesh_fillVertices(Mesh* mesh, Vertex* verticesSrc, uint count) {
    Vertex* pvSrc = verticesSrc;
    Vertex* pvDst = mesh->vertices;
    Vertex* endSrc = &verticesSrc[count];
    while(pvSrc < endSrc) {
        *pvDst = *pvSrc;
        pvDst ++;
        pvSrc ++;
    }
}

const PerInstanceUniforms piu_default = PIU_DEFAULT;

PerFrameUniforms pfu_default = {
    {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    },
    0.f, 0.f, 0.f, 0.f
};


