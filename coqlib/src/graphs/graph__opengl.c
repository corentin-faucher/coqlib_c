//
//  graph__opengl.c
//  Pour utiliser OpenGL (au lieu de Metal)
//
//  Created by Corentin Faucher on 2024-10-03.
//

#include "graph__opengl.h"

#include "../utils/util_base.h"
#include "../utils/util_file.h"
#include "../utils/util_string.h"
#include <OpenGL/OpenGL.h>


#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#pragma mark - Instance Uniforms buffer -
// IUB : Instance uniforms buffer, tempon pour passer un packet de InstanceUniforms.
#define IUB_MaxInstances 500

static GLuint IUB_uniformBlockId_ = 0;
static GLuint IUB_bindingPoint_ = 0;
static GLuint IUB_bufferId_ = 0;

void IUsBuffer_opengl_init_(GLuint program) {
    IUB_uniformBlockId_ = glGetUniformBlockIndex(program, "InstanceBufferType");
    glUniformBlockBinding(program, IUB_uniformBlockId_, IUB_bindingPoint_);
    glGenBuffers(1, &IUB_bufferId_);
    glBindBuffer(GL_UNIFORM_BUFFER, IUB_bufferId_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(InstanceUniforms)*IUB_MaxInstances, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, IUB_bindingPoint_, IUB_bufferId_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


}
void IUsBuffer_opengl_deinit_(void) {
    glDeleteBuffers(1, &IUB_bufferId_);
}
void instanceuniform_glBind(const InstanceUniforms* iu) {
    glBindBuffer(GL_UNIFORM_BUFFER, IUB_bufferId_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(InstanceUniforms), iu);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
/// Création du buffer.
void  iusbuffer_engine_init_(IUsBuffer* iusbuffer, uint32_t count) {
    if(count > IUB_MaxInstances) {
        printerror("With OpenGL, the maximum number of instance is %d.", IUB_MaxInstances);
        count = IUB_MaxInstances;
    }
    uint_initConst(&iusbuffer->max_count, count);
    iusbuffer->actual_count = count;
    *(InstanceUniforms**)&iusbuffer->ius = coq_calloc(count, sizeof(InstanceUniforms));
//    glGenBuffers(1, &piusbuffer->glBufferId);
//    glBindBuffer(GL_UNIFORM_BUFFER, piusbuffer->glBufferId);
//    glBufferData(GL_UNIFORM_BUFFER, size, piusbuffer->pius, GL_DYNAMIC_DRAW);
//    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
/// Libère l'espace du buffer (et array de piu si nécessaire)
void  iusbuffer_engine_deinit_(IUsBuffer* iusbuffer) {
    coq_free(iusbuffer->ius);
    *(InstanceUniforms**)&iusbuffer->ius = NULL;
}
void  iusbuffer_glBind(IUsBuffer* iusbuffer) {
    if(iusbuffer->actual_count > iusbuffer->max_count) {
        printerror("Overflow...");
        iusbuffer->actual_count = iusbuffer->max_count;
    }
    size_t size = iusbuffer->actual_count * sizeof(InstanceUniforms);
    glBindBuffer(GL_UNIFORM_BUFFER, IUB_bufferId_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, iusbuffer->ius);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

#pragma mark - CoqGraph_Init, Init du stuff graphique : buffer, mesh, texture...
static bool isOpenGL_3_1_ = false;
/*-- Index des variable GLSL --------*/
static GLuint GL_programId_ = 0;
static GLint  GL_frame_projectionId_ = 0;
static GLint  GL_frame_timeId_ = 0;

void   CoqGraph_opengl_init(bool loadCoqlibPngs)
{
    // Version
    const char* version_str = (const char*)glGetString(GL_VERSION);
    float       version_float = 3.1;
    sscanf(version_str, "%f", &version_float);
    isOpenGL_3_1_ = version_float > 3.0;
    printdebug("Is at least OpenGL 3.1: %s.", isOpenGL_3_1_ ? "yes ✅" : "no ⚠️");
    if(!isOpenGL_3_1_) {
        printwarning("Cannot draw multiple instance with openGL %s ?", version_str);
    }
    // Shaders
    // Vertex shader
    GLint info_length;
    GLchar* info_str;
    const char* glsl_version = isOpenGL_3_1_ ? "\n\n#version 410\n\n" : "\n\n#version 300 es\n\n";
    const char* shader_path = FileManager_getResourcePathOpt("vertex_shader", "glsl", NULL);
    const char* shader_content = FILE_stringContentOptAt(shader_path);
    if(!shader_content) {
        printerror("Cannot load shader."); return;
    }
    const char* vertexShader_content = String_createCat(glsl_version, shader_content);
    // printf(vertexShader_content);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShader_content, NULL);
    coq_free((char*)vertexShader_content);
    glCompileShader(vertexShaderId);
    glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &info_length);
    if(info_length > 1) {
        info_str = calloc(1, info_length + 1);
        glGetShaderInfoLog(vertexShaderId, info_length, NULL, info_str);
        printerror("Vertex shader: %s", info_str);
        free(info_str);
    } else {
        printok("Vertex shader loaded.");
    }
    // Fragment shader
    shader_path = FileManager_getResourcePathOpt("fragment_shader", "glsl", NULL);
    shader_content = FILE_stringContentOptAt(shader_path);
    const char* fragmentShader_content = String_createCat(glsl_version, shader_content);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &fragmentShader_content, NULL);
    coq_free((char*)fragmentShader_content);
    glCompileShader(fragmentShaderId);
    glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &info_length);
    if(info_length > 1) {
        info_str = calloc(1, info_length + 1);
        glGetShaderInfoLog(fragmentShaderId, info_length, NULL, info_str);
        printerror("Fragment shader: %s", info_str);
        free(info_str);
    } else {
        printok("Fragment shader loaded.");
    }
    // Program
    GL_programId_ = glCreateProgram();
    glAttachShader(GL_programId_, vertexShaderId);
    glAttachShader(GL_programId_, fragmentShaderId);
    glLinkProgram(GL_programId_);
    // Juste un program... on peut le setter tout suite (pas de changement)
    glUseProgram(GL_programId_);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    // Blending ordinaire...
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Index des variable GLSL
    GL_frame_projectionId_ = glGetUniformLocation(GL_programId_, "frame_projection");
    GL_frame_timeId_ =       glGetUniformLocation(GL_programId_, "frame_time");

    // Instance uniforms buffer
    IUsBuffer_opengl_init_(GL_programId_);
    // Meshes de bases
    Mesh_opengl_init_(GL_programId_);
    Mesh_init_();
    // Textures
    Texture_opengl_init_(GL_programId_);
    Texture_init_(loadCoqlibPngs);
}
void   CoqGraph_opengl_deinit(void) {
    IUsBuffer_opengl_deinit_();
    glDeleteProgram(GL_programId_);
}

#pragma mark - Dessin d'une instance (groupe d'instances) OpenGL -
/*-- Mesh et texture presentement utilisees --*/
static Texture    *currentTexture_ = NULL;
static Mesh       *currentMesh_ = NULL;
static GLenum      currentMesh_primitive_ = GL_TRIANGLE_STRIP;
static uint32_t    currentMesh_indexCount_ = 0;
static uint32_t    currentMesh_vertexCount_ = 0;

void  rendering_opengl_initForDrawing(void) {
    currentMesh_ = NULL;
    currentTexture_ = NULL;
}
void  rendering_opengl_draw(Mesh *const mesh, Texture *const tex, InstanceUniforms const*const iu) {
    // 1. Mise a jour de la mesh ?
    if(currentMesh_ != mesh) {
        currentMesh_ = mesh;
        currentMesh_vertexCount_ = currentMesh_->vertex_count;
        currentMesh_indexCount_ = currentMesh_->index_count;
        currentMesh_primitive_ = currentMesh_->primitive_type;
        mesh_glBind(currentMesh_);
    }
    // 2. Mise a jour de la texture ?
    if(currentTexture_ != tex) {
        currentTexture_ = tex;
        texture_glBind(currentTexture_);
    }
    // 3. Cas standard (une instance)
    instanceuniform_glBind(iu);
    if(currentMesh_indexCount_) {
      glDrawElements(GL_TRIANGLES, currentMesh_indexCount_, GL_UNSIGNED_SHORT, 0);
    } else {
      glDrawArrays(currentMesh_primitive_, 0, currentMesh_vertexCount_);
    }
}
void  rendering_opengl_drawMulti(Mesh *const mesh, Texture *const tex, IUsBuffer *const iusBuffer) {
    // 1. Mise a jour de la mesh ?
    if(currentMesh_ != mesh) {
        currentMesh_ = mesh;
        currentMesh_vertexCount_ = currentMesh_->vertex_count;
        currentMesh_indexCount_ = currentMesh_->index_count;
        currentMesh_primitive_ = currentMesh_->primitive_type;
        mesh_glBind(currentMesh_);
    }
    // 2. Mise a jour de la texture ?
    if(currentTexture_ != tex) {
        currentTexture_ = tex;
        texture_glBind(currentTexture_);
    }
    // 4. Cas multi-instance
    uint32_t instanceCount = iusBuffer->actual_count;
    if(instanceCount == 0) {
        printwarning("No instance to draw.");
        return;
    }
    // (au moins 3.1 pour les instances ??)
    if(isOpenGL_3_1_) {
        iusbuffer_glBind(iusBuffer);
        if(currentMesh_indexCount_) {
            glDrawElementsInstanced(GL_TRIANGLES, currentMesh_indexCount_, GL_UNSIGNED_SHORT,
                                    0, instanceCount);
        } else {
            glDrawArraysInstanced(currentMesh_primitive_, 0,
                                  currentMesh_vertexCount_, instanceCount);
        }
        return;
    }
    // Sinon boucle pour dessiner toutes les instances... :(  ??
    // Il doit il y avoir une meilleure solution... ?
    const InstanceUniforms       *iu =   iusBuffer->ius;
    const InstanceUniforms *const end = &iusBuffer->ius[iusBuffer->actual_count];
    for(; iu < end; iu++) {
        instanceuniform_glBind(iu);
        if(currentMesh_indexCount_) {
          glDrawElements(GL_TRIANGLES, currentMesh_indexCount_, GL_UNSIGNED_SHORT, 0);
        } else {
          glDrawArrays(currentMesh_primitive_, 0, currentMesh_vertexCount_);
        }
    }
}

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
