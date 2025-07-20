//
//  graph_mesh_opengl.c
//  Implementation openGL des mesh.
//
//  Created by Corentin Faucher on 2023-10-29.
//

#include "graph__opengl.h"
#include "graph_mesh_private.h"
#include "../utils/util_base.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
// Vertex attribute locations... (position, uv, color, normal... d'un vertex)
static GLuint Mesh_in_position_id_ = 0;
static GLuint Mesh_in_uv_id_ = 0;

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

//static GLuint Mesh_in_normal_id_ = 0;
void Mesh_opengl_defaultInitVertexAttrLoc_(GLuint program) {
    Mesh_in_position_id_ = glGetAttribLocation(program, "in_position");
    Mesh_in_uv_id_ =       glGetAttribLocation(program, "in_uv");
//    Mesh_in_normal_id_ =   glGetAttribLocation(program, "in_normal");
}
void (*Mesh_opengl_initVertexAttributeLocations)(GLuint program) = Mesh_opengl_defaultInitVertexAttrLoc_;

void mesh_opengl_defaultSetVertexAttributes_(Mesh const* mesh) {
    glVertexAttribPointer(Mesh_in_position_id_, 3, GL_FLOAT, GL_FALSE,
            mesh->vertexSize, 0);
    glEnableVertexAttribArray(Mesh_in_position_id_);
    glVertexAttribPointer(Mesh_in_uv_id_, 2, GL_FLOAT, GL_FALSE,
            mesh->vertexSize, BUFFER_OFFSET(sizeof(float)*3));
    glEnableVertexAttribArray(Mesh_in_uv_id_);
//    glVertexAttribPointer(_Mesh_in_normal_id, 3, GL_FLOAT, GL_FALSE,
//            sizeof(Vertex), BUFFER_OFFSET(sizeof(GL_FLOAT)*5));
//    glEnableVertexAttribArray(_Mesh_in_normal_id);
}
void (*mesh_opengl_setVertexAttributes)(Mesh const* mesh) = mesh_opengl_defaultSetVertexAttributes_;


void mesh_engine_initBuffers_(Mesh* const mesh, const void* const verticesOpt,
                              const uint16_t* const indicesOpt)
{
    if(mesh->_glVertexBufferId) {
        printerror("Vertices buffer already init.");
    }
    // Génération des buffers
    glGenVertexArrays(1, &mesh->_glVertexArrayId);
    glGenBuffers(1, &mesh->_glVertexBufferId);
    if(mesh->actualIndexCount && indicesOpt) {
        glGenBuffers(1, &mesh->_glIndicesBufferId);
    } else if(indicesOpt || mesh->actualIndexCount) {
        printwarning("Missing indices array or indexCount.");
    }
    // Binding avec vertices
    size_t const verticesSize = mesh->vertexCount * mesh->vertexSize;
    glBindVertexArray(mesh->_glVertexArrayId);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_glVertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, verticesOpt, GL_STATIC_DRAW);
    if(mesh->_glIndicesBufferId) {
        size_t indicesSize = mesh->actualIndexCount * sizeof(uint16_t);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_glIndicesBufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indicesOpt, GL_STATIC_DRAW);
    }
    // Setter le mapping des attributs d'un vertex (pos, uv, color, normal...)
    // Customizable en fonction de la structure des Vertex.
    mesh_opengl_setVertexAttributes(mesh);
}
void   mesh_engine_deinit_(Mesh*const mesh) {
    if(mesh->_glVertexBufferId)
        glDeleteBuffers(1, &mesh->_glVertexBufferId);
    if(mesh->_glIndicesBufferId)
        glDeleteBuffers(1, &mesh->_glIndicesBufferId);
    if(mesh->_glVertexArrayId)
        glDeleteVertexArrays(1, &mesh->_glVertexArrayId);
    mesh->_glVertexBufferId = 0;
    mesh->_glVertexArrayId = 0;
    mesh->_glIndicesBufferId = 0;
}
void   mesh_render_tryToUpdateVerticesAndIndiceCount(Mesh *const mesh) {
    if(!(mesh->flags & mesh_flag_mutable)) return;
    if(!(mesh->flags & mesh_flag__needUpdate)) return;
    size_t const verticesSize = mesh->vertexCount * mesh->vertexSize;
    // Copier les nouveaux vertex
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_glVertexBufferId);
    void* vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    memcpy(vertices, mesh->verticesEdit, verticesSize);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    // Nouveau nombre d'indices actifs ?
    if(mesh->newIndexCountOpt) {
        uint_initConst(&mesh->actualIndexCount, uminu(mesh->newIndexCountOpt, mesh->maxIndexCount));
        mesh->newIndexCountOpt = 0;
    }
    // Ok, fini d'updater
    mesh->flags &= ~mesh_flag__needUpdate;
}

// De "gl3.h" :
//#define GL_POINTS                         0x0000
//#define GL_LINES                          0x0001
//#define GL_LINE_LOOP                      0x0002
//#define GL_LINE_STRIP                     0x0003
//#define GL_TRIANGLES                      0x0004
//#define GL_TRIANGLE_STRIP                 0x0005
//#define GL_TRIANGLE_FAN                   0x0006
static uint32_t Mesh_GLprimitiveOfMyPrimitive_[] = {
    0, // points
    1, // lines
    3, // lineStrip (skip line_loop)
    4, // triangles
    5, // triangleStrip
};
MeshToDraw mesh_render_getMeshToDraw(Mesh const*const mesh) {
    return (MeshToDraw) {
        .vertexCount =    mesh->vertexCount,
        .verticesSize =   mesh->verticesSize,
        .indexCount =     mesh->actualIndexCount,
        .cull_mode =      mesh->cull_mode,
        .glVertexArrayId = mesh->_glVertexArrayId,
        .glVertexBufferId = mesh->_glVertexBufferId,
        .glIndicesBufferId = mesh->_glIndicesBufferId,
        .glPrimitiveType = Mesh_GLprimitiveOfMyPrimitive_[mesh->primitive_type],
    };
}

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
