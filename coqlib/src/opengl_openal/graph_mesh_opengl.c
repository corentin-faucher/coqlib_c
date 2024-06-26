//
//  graph_mesh_opengl.c
//  Implementation openGL des mesh.
//
//  Created by Corentin Faucher on 2023-10-29.
//


#include "../graphs/graph__opengl.h"
#include "../utils/util_base.h"

// #include <stddef.h>
// #include <stdint.h>
// #include <stdbool.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

typedef struct Mesh {
    uint32_t  vertex_count;       // Le nombre de vertex.
    size_t    vertices_size;      // La taille en bytes de l'array vertices.
    uint32_t  index_count;        // 0 si triangle strip (pas besoin d'indices de vertex).
    uint16_t  primitive_type;
    uint16_t  cull_mode;
    bool      isShared;

    // Ids OpenGL
    GLuint    vertex_array_id;   // VAO
    GLuint    vertex_buffer_id;  // VBO
    GLuint    indices_buffer_id; // VBO des indices
    bool      need_to_update_vertices;

    Vertex    vertices[1];         // Array des vertex. A LA FIN, fait varier la taille.
} Mesh;

static Vertex mesh_sprite_vertices_[4] = {
    {-0.5, 0.5, 0, 0.0001, 0.0001, 0,0,1},
    {-0.5,-0.5, 0, 0.0001, 0.9999, 0,0,1},
    { 0.5, 0.5, 0, 0.9999, 0.0001, 0,0,1},
    { 0.5,-0.5, 0, 0.9999, 0.9999, 0,0,1},
};
//static Vertex mesh_sprite_vertices_[4] = {
//    {-0.5, 0.5, 0, 0,0, 0,0,1},
//    {-0.5,-0.5, 0, 0,1, 0,0,1},
//    { 0.5, 0.5, 0, 1,0, 0,0,1},
//    { 0.5,-0.5, 0, 1,1, 0,0,1},
//};
Mesh*  mesh_sprite = NULL;

static GLuint Mesh_in_position_id_ = 0;
static GLuint Mesh_in_uv_id_ = 0;
//static GLuint Mesh_in_normal_id_ = 0;

void Mesh_init(GLuint program) {
    Mesh_in_position_id_ = glGetAttribLocation(program, "in_position");
    Mesh_in_uv_id_ =       glGetAttribLocation(program, "in_uv");
//    Mesh_in_normal_id_ =   glGetAttribLocation(program, "in_normal");

    // Init de la sprite.
    mesh_sprite = Mesh_createEmpty(mesh_sprite_vertices_, 4, NULL, 0,
                            mesh_primitive_triangleStrip, mesh_cullMode_none, true);
    mesh_sprite->need_to_update_vertices = true;
}

Mesh*  Mesh_createEmpty(const Vertex* const verticesOpt, uint32_t vertexCount,
                        const uint16_t* const indicesOpt, uint32_t indexCount,
                        enum MeshPrimitiveType primitive_type,
                        enum MeshCullMode cull_mode, bool isShared) {
    size_t mesh_size = sizeof(Mesh) + sizeof(Vertex) * (vertexCount - 1);
    size_t vertices_size = vertexCount * sizeof(Vertex);
    Mesh* mesh = coq_calloc(1, mesh_size);

    mesh->vertex_count = vertexCount;
    mesh->vertices_size = vertices_size;
    mesh->index_count = indexCount;
    mesh->primitive_type = primitive_type;
    mesh->cull_mode = cull_mode;
    mesh->isShared = isShared;

    if(verticesOpt) {
        memcpy(mesh->vertices, verticesOpt, mesh->vertices_size);
    }
    glGenVertexArrays(1, &mesh->vertex_array_id);
    glGenBuffers(1, &mesh->vertex_buffer_id);
    if(indexCount && indicesOpt) {
        glGenBuffers(1, &mesh->indices_buffer_id);
    } else if(indicesOpt || indexCount) {
        printwarning("Missing indices array or indexCount.");
    }
    glBindVertexArray(mesh->vertex_array_id);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, vertices_size, mesh->vertices, GL_STATIC_DRAW);
    if(mesh->indices_buffer_id) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_buffer_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indexCount, indicesOpt, GL_STATIC_DRAW);
    }
    glVertexAttribPointer(Mesh_in_position_id_, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), 0);
    glEnableVertexAttribArray(Mesh_in_position_id_);
    glVertexAttribPointer(Mesh_in_uv_id_, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), BUFFER_OFFSET(sizeof(GL_FLOAT)*3));
    glEnableVertexAttribArray(Mesh_in_uv_id_);
    
//    glVertexAttribPointer(_Mesh_in_normal_id, 3, GL_FLOAT, GL_FALSE,
//            sizeof(Vertex), BUFFER_OFFSET(sizeof(GL_FLOAT)*5));
//    glEnableVertexAttribArray(_Mesh_in_normal_id);

    return mesh;
}
void   mesh_destroyAndNull(Mesh** const meshRef) {
    if(*meshRef == NULL) return;
    if((*meshRef)->vertex_buffer_id)
        glDeleteBuffers(1, &(*meshRef)->vertex_buffer_id);
    if((*meshRef)->indices_buffer_id)
        glDeleteBuffers(1, &(*meshRef)->indices_buffer_id);
    if((*meshRef)->vertex_array_id)
        glDeleteVertexArrays(1, &(*meshRef)->vertex_array_id);
    coq_free(*meshRef);  // (free aussi les vertices)
    *meshRef = NULL;
}
void  mesh_glBind(Mesh* mesh) {
    if(mesh->need_to_update_vertices) {
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer_id);
      Vertex* vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
      memcpy(vertices, mesh->vertices, mesh->vertices_size);
      glUnmapBuffer(GL_ARRAY_BUFFER);
      mesh->need_to_update_vertices = false;
    }
    glBindVertexArray(mesh->vertex_array_id);
}
Vertex* mesh_vertices(Mesh* mesh) {
    return mesh->vertices;
}
void    mesh_needToUpdateVertices(Mesh* mesh) {
    mesh->need_to_update_vertices = true;
}
uint32_t mesh_indexCount(Mesh* mesh) {
    return mesh->index_count;
}
uint32_t mesh_vertexCount(Mesh* mesh) {
    return mesh->vertex_count;
}
size_t   mesh_verticesSize(Mesh* mesh) {
    return mesh->vertices_size;
}
enum MeshPrimitiveType mesh_primitiveType(Mesh* mesh) {
    return mesh->primitive_type;
}
enum MeshCullMode      mesh_cullMode(Mesh* mesh) {
    return mesh->cull_mode;
}
bool     mesh_isShared(Mesh* mesh) {
    return mesh->isShared;
}
