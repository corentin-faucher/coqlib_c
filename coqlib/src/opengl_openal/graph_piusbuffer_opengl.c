//
//  graph_piusbuffer_opengl.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-07.
//

#include "../utils/utils_base.h"
#include "../graphs/graph_base.h"
#include "../graphs/graph__opengl.h"

#define PIUB_MaxInstances 500

static GLuint PIUB_uniformBlockId_ = 0;
static GLuint PIUB_bindingPoint_ = 0;
static GLuint PIUB_bufferId_ = 0;

void   PIUsBuffer_GLinit(GLuint program) {
    PIUB_uniformBlockId_ = glGetUniformBlockIndex(program, "InstanceBufferType");
    glUniformBlockBinding(program, PIUB_uniformBlockId_, PIUB_bindingPoint_);
    glGenBuffers(1, &PIUB_bufferId_);
    glBindBuffer(GL_UNIFORM_BUFFER, PIUB_bufferId_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstanceUniforms)*PIUB_MaxInstances, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, PIUB_bindingPoint_, PIUB_bufferId_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void perinstanceuniform_glBind(const PerInstanceUniforms* piu) {
    glBindBuffer(GL_UNIFORM_BUFFER, PIUB_bufferId_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerInstanceUniforms), piu);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/// Création du buffer. 
void   piusbuffer_init(PIUsBuffer* piusbuffer, size_t size) {
    piusbuffer->pius = coq_calloc(1, size);
    piusbuffer->size = size;
//    glGenBuffers(1, &piusbuffer->glBufferId);
//    glBindBuffer(GL_UNIFORM_BUFFER, piusbuffer->glBufferId);
//    glBufferData(GL_UNIFORM_BUFFER, size, piusbuffer->pius, GL_DYNAMIC_DRAW);
//    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
/// Libère l'espace du buffer (et array de piu si nécessaire)
void   piusbuffer_deinit(PIUsBuffer* piusbuffer) {
//    glDeleteBuffers(1, &piusbuffer->glBufferId);
    coq_free(piusbuffer->pius);
    piusbuffer->pius = NULL;
    piusbuffer->size = 0;
}

void  piusbuffer_glBind(const PIUsBuffer* piusbuffer) {
    size_t size = piusbuffer->size;
    if(size > PIUB_MaxInstances * sizeof(PerInstanceUniforms)) {
        size = PIUB_MaxInstances * sizeof(PerInstanceUniforms);
        printwarning("Max instances %d.", PIUB_MaxInstances);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, PIUB_bufferId_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, piusbuffer->pius);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

//    glBindBuffer(GL_UNIFORM_BUFFER, piusbuffer->glBufferId);
//    PerInstanceUniforms* pius = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
//    memcpy(pius, piusbuffer->pius, piusbuffer->size);
//    glUnmapBuffer(GL_UNIFORM_BUFFER);
}
