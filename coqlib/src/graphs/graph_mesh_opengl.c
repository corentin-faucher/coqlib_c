//
//  graph_mesh_opengl.c
//  Implementation openGL des mesh.
//
//  Created by Corentin Faucher on 2023-10-29.
//


#include "graph__opengl.h"
#include "../utils/util_base.h"

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static GLuint Mesh_in_position_id_ = 0;
static GLuint Mesh_in_uv_id_ = 0;
//static GLuint Mesh_in_normal_id_ = 0;

#pragma mark - OpenGL Specific
void Mesh_opengl_init_(GLuint program) {
    Mesh_in_position_id_ = glGetAttribLocation(program, "in_position");
    Mesh_in_uv_id_ =       glGetAttribLocation(program, "in_uv");
//    Mesh_in_normal_id_ =   glGetAttribLocation(program, "in_normal");
}
void  mesh_glBind(Mesh *const mesh) {
    if(mesh->_needToUpdateVertices) {
        size_t const verticesSize = mesh->vertex_count * sizeof(Vertex);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->_glVertexBufferId);
        Vertex* vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
        memcpy(vertices, mesh->_verticesCopy, verticesSize);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        mesh->_needToUpdateVertices = false;
    }
    glBindVertexArray(mesh->_glVertexArrayId);
}

#pragma mark - Dépendant de l'engine
void mesh_engine_initBuffers_(Mesh* const mesh, const Vertex* const verticesOpt, const uint16_t* const indicesOpt) {
    glGenVertexArrays(1, &mesh->_glVertexArrayId);
    glGenBuffers(1, &mesh->_glVertexBufferId);
    if(mesh->index_count && indicesOpt) {
        glGenBuffers(1, &mesh->_glIndicesBufferId);
    } else if(indicesOpt || mesh->index_count) {
        printwarning("Missing indices array or indexCount.");
    }
    size_t const verticesSize = mesh->vertex_count * sizeof(Vertex);
    glBindVertexArray(mesh->_glVertexArrayId);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_glVertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, verticesOpt, GL_STATIC_DRAW);
    if(mesh->_glIndicesBufferId) {
        size_t indicesSize = mesh->index_count * sizeof(uint16_t);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_glIndicesBufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indicesOpt, GL_STATIC_DRAW);
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

    // Garder une copie des vertices pour édition...
    mesh->_verticesCopy = coq_calloc(1, verticesSize);
    if(verticesOpt)
        memcpy(mesh->_verticesCopy, verticesOpt, verticesSize);
}
void   mesh_engine_deinit(Mesh* mesh) {
    if(mesh->_glVertexBufferId)
        glDeleteBuffers(1, &mesh->_glVertexBufferId);
    if(mesh->_glIndicesBufferId)
        glDeleteBuffers(1, &mesh->_glIndicesBufferId);
    if(mesh->_glVertexArrayId)
        glDeleteVertexArrays(1, &mesh->_glVertexArrayId);
    coq_free(mesh->_verticesCopy);
    mesh->_verticesCopy = NULL;
}
/// Référence des vertices pour mise à jour des positions.
/// Caller `mesh_releaseVertices` une fois fini.
Vertex*  mesh_engine_retainVertices(Mesh* mesh) {
    if(mesh->_needToUpdateVertices) {
        printwarning("Too many update ?");  // Pas très grave... (Je crois)
        mesh->_needToUpdateVertices = false;
    }
    return mesh->_verticesCopy;
}
/// Fini d'éditer les vertex.
void     mesh_engine_releaseVertices(Mesh* mesh) {
    mesh->_needToUpdateVertices = true;
}





#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
