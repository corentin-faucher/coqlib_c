//
//  graph_mesh.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#include "graph_mesh.h"

#include "../utils/util_base.h"

#pragma mark - Mesh de base et init -------------

const Vector3 mesh_defNorm = {{ 0, 0, 1 }};

// Point central avec normal vers le haut.
static const Vertex vertex_default_origin_ = {{
    0.f, 0.f, 0.f,  0.5f, 0.5f,  0.f,0.f,1.f
}};
static const Vertex mesh_sprite_vertices_[4] = {
    {{-0.5, 0.5, 0, 0.0001, 0.0001, 0,0,1}},
    {{-0.5,-0.5, 0, 0.0001, 0.9999, 0,0,1}},
    {{ 0.5, 0.5, 0, 0.9999, 0.0001, 0,0,1}},
    {{ 0.5,-0.5, 0, 0.9999, 0.9999, 0,0,1}},
};
static const Vertex mesh_shaderQuad_vertices_[4] = {
    {{-1.0, 1.0, 0, 0.0, 0.0, 0,0,1}},
    {{-1.0,-1.0, 0, 0.0, 1.0, 0,0,1}},
    {{ 1.0, 1.0, 0, 1.0, 0.0, 0,0,1}},
    {{ 1.0,-1.0, 0, 1.0, 1.0, 0,0,1}},
};

Mesh   mesh_sprite = {} ;
Mesh   mesh_shaderQuad_ = {};

void   mesh_init(Mesh* const mesh, const Vertex* const verticesOpt, uint32_t const vertexCount,
                const uint16_t* const indicesOpt, uint32_t const indexCount,
                enum MeshPrimitiveType const primitive_type, enum MeshCullMode const cull_mode, bool const isShared) {
    uint_initConst(&mesh->vertex_count, vertexCount);
    uint_initConst(&mesh->index_count, indexCount);
    mesh->primitive_type = primitive_type;
    mesh->cull_mode = cull_mode;
    *(bool*)&mesh->isShared = isShared;
    
    mesh_engine_initBuffers_(mesh, verticesOpt, indicesOpt);
}

void   Mesh_init_(void) {
    if(mesh_sprite.vertex_count) { printerror("Mesh already init."); return; }
    // Mesh par défaut (sprite)
    mesh_init(&mesh_sprite, mesh_sprite_vertices_, 4, NULL, 0, mesh_primitive_triangleStrip, mesh_cullMode_none, true);
    mesh_init(&mesh_shaderQuad_, mesh_shaderQuad_vertices_, 4, NULL, 0, mesh_primitive_triangleStrip, mesh_cullMode_none, true);
}

#pragma mark - "Bar" i.e. `====`

static const Vertex mesh_bar_vertices_[8] = {
    {{-0.5000, 0.5, 0,  0.000,0,  0,0,1}},
    {{-0.5000,-0.5, 0,  0.000,1,  0,0,1}},
    {{-0.1667, 0.5, 0,  0.333,0,  0,0,1}},
    {{-0.1667,-0.5, 0,  0.333,1,  0,0,1}},
    {{ 0.1667, 0.5, 0,  0.667,0,  0,0,1}},
    {{ 0.1667,-0.5, 0,  0.667,1,  0,0,1}},
    {{ 0.5000, 0.5, 0,  1.000,0,  0,0,1}},
    {{ 0.5000,-0.5, 0,  1.000,1,  0,0,1}},
};
static const Vertex mesh_vbar_vertices_[8] = {
    {{ 0.5, 0.5000, 0,  1, 0.000,  0,0,1}},
    {{-0.5, 0.5000, 0,  0, 0.000,  0,0,1}},
    {{ 0.5, 0.1667, 0,  1, 0.333,  0,0,1}},
    {{-0.5, 0.1667, 0,  0, 0.333,  0,0,1}},
    {{ 0.5,-0.1667, 0,  1, 0.667,  0,0,1}},
    {{-0.5,-0.1667, 0,  0, 0.667,  0,0,1}},
    {{ 0.5,-0.5000, 0,  1, 1.000,  0,0,1}},
    {{-0.5,-0.5000, 0,  0, 1.000,  0,0,1}},
};
Mesh*  Mesh_createHorizontalBar(void) {
    Mesh* const bar = coq_callocTyped(Mesh);
    mesh_init(bar, mesh_bar_vertices_, 8, NULL, 0, mesh_primitive_triangleStrip, mesh_cullMode_none, false);
    return bar;
}
Mesh*  Mesh_createVerticalBar(void) {
    Mesh* const bar = coq_callocTyped(Mesh);
    mesh_init(bar, mesh_vbar_vertices_, 8, NULL, 0, mesh_primitive_triangleStrip, mesh_cullMode_none, false);
    return bar;
}

#pragma mark - Frame (e.g. cadre stretchable) ----------------

static const Vertex   mesh_frame_vertices_[] = {
    {{-0.5000, 0.5000, 0,  0.000,0.000,  0,0,1}},
    {{-0.5000, 0.1667, 0,  0.000,0.333,  0,0,1}},
    {{-0.5000,-0.1667, 0,  0.000,0.667,  0,0,1}},
    {{-0.5000,-0.5000, 0,  0.000,1.000,  0,0,1}},
    {{-0.1667, 0.5000, 0,  0.333,0.000,  0,0,1}},
    {{-0.1667, 0.1667, 0,  0.333,0.333,  0,0,1}},
    {{-0.1667,-0.1667, 0,  0.333,0.667,  0,0,1}},
    {{-0.1667,-0.5000, 0,  0.333,1.000,  0,0,1}},
    {{ 0.1667, 0.5000, 0,  0.667,0.000,  0,0,1}},
    {{ 0.1667, 0.1667, 0,  0.667,0.333,  0,0,1}},
    {{ 0.1667,-0.1667, 0,  0.667,0.667,  0,0,1}},
    {{ 0.1667,-0.5000, 0,  0.667,1.000,  0,0,1}},
    {{ 0.5000, 0.5000, 0,  1.000,0.000,  0,0,1}},
    {{ 0.5000, 0.1667, 0,  1.000,0.333,  0,0,1}},
    {{ 0.5000,-0.1667, 0,  1.000,0.667,  0,0,1}},
    {{ 0.5000,-0.5000, 0,  1.000,1.000,  0,0,1}},
};
static const uint16_t mesh_frame_indices_[] = {
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
Mesh*  Mesh_createFrame(void) {
    Mesh* const frame = coq_callocTyped(Mesh);
    mesh_init(frame, mesh_frame_vertices_, 16, mesh_frame_indices_, 54, mesh_primitive_triangle, mesh_cullMode_none, false);
    return frame;
}

#pragma mark - Fan (disque <-> pointe) ----------------

static const uint16_t _mesh_fan_indices[] = {
    0, 1, 2,   0, 2, 3,
    0, 3, 4,   0, 4, 5,
    0, 5, 6,   0, 6, 7,
    0, 7, 8,   0, 8, 9
};
Mesh*  Mesh_createFan(void) {
    Mesh* const fan = coq_callocTyped(Mesh); 
    mesh_init(fan, NULL, 10, _mesh_fan_indices, 24, mesh_primitive_triangle, mesh_cullMode_none, false);
    // Init des vertices pour une fan.
    Vertex* vertices = mesh_engine_retainVertices(fan);
    vertices[0] = vertex_default_origin_;  // Centre de la fan en (0, 0).
    Vertex* p = &vertices[1];
    Vertex* const end = &vertices[10];
    float i = 0.f;
    while(p < end) {
        p->pos.x = -0.5f * sinf(2.f * M_PI * i / 8.f);
        p->pos.y =  0.5f * cosf(2.f * M_PI * i / 8.f);
        p->uv.x = 0.5f - 0.5f * sinf(2.f * M_PI * i / 8.f);
        p->uv.y = 0.5f - 0.5f * cosf(2.f * M_PI * i / 8.f);
        p->norm.z = 1.f;
        i++;
        p++;
    }
    mesh_engine_releaseVertices(fan);
    return fan;
}
void   mesh_fan_update(Mesh* fan, float ratio) {
    if(fan->vertex_count < 10) {
        printerror("Fan mesh with vertex_count < 10.");
        return;
    }
    Vertex* vertices = mesh_engine_retainVertices(fan);
    Vertex* p = &vertices[2];  // (On skip le premier, reste a (0, 0.5).)
    Vertex* const end = &vertices[10];
    float i = 1.f;
    while(p < end) {
        p->pos.x = -0.5f * sinf(ratio * 2.f * M_PI * i / 8.f);
        p->pos.y =  0.5f * cosf(ratio * 2.f * M_PI * i / 8.f);
        p->uv.x = 0.5f - 0.5f * sinf(ratio * 2.f * M_PI * i / 8.f);
        p->uv.y = 0.5f - 0.5f * cosf(ratio * 2.f * M_PI * i / 8.f);
        i++;
        p++;
    }
    mesh_engine_releaseVertices(fan);
}

#pragma mark - Graphique ------------
/// Les xs/ys devraient être préformaté pour être contenu dans [-0.5, 0.5] x [-0.5, 0.5]...
/// delta: épaisseur des lignes.
/// ratio: étirement en largeur.
Mesh*  Mesh_createPlot(float* xs, float* ys, uint32_t count, float delta, float ratio) {
    uint32_t n_lines = count - 1;
    uint32_t n_points = count;
    uint32_t vertices_count = 4*n_lines + 4*n_points;
    uint32_t indices_count =  6*n_lines + 6*n_points;
    Vertex* vertices = coq_calloc(vertices_count, sizeof(Vertex));
    uint16_t* indices = coq_calloc(indices_count, sizeof(uint16_t));
    
    // Lignes
    Vertex* v = &vertices[0];
    uint16_t* ind = &indices[0];
    float* xn = &xs[1]; float* yn = &ys[1]; // (pointeur sur next)
    float* x =  &xs[0]; float* y =  &ys[0]; // (courant)
    for(uint32_t i = 0; i < n_lines; i++) {
        float theta = atanf((*yn - *y) / (*xn - *x));
        float deltax = delta * sinf(theta) / ratio;
        float deltay = delta * cosf(theta);
        *v = (Vertex) {{ *x - deltax, *y + deltay, 0,     0, 0,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ *x + deltax, *y - deltay, 0,     0, 1,  0, 0, 1 }};  v++;
        x++; y++;  xn++; yn++;
        *v = (Vertex) {{ *x - deltax, *y + deltay, 0,  0.75, 0,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ *x + deltax, *y - deltay, 0,  0.75, 1,  0, 0, 1 }};  v++;
        *ind = 4*i;      ind++;
        *ind = 4*i + 1;  ind++;
        *ind = 4*i + 2;  ind++;
        *ind = 4*i + 1;  ind++;
        *ind = 4*i + 2;  ind++;
        *ind = 4*i + 3;  ind++;
    }
    // Points
    float const pts_deltax = 1.15f*delta / ratio;
    float const pts_deltay = 1.15f*delta;
    uint16_t const ind_dec = 4*n_lines;
    x =  &xs[0]; y =  &ys[0]; // (courant)
    for(uint32_t i = 0; i < n_points; i++) {
        *v = (Vertex) {{ *x - pts_deltax, *y + pts_deltay, 0,     1, 0,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ *x - pts_deltax, *y - pts_deltay, 0,     1, 1,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ *x + pts_deltax, *y + pts_deltay, 0,     2, 0,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ *x + pts_deltax, *y - pts_deltay, 0,     2, 1,  0, 0, 1 }};  v++;
        x++; y++;
        *ind = ind_dec + 4*i;      ind++;
        *ind = ind_dec + 4*i + 1;  ind++;
        *ind = ind_dec + 4*i + 2;  ind++;
        *ind = ind_dec + 4*i + 1;  ind++;
        *ind = ind_dec + 4*i + 2;  ind++;
        *ind = ind_dec + 4*i + 3;  ind++;
    }
    Mesh* plot = coq_callocTyped(Mesh);
    mesh_init(plot, vertices, vertices_count, indices, indices_count, mesh_primitive_triangle, mesh_cullMode_none, false);
    coq_free(vertices);
    coq_free(indices);
    return plot;
}
Mesh*  Mesh_createPlotGrid(float xmin, float xmax, float xR, float deltaX,
                           float ymin, float ymax, float yR, float deltaY,
                           float lineWidthRatio) {
    float x0 = xR - floorf((xR - xmin) / deltaX) * deltaX;
    uint32_t m = (uint32_t)((xmax - x0) / deltaX) + 1;
    float xlinedelta = deltaX * lineWidthRatio * 0.5;
    float y0 = yR - floorf((yR - ymin) / deltaY) * deltaY;
    uint32_t n = (uint32_t)((ymax - y0) / deltaY) + 1;
    float ylinedelta = deltaY * lineWidthRatio * 0.5;
    
    uint32_t vertices_count = 4*(m + n);
    uint32_t indices_count =  6*(m + n);
    Vertex* vertices = coq_calloc(vertices_count, sizeof(Vertex));
    uint16_t* indices = coq_calloc(indices_count, sizeof(uint16_t));
    
    Vertex* v = &vertices[0];
    uint16_t* ind = &indices[0];
    // Ligne verticales (divisions en x)
    for(uint32_t i = 0; i < m; i++) {
        float x = x0 + (float)i * deltaX;
        *v = (Vertex) {{ x - xlinedelta, ymax, 0,     0, 0,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ x - xlinedelta, ymin, 0,     0, 1,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ x + xlinedelta, ymax, 0,     1, 0,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ x + xlinedelta, ymin, 0,     1, 1,  0, 0, 1 }};  v++;
        *ind = 4*i;      ind++;
        *ind = 4*i + 1;  ind++;
        *ind = 4*i + 2;  ind++;
        *ind = 4*i + 1;  ind++;
        *ind = 4*i + 2;  ind++;
        *ind = 4*i + 3;  ind++;
    }
    // Ligne horizontales (divisions en y)
    uint16_t const ind_dec = 4*m;
    for(uint32_t i = 0; i < n; i++) {
        float y = y0 + (float)i * deltaY;
        *v = (Vertex) {{ xmin, y + ylinedelta, 0,     0, 0,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ xmin, y - ylinedelta, 0,     0, 1,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ xmax, y + ylinedelta, 0,     1, 0,  0, 0, 1 }};  v++;
        *v = (Vertex) {{ xmax, y - ylinedelta, 0,     1, 1,  0, 0, 1 }};  v++;
        *ind = ind_dec + 4*i;      ind++;
        *ind = ind_dec + 4*i + 1;  ind++;
        *ind = ind_dec + 4*i + 2;  ind++;
        *ind = ind_dec + 4*i + 1;  ind++;
        *ind = ind_dec + 4*i + 2;  ind++;
        *ind = ind_dec + 4*i + 3;  ind++;
    }
    Mesh* plotGrid = coq_callocTyped(Mesh);
    mesh_init(plotGrid, vertices, vertices_count, indices, indices_count, mesh_primitive_triangle, mesh_cullMode_none, false); 
    coq_free(vertices);
    coq_free(indices);
    return plotGrid;
}

#pragma mark - Courbe ------------
#warning TODO...

static const Vertex _vertex_uv00 = {{
    0.f, 0.f, 0.f,  0.f,0.f,  0.f,0.f,1.f
}};
static const Vertex _vertex_uv01 = {{
    0.f, 0.f, 0.f,  0.f,1.f,  0.f,0.f,1.f
}};
static const Vertex _vertex_uv10 = {{
    0.f, 0.f, 0.f,  0.f,1.f,  0.f,0.f,1.f
}};
static const Vertex _vertex_uv11 = {{
    0.f, 0.f, 0.f,  1.f,1.f,  0.f,0.f,1.f
}};
Mesh*  Mesh_creatCurve(Vector2* v_arr, uint32_t v_count, float delta) {
    if(v_count < 2) {
        printerror("Mesh curve with less than 2 pos."); return &mesh_sprite;
    }
    uint32_t vertex_count = 4*(v_count - 1);
    Mesh* const curve = coq_callocTyped(Mesh);
    mesh_init(curve, NULL, vertex_count, NULL, 0, mesh_primitive_triangleStrip, mesh_cullMode_none, false);
    Vertex* vertices = mesh_engine_retainVertices(curve);
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
        (*vertices).pos.x = vul.x; (*vertices).pos.y = vul.y;
        vertices++;
        (*vertices) = _vertex_uv01;
        (*vertices).pos.x = vdl.x; (*vertices).pos.y = vdl.y;
        vertices++;
        (*vertices) = _vertex_uv10;
        (*vertices).pos.x = vur.x; (*vertices).pos.y = vur.y;
        vertices++;
        (*vertices) = _vertex_uv11;
        (*vertices).pos.x = vdr.x; (*vertices).pos.y = vdr.y;
        vertices++;
        vul = vul_next;
        vdl = vdl_next;
    }
    mesh_engine_releaseVertices(curve);
    return curve;
}
