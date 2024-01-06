//
//  graph_mesh.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#include "_graph_mesh.h"

static Vertex _vertex_default_origin = {
    0.f, 0.f, 0.f,  0.5f,0.5f,  0.f,0.f,1.f
};
static Vertex _vertex_uv00 = {
    0.f, 0.f, 0.f,  0.f,0.f,  0.f,0.f,1.f
};
static Vertex _vertex_uv01 = {
    0.f, 0.f, 0.f,  0.f,1.f,  0.f,0.f,1.f
};
static Vertex _vertex_uv10 = {
    0.f, 0.f, 0.f,  0.f,1.f,  0.f,0.f,1.f
};
static Vertex _vertex_uv11 = {
    0.f, 0.f, 0.f,  1.f,1.f,  0.f,0.f,1.f
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
static Vertex _mesh_vbar_vertices[8] = {
    { 0.5, 0.5000, 0,  1, 0.000,  0,0,1},
    {-0.5, 0.5000, 0,  0, 0.000,  0,0,1},
    { 0.5, 0.1667, 0,  1, 0.333,  0,0,1},
    {-0.5, 0.1667, 0,  0, 0.333,  0,0,1},
    { 0.5,-0.1667, 0,  1, 0.667,  0,0,1},
    {-0.5,-0.1667, 0,  0, 0.667,  0,0,1},
    { 0.5,-0.5000, 0,  1, 1.000,  0,0,1},
    {-0.5,-0.5000, 0,  0, 1.000,  0,0,1},
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

Mesh*  Mesh_createHorizontalBar(void) {
    return Mesh_createEmpty(_mesh_bar_vertices, 8, NULL, 0,
                            mesh_primitive_triangleStrip, mesh_cullMode_none, false);
}
Mesh* Mesh_createVerticalBar(void) {
    return Mesh_createEmpty(_mesh_vbar_vertices, 8, NULL, 0,
                            mesh_primitive_triangleStrip, mesh_cullMode_none, false);
}
Mesh*  Mesh_createFrame(void) {
    return Mesh_createEmpty(_mesh_frame_vertices, 16, _mesh_frame_indices, 54,
                            mesh_primitive_triangle, mesh_cullMode_none, false);
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

Mesh*  Mesh_creatCurve(Vector2* v_arr, uint32_t v_count, float delta) {
    if(v_count < 2) {
        printerror("Mesh curve with less than 2 pos."); return mesh_sprite;
    }
    uint32_t vertex_count = 4*(v_count - 1);
    Mesh* mesh = Mesh_createEmpty(NULL, vertex_count, NULL, 0, mesh_primitive_triangleStrip, mesh_cullMode_none, false);
    Vertex* vertices = mesh_vertices(mesh);
    // Init des deux premier vertex a gauche. (up-left et down-left).
    Vector2 v0 = v_arr[0], v1 = v_arr[1];
    Vector2 v01 = vector2_minus(v1, v0);
    float   n01 = vector2_norm(v01);
    Vector2 v01d = vector2_times(vector2_cross(v01), delta / n01);
    Vector2 v01u = vector2_times(v01d, -1.f);
    Vector2 vul = vector2_add(v0, v01u);
    Vector2 vdl = vector2_add(v0, v01d);
    // Reste des vertex
    for(uint32_t index = 1; index < v_count; index ++) {
        Vector2 v2;
        Vector2 v12;
        float   n12;
        Vector2 v12d, v12u;
        Vector2 vul_next, vdl_next;
        // Calcul des vertex à droite
        Vector2 vur, vdr;
        // Fin ?
        if(index >= v_count - 1) {
            vur = vector2_add(v1, v01u);
            vdr = vector2_add(v1, v01d);
            vul_next = vur;
            vdl_next = vdr;
        } else {
            v2 = v_arr[index+1];
            v12 = vector2_minus(v2, v1);
            n12 = vector2_norm(v12);
            v12d = vector2_times(vector2_cross(v12), delta / n12);
            v12u = vector2_times(v12d, -1.f);
            // Vers le bas ?
            if(vector2_dot(vector2_minus(v12d, v01d), v01) < 0.f) {
                vur =      vector2_add(v1, v01u);
                vdr =      vector2_add(v1, vector2_add(v01d, vector2_times(v01, vector2_dot(v12d, v01) / (n01*n01))));
                vul_next = vector2_add(v1, v12u);
                vdl_next = vector2_add(v1, vector2_add(v12d, vector2_times(v12, vector2_dot(v01d, v12) / (n12*n12))));
            } else {
                vur =      vector2_add(v1, vector2_add(v01u, vector2_times(v01, vector2_dot(v12u, v01) / (n01*n01))));
                vdr =      vector2_add(v1, v01d);
                vul_next = vector2_add(v1, vector2_add(v12u, vector2_times(v12, vector2_dot(v01u, v12) / (n12*n12))));
                vdl_next = vector2_add(v1, v12d);
            }
            // Mise a jour pour next.
            v1 = v2;
            v01 = v12;
            n01 = n12;
            v01d = v12d;
            v01u = v12u;
        }
        // Ajout des 4 vertex du segment.
        (*vertices) = _vertex_uv00;
        (*vertices).x = vul.x; (*vertices).y = vul.y;
        vertices++;
        (*vertices) = _vertex_uv01;
        (*vertices).x = vdl.x; (*vertices).y = vdl.y;
        vertices++;
        (*vertices) = _vertex_uv10;
        (*vertices).x = vur.x; (*vertices).y = vur.y;
        vertices++;
        (*vertices) = _vertex_uv11;
        (*vertices).x = vdr.x; (*vertices).y = vdr.y;
        vertices++;
        vul = vul_next;
        vdl = vdl_next;
    }
    return mesh;
}
