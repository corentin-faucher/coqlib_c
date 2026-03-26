//
//  graph_mesh.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#include "graph_mesh.h"
#include "graph_mesh_private.h"

#include "../utils/util_base.h"
#include "../coq__buildConfig.h"

// MARK: - Mesh de base et init 
bool Mesh_engineIsMetal_ = false;

Mesh*  Mesh_create(MeshInit initInfo)
{
    if(!initInfo.vertexCount) {
        printerror("vertexCount is 0."); return NULL;
    }
    size_t const vertexSize = (initInfo.vertexSizeOpt ? initInfo.vertexSizeOpt : sizeof(Vertex));
    size_t const verticesSize = initInfo.vertexCount * vertexSize;
    // Si non mutable, on ne garde pas l'array `vertices`. (Seulement dans le buffer Metal/OpenGL)
    size_t meshSize;
    initInfo.flags &= ~mesh_flags__privates;
    bool metal_no_buffer = false;
    if(Mesh_engineIsMetal_ && (initInfo.flags & mesh_flag_mutable) && (verticesSize < Mesh_verticesSizeForBuffer)) {
    // Double buffering directement dans Mesh.vertices0.
        meshSize = sizeof(Mesh) + 2*verticesSize - sizeof(float);
        metal_no_buffer = true;
    } else {
        meshSize = sizeof(Mesh) + verticesSize - sizeof(float);
    }
    // Création de la mesh.
    Mesh *mesh = coq_calloc(1, meshSize);
    uint_initConst(&mesh->vertexCount, initInfo.vertexCount);
    uint_initConst(&mesh->vertexSize, (uint32_t)vertexSize);
    size_initConst(&mesh->verticesSize, verticesSize);
    if(initInfo.indicesOpt && initInfo.indexCountOpt) {
        uint_initConst(&mesh->maxIndexCount, initInfo.indexCountOpt);
        uint_initConst(&mesh->actualIndexCount, initInfo.indexCountOpt);
    } else if(initInfo.indicesOpt || initInfo.indexCountOpt) {
        printwarning("Missing indices or index Count.");
        initInfo.indicesOpt = NULL;
        initInfo.indexCountOpt = 0;
    }
    *(uint16_t*)&mesh->primitive_type = initInfo.primitiveType;
    *(uint16_t*)&mesh->cull_mode =    initInfo.cullMode;
    mesh->flags = initInfo.flags;
    if(metal_no_buffer) {
        *(float**)&mesh->mtl_vertices1Opt = (float*)((char*)mesh->vertices0 + verticesSize);
    }
    // Filling
    if(initInfo.verticesOpt) {
        memcpy(mesh->vertices0, initInfo.verticesOpt, verticesSize);
        if(mesh->mtl_vertices1Opt)
            memcpy(mesh->mtl_vertices1Opt, initInfo.verticesOpt, verticesSize);
    } else if(!(mesh->flags & mesh_flag_mutable)) {
        printwarning("Non mutable mesh without init vertices.");
    }
    if(initInfo.indicesOpt && initInfo.indexCountOpt) {
        mesh->indicesOpt = coq_callocSimpleArray(initInfo.indexCountOpt, uint16_t);
        size_t const indicesSize = initInfo.indexCountOpt * sizeof(uint16_t);
        memcpy(mesh->indicesOpt, initInfo.indicesOpt, indicesSize);
    }
    return mesh;
}

MeshToEdit mesh_retainToEditOpt(Mesh*const mesh) {
    if(!(mesh->flags & mesh_flag_mutable)) {
        printerror("Trying to edit non mutable mesh.");
        return (MeshToEdit) {};
    }
    if(mesh->flags & mesh_flag__editing) {
        printerror("Mesh already in edit mode. Forget releaseVertices?");
        return (MeshToEdit) {};
    }
    mesh->flags |= mesh_flag__editing;
    void* verticesToEdit = mesh->vertices0;
    if(mesh->mtl_vertices1Opt) { // Cas Metal avec double vertices et pas de buffer.
        // Si vertices0 sont en lecture, les vertices1 peuvent être édités.
        if(!mesh->mtl_readBuffer1)
            verticesToEdit = mesh->mtl_vertices1Opt;
    }
    return (MeshToEdit) {
        .v =    verticesToEdit,
        .beg =  verticesToEdit,
        .end = (char*)verticesToEdit + mesh->verticesSize,
        .vertexSize = mesh->vertexSize,
        ._mesh = mesh,
    };
}
void       meshtoedit_release(MeshToEdit const meshEdit) {
    Mesh*const mesh = meshEdit._mesh;
    if(!(mesh->flags & mesh_flag_mutable) ||
       !(mesh->flags & mesh_flag__editing)) {
        printerror("Bad vertices release."); return;
    }
    mesh->newIndexCountOpt = meshEdit.indexCount;
    mesh->verticesEdited = meshEdit.beg;
    mesh->flags &= ~ mesh_flag__editing;
}
void  meshref_init(Mesh*const*const meshRef, Mesh*const initValue) {
    if(*meshRef || !initValue) { printerror("Already init or no initValue."); return; }
    *(Mesh**)meshRef = initValue;
}
void  meshref_render_releaseAndNull(Mesh*const*const meshRef) {
    if(!meshRef) { printerror("No mesh ref."); return; }
    Mesh*const toRelease = *meshRef;
    *(Mesh**)meshRef = (Mesh*)NULL;
    if(!toRelease) return; // (ok pas grave, déjà release)
    if(toRelease->flags & mesh_flag_shared) return;
    mesh_render_deinit_(toRelease);
    coq_free(toRelease);
}

// MARK: - "Bar" i.e. `====`
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
    return Mesh_create((MeshInit) {
        .verticesOpt = mesh_bar_vertices_,
        .vertexCount = 8,
        .primitiveType = meshprimitive_triangleStrip,
        .flags = mesh_flag_mutable,
    });
}
Mesh*  Mesh_createVerticalBar(void) {
    return Mesh_create((MeshInit) {
        .verticesOpt = mesh_vbar_vertices_,
        .vertexCount = 8,
        .primitiveType = meshprimitive_triangleStrip,
        .flags = mesh_flag_mutable,
    });
}

// MARK: - Frame (e.g. cadre stretchable)
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
    return Mesh_create((MeshInit) {
        .verticesOpt = mesh_frame_vertices_,
        .vertexCount = 16,
        .indicesOpt = mesh_frame_indices_,
        .indexCountOpt = 54,
        .primitiveType = meshprimitive_triangle,
        .flags = mesh_flag_mutable,
    });
}
void   mesh_frame_setCenterRatios(Mesh*const frame, float const ratioX, float const ratioY) {
    withMeshToEdit_beg(me, frame, Vertex, v)
    float xPos = 0.5f * ratioX;
    v[4].pos.x = -xPos;
    v[5].pos.x = -xPos;
    v[6].pos.x = -xPos;
    v[7].pos.x = -xPos;
    v[8].pos.x =  xPos;
    v[9].pos.x =  xPos;
    v[10].pos.x = xPos;
    v[11].pos.x = xPos;
    float yPos = 0.5f * ratioY;
    v[1].pos.y =   yPos;
    v[5].pos.y =   yPos;
    v[9].pos.y =   yPos;
    v[13].pos.y =  yPos;
    v[2].pos.y =  -yPos;
    v[6].pos.y =  -yPos;
    v[10].pos.y = -yPos;
    v[14].pos.y = -yPos;
    withMeshToEdit_end(me)
}

// MARK: - Fan (disque <-> pointe)
static const uint16_t _mesh_fan_indices[] = {
    0, 1, 2,   0, 2, 3,
    0, 3, 4,   0, 4, 5,
    0, 5, 6,   0, 6, 7,
    0, 7, 8,   0, 8, 9
};
// Point central avec normal vers le haut.
static const Vertex vertex_default_origin_ = {{
    0.f, 0.f, 0.f,  0.5f, 0.5f,  0.f,0.f,1.f
}};
Mesh*  Mesh_createFan(void) {
    Mesh *fan = NULL;
    with_beg(Vertex, vertices, coq_callocSimpleArray(10, Vertex))
    vertices[0] = vertex_default_origin_;  // Centre de la fan en (0, 0).
    Vertex* const end = &vertices[10];
    float i = 0.f;
    for(Vertex* v = &vertices[1]; v < end; v++, i++) {
        v->pos.x = -0.5f * sinf(2.f * M_PI * i / 8.f);
        v->pos.y =  0.5f * cosf(2.f * M_PI * i / 8.f);
        v->uv.x = 0.5f - 0.5f * sinf(2.f * M_PI * i / 8.f);
        v->uv.y = 0.5f - 0.5f * cosf(2.f * M_PI * i / 8.f);
        v->norm.z = 1.f;
    }
    fan = Mesh_create((MeshInit) {
        .verticesOpt = vertices,
        .vertexCount = 10,
        .indicesOpt = _mesh_fan_indices,
        .indexCountOpt = 24,
        .primitiveType = meshprimitive_triangle,
        .flags = mesh_flag_mutable,
    });
    with_end(vertices)
    return fan;
}
void   mesh_fan_update(Mesh* fan, float ratio) {
    if(fan->vertexCount < 10) {
        printerror("Fan mesh with vertex_count < 10.");
        return;
    }
    withMeshToEdit_beg(me, fan, Vertex, vertices)
    Vertex* const end = &vertices[10];
    float i = 1.f;
    for(Vertex* v = &vertices[2]; v < end; v++, i++) { // (On skip le premier, reste a (0, 0.5).)
        v->pos.x = -0.5f * sinf(ratio * 2.f * M_PI * i / 8.f);
        v->pos.y =  0.5f * cosf(ratio * 2.f * M_PI * i / 8.f);
        v->uv.x = 0.5f - 0.5f * sinf(ratio * 2.f * M_PI * i / 8.f);
        v->uv.y = 0.5f - 0.5f * cosf(ratio * 2.f * M_PI * i / 8.f);
    }
    withMeshToEdit_end(me)
}

// MARK: - Graphique
/// Les xs/ys devraient être préformaté pour être contenu dans [-0.5, 0.5] x [-0.5, 0.5]...
/// delta: épaisseur des lignes.
/// ratio: étirement en largeur.
Mesh*  Mesh_createPlot(float* xs, float* ys, uint32_t count, float delta, float ratio) {
    uint32_t n_lines = count - 1;
    uint32_t n_points = count;
    uint32_t vertices_count = 4*n_lines + 4*n_points;
    uint32_t indices_count =  6*n_lines + 6*n_points;
    Mesh* plot = NULL;
    with_beg(Vertex, vertices, coq_callocSimpleArray(vertices_count, Vertex))
    with_beg(uint16_t, indices, coq_callocSimpleArray(indices_count, uint16_t))
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
    plot = Mesh_create((MeshInit) {
        .verticesOpt = vertices,
        .vertexCount = vertices_count,
        .indicesOpt = indices,
        .indexCountOpt = indices_count,
        .primitiveType = meshprimitive_triangle,
    });
    with_end(indices)
    with_end(vertices)
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
    Mesh* plotGrid = Mesh_create((MeshInit) {
        .verticesOpt = vertices,
        .vertexCount = vertices_count,
        .indicesOpt = indices,
        .indexCountOpt = indices_count,
        .primitiveType = meshprimitive_triangle,
    });
    coq_free(vertices);
    coq_free(indices);
    return plotGrid;
}

#define Mesh_chunck_count_ 256
#define Mesh_obj_lineMax_  128
//typedef struct {
//    void* array;
//    size_t count;
//    size_t countMax;
//    size_t elSize;
//} FillArray;
//FillArray FillArray_new(size_t elementSize) {
//    return (FillArray) {
//        .array = coq_malloc(Mesh_chunck_count_*elementSize),
//        .countMax = Mesh_chunck_count_,
//        .elSize = elementSize,
//    };
//}
//void fillarray_add(FillArray* fa, void* addedEl) {
//}
typedef struct {
    uint16_t v, t, n;
} VTNindex;

Mesh*  Mesh_createFromObjFile(char const*const path)
{
    if(!path) {  printerror("No path to open."); return NULL; }
    guard_let(FILE*, f, fopen(path, "r"), printerror("Cannot open %s.", path), NULL)
    Vector3* positions = coq_malloc(Mesh_chunck_count_*sizeof(Vector3));
    size_t   positions_count = 0;
    size_t   positions_maxCount = Mesh_chunck_count_;
    Vector3* normals = coq_malloc(Mesh_chunck_count_*sizeof(Vector3));
    size_t   normals_count = 0;
    size_t   normals_maxCount = Mesh_chunck_count_;
    Vector2* uvs = coq_malloc(Mesh_chunck_count_*sizeof(Vector2));
    size_t   uvs_count = 0;
    size_t   uvs_maxCount = Mesh_chunck_count_;
    VTNindex* indices = coq_malloc(Mesh_chunck_count_*sizeof(VTNindex));
    size_t   indices_count = 0;
    size_t   indices_maxCount = Mesh_chunck_count_;
    
    char line[Mesh_obj_lineMax_];
    char first[32];
    Vector3 tmp;
    VTNindex ind0, ind1, ind2;
    // Lecture du fichier obj.
    while(fgets(line, Mesh_obj_lineMax_, f)) {
        sscanf(line, "%s", first);
        if(strcmp(first, "v") == 0) {
            sscanf(line, "%s %f %f %f", first, &tmp.x, &tmp.y, &tmp.z);
            positions[positions_count] = tmp;
            positions_count++;
            if(positions_count >= positions_maxCount) {
                positions_maxCount += Mesh_chunck_count_;
                positions = coq_realloc(positions, positions_maxCount*sizeof(Vector3));
            }
            continue;
        }
        if(strcmp(first, "vn") == 0) {
            sscanf(line, "%s %f %f %f", first, &tmp.x, &tmp.y, &tmp.z);
            normals[normals_count] = tmp;
            normals_count++;
            if(normals_count >= normals_maxCount) {
                normals_maxCount += Mesh_chunck_count_;
                normals = coq_realloc(normals, normals_maxCount*sizeof(Vector3));
            }
            continue;
        }
        if(strcmp(first, "vt") == 0) {
            sscanf(line, "%s %f %f", first, &tmp.x, &tmp.y);
            uvs[uvs_count] = tmp.xy;
            uvs_count++;
            if(uvs_count >= uvs_maxCount) {
                uvs_maxCount += Mesh_chunck_count_;
                uvs = coq_realloc(uvs, uvs_maxCount*sizeof(Vector2));
            }
            continue;
        }
        if(strcmp(first, "f") == 0) {
            sscanf(line, "%s %hd/%hd/%hd %hd/%hd/%hd %hd/%hd/%hd", first,
                &ind0.v, &ind0.t, &ind0.n,
                &ind1.v, &ind1.t, &ind1.n,
                &ind2.v, &ind2.t, &ind2.n);
            indices[indices_count] = ind0;
            indices[indices_count+1] = ind1;
            indices[indices_count+2] = ind2;
            indices_count += 3;
            if(indices_count+2 >= indices_maxCount) {
                indices_maxCount += Mesh_chunck_count_;
                indices = coq_realloc(indices, indices_maxCount*sizeof(VTNindex));
            }
            continue;
        }
    }
    fclose(f);
    
    // Version indexé avec les coord. uv.
    Mesh* mesh = NULL;
    Vertex*   mesh_vertices = NULL;
    uint16_t* mesh_indices = NULL;
    if(!uvs_count || !indices_count) {
        printerror("No vertex ?");
        goto release_and_return_mesh;
    }
    mesh_vertices = coq_callocSimpleArray(uvs_count, Vertex);
    mesh_indices = coq_callocSimpleArray(indices_count, uint16_t);
    for(size_t i = 0; i < indices_count; i++) {
        VTNindex const ind = indices[i];
        uint16_t index = ind.t-1;
        mesh_vertices[index] = (Vertex) {
            .pos = positions[ind.v-1],
            .norm = normals[ind.n-1],
            .uv = uvs[index],
        };
        mesh_indices[i] = index;
        
    }
    mesh = Mesh_create((MeshInit) {
        .verticesOpt = mesh_vertices,
        .vertexCount = (uint32_t)uvs_count,
        .indicesOpt = mesh_indices,
        .indexCountOpt = (uint32_t)indices_count,
        .primitiveType = meshprimitive_triangle,
    });

release_and_return_mesh:
    if(mesh_vertices) coq_free(mesh_vertices);
    if(mesh_indices)  coq_free(mesh_indices);
    coq_free(positions);
    coq_free(normals);
    coq_free(uvs);
    coq_free(indices);
    return mesh;
}


// MARK: - Courbe
// TODO: Compléter "Curve".
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
        printerror("Mesh curve with less than 2 pos."); return NULL;
    }
    uint32_t vertex_count = 4*(v_count - 1);
    Mesh *curve = NULL;
    with_beg(Vertex, vertices, coq_callocSimpleArray(vertex_count, Vertex))
    // Init des deux premier vertex a gauche. (up-left et down-left).
    Vector2 v0 = v_arr[0], v1 = v_arr[1];
    Vector2 v01 = vector2_minus(v1, v0);
    float   n01 = vector2_norm(v01);
    Vector2 v01d = vector2_times(vector2_cross(v01), delta / n01);
    Vector2 v01u = vector2_times(v01d, -1.f);
    Vector2 vul = vector2_add(v0, v01u);
    Vector2 vdl = vector2_add(v0, v01d);
    // Reste des vertex
    Vertex *v = vertices;
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
        (*v) = _vertex_uv00;
        (*v).pos.x = vul.x; (*v).pos.y = vul.y;
        v++;
        (*v) = _vertex_uv01;
        (*v).pos.x = vdl.x; (*v).pos.y = vdl.y;
        v++;
        (*v) = _vertex_uv10;
        (*v).pos.x = vur.x; (*v).pos.y = vur.y;
        v++;
        (*v) = _vertex_uv11;
        (*v).pos.x = vdr.x; (*v).pos.y = vdr.y;
        v++;
        vul = vul_next;
        vdl = vdl_next;
    }
    curve = Mesh_create((MeshInit) {
        .verticesOpt = vertices,
        .vertexCount = vertex_count,
        .primitiveType = meshprimitive_triangleStrip,
        .flags = mesh_flag_mutable,
    });
    with_end(vertices)
    return curve;
}
