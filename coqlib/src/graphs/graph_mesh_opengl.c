//
//  graph_mesh_opengl.c
//  Implementation openGL des mesh.
//
//  Created by Corentin Faucher on 2023-10-29.
//

#include "graph__opengl.h"
#include "graph_mesh_private.h"
#include "../utils/util_base.h"

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

// Par défaut on utilise les `Vertex` de `graph_mesh.h`
// avec location=0 -> position,
// location=1 -> coord. uv,
// location=2 -> vect. normal.
void mesh_opengl_defaultSetVertexAttributes_(void) {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);
}
void (*mesh_opengl_setVertexAttributes_)(void) = NULL;
void   Mesh_opengl_initSetVertexAttributes(void (*setVertexAttributesOpt)(void)) {
    if(mesh_opengl_setVertexAttributes_) {
        printerror("mesh_opengl_setVertexAttributes already set.");
        return;
    }
    if(!setVertexAttributesOpt) {
    // Cas avec les `Vertex` par défaut.
        mesh_opengl_setVertexAttributes_ = mesh_opengl_defaultSetVertexAttributes_;
    } else {
        mesh_opengl_setVertexAttributes_ = setVertexAttributesOpt;
    }
}

void   mesh_render_deinit_(Mesh*const mesh) {
    if(mesh->indicesOpt) {
        coq_free(mesh->indicesOpt);
        mesh->indicesOpt = NULL;
    }
    if(mesh->glVertexBuffer0Id)
        glDeleteBuffers(1, &mesh->glVertexBuffer0Id);
//    if(mesh->glVertexBuffer1Id)
//        glDeleteBuffers(1, &mesh->glVertexBuffer1Id);
    if(mesh->glIndicesBufferId)
        glDeleteBuffers(1, &mesh->glIndicesBufferId);
    if(mesh->glVertexArrayId)
        glDeleteVertexArrays(1, &mesh->glVertexArrayId);
    mesh->glVertexBuffer0Id = 0;
//    mesh->glVertexBuffer1Id = 0;
    mesh->glVertexArrayId = 0;
    mesh->glIndicesBufferId = 0;
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

void       mesh_render_checkMeshInit(Mesh*const mesh) {
    if(mesh->glVertexArrayId) return;
    if(!mesh_opengl_setVertexAttributes_) {
        printerror("Function mesh_opengl_setVertexAttributes undefined. Missing call to Mesh_opengl_initVertexAttributes?");
        return;
    }
    // Génération des buffers (data des indices et vertex)
    glGenBuffers(1, &mesh->glVertexBuffer0Id);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->glVertexBuffer0Id);
    glBufferData(GL_ARRAY_BUFFER, mesh->verticesSize, mesh->vertices0, GL_STATIC_DRAW);
    if(mesh->maxIndexCount && mesh->indicesOpt) {
        glGenBuffers(1, &mesh->glIndicesBufferId);
    } else if(mesh->indicesOpt || mesh->maxIndexCount) {
        printwarning("Missing indices array or indexCount.");
    }
        
    // Initialisation du vertex array object (VAO)
    // -> Infos où trouver les (positions, coord. uv, ...), i.e. vertex,
    //  et indices d'une mesh.
    glGenVertexArrays(1, &mesh->glVertexArrayId);
    glBindVertexArray(mesh->glVertexArrayId);
    // Liaison des indices au VAO. 
    // ** Attention à `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,...)`,
    //    s'attache au VAO présentement bindé **
    if(mesh->glIndicesBufferId) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->glIndicesBufferId);
        size_t const indicesSize = mesh->maxIndexCount * sizeof(uint16_t);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, mesh->indicesOpt, GL_STATIC_DRAW);
        coq_free(mesh->indicesOpt);
        mesh->indicesOpt = NULL;
    }
    // Liaison/binding des vertex `attributes` (position, uv, ...)
    // dans le VAO avec `glVertexAttribPointer` et `glEnableVertexAttribArray`.
    // Les pointeurs sont relatif au Vertex buffer présentement bindé (mesh->glVertexBuffer0Id).
    mesh_opengl_setVertexAttributes_();
    // Binder à zéro (pour éviter qu'un buffer futur quelconque vienne se binder à ce VAO).
    glBindVertexArray(0); 
}

/// Obtenir les vertex disponible pour être dessiner.
void       mesh_render_checkMeshUpdate(Mesh*const mesh) {
    if(!mesh->verticesEdited) return;
    float*const edited = mesh->verticesEdited;
    mesh->verticesEdited = NULL;
    if(!(mesh->flags & mesh_flag_mutable)) {
        printerror("Non mutable mesh with edited vertices.");
        return;
    }
    if(mesh->newIndexCountOpt) {
        uint_initConst(&mesh->actualIndexCount, uminu(mesh->newIndexCountOpt, mesh->maxIndexCount));
        mesh->newIndexCountOpt = 0;
    }
    if(!mesh->glVertexBuffer0Id) { // || !mesh->glVertexBuffer1Id) {
        printwarning("Buffer still not init?");
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh->glVertexBuffer0Id);
    void*const verticesDst = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    memcpy(verticesDst, edited, mesh->verticesSize);
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

/// Vérifie s'il faut initialiser le buffer.
/// Dans le cas d'une mesh mutable, vérifier s'il y a eu mise à jour des vertices.
MeshToDraw mesh_render_getToDraw(Mesh const*const mesh) {
    return (MeshToDraw) {
        .vertexCount =    mesh->vertexCount,
        .verticesSize =   mesh->verticesSize,
        .indexCount =     mesh->actualIndexCount,
        .cull_mode =      mesh->cull_mode,
        // OpenGL specific
        .glVertexArrayId = mesh->glVertexArrayId,
        .glPrimitiveType = Mesh_GLprimitiveOfMyPrimitive_[mesh->primitive_type],
    };
}

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
