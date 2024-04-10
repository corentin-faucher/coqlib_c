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
void   piusbuffer_init_(PIUsBuffer* piusbuffer, uint32_t count) {
    if(count > PIUB_MaxInstances) {
        printerror("With OpenGL, the maximum number of instance is %d.", PIUB_MaxInstances);
        count = PIUB_MaxInstances;
    }
    size_t size = count * sizeof(PerInstanceUniforms);
    *piusbuffer = (PIUsBuffer) {
        count, count, size,
        coq_calloc(1, size), NULL
    };
//    glGenBuffers(1, &piusbuffer->glBufferId);
//    glBindBuffer(GL_UNIFORM_BUFFER, piusbuffer->glBufferId);
//    glBufferData(GL_UNIFORM_BUFFER, size, piusbuffer->pius, GL_DYNAMIC_DRAW);
//    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
/// Libère l'espace du buffer (et array de piu si nécessaire)
void   piusbuffer_deinit_(PIUsBuffer* piusbuffer) {
    coq_free(piusbuffer->pius);
    *piusbuffer = (PIUsBuffer) { 0 };
}

void  piusbuffer_glBind(PIUsBuffer* piusbuffer) {
    if(piusbuffer->actual_count > piusbuffer->max_count) {
        printerror("Overflow..."); 
        piusbuffer->actual_count = piusbuffer->max_count;
    }
    size_t size = piusbuffer->actual_count * sizeof(PerInstanceUniforms);
    glBindBuffer(GL_UNIFORM_BUFFER, PIUB_bufferId_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, piusbuffer->pius);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
