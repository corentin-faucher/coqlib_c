//
//  graph_mesh.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#include "graph_mesh.h"
#include <stdlib.h>  // Truc c de base...
#include <math.h>
#include "utils.h"
static Vertex _vertex_default_origin = {
    0.f, 0.f, 0.f,  0.5f,0.5f,  0.f,0.f,1.f
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
static uint16_t _mesh_frame_indices[] = {
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

static uint16_t _mesh_fan_indices[] = {
    0, 1, 2,   0, 2, 3,
    0, 3, 4,   0, 4, 5,
    0, 5, 6,   0, 6, 7,
    0, 7, 8,   0, 8, 9
};

Mesh*  Mesh_createBar(void) {
    Mesh* bar = Mesh_createEmpty(_mesh_bar_vertices, 8, NULL, 0,
                                 mesh_primitive_triangleStrip, mesh_cullMode_none, false);
    return bar;
}
Mesh*  Mesh_createFrame(void) {
    Mesh* frame = Mesh_createEmpty(_mesh_frame_vertices, 16, _mesh_frame_indices, 54,
                                   mesh_primitive_triangle, mesh_cullMode_none, false);
    return frame;
}

Mesh*  Mesh_createFan(void) {
    Mesh* fan = Mesh_createEmpty(NULL, 10, _mesh_fan_indices, 24,
                                 mesh_primitive_triangle, mesh_cullMode_none, false);
    // Init des vertices pour une fan.
    Vertex* vertices = mesh_vertices(fan);
    vertices[0] = _vertex_default_origin;  // Centre de la fan en (0, 0).
    Vertex* p = &vertices[1];
    Vertex* const end = &vertices[10];
    float i = 0.f;
    while(p < end) {
        p->x = -0.5f * sinf(2.f * M_PI * i / 8.f);
        p->y =  0.5f * cosf(2.f * M_PI * i / 8.f);
        p->u = 0.5f - 0.5f * sinf(2.f * M_PI * i / 8.f);
        p->v = 0.5f - 0.5f * cosf(2.f * M_PI * i / 8.f);
        p->nz = 1.f;
        i++;
        p++;
    }
    return fan;
}
void   mesh_fan_update(Mesh* fan, float ratio) {
    if(mesh_vertexCount(fan) < 10) {
        printerror("Fan mesh with vertex_count < 10.");
        return;
    }
    Vertex* vertices = mesh_vertices(fan);
    Vertex* p = &vertices[2];  // (On skip le premier, reste a (0, 0.5).)
    Vertex* const end = &vertices[10];
    float i = 1.f;
    while(p < end) {
        p->x = -0.5f * sinf(ratio * 2.f * M_PI * i / 8.f);
        p->y =  0.5f * cosf(ratio * 2.f * M_PI * i / 8.f);
        p->u = 0.5f - 0.5f * sinf(ratio * 2.f * M_PI * i / 8.f);
        p->v = 0.5f - 0.5f * cosf(ratio * 2.f * M_PI * i / 8.f);
        i++;
        p++;
    }
}


