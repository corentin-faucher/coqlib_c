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
#include "graph_base.h"
#include "graph_mesh.h"
#include "graph_texture.h"
#include <OpenGL/OpenGL.h>

/// MARK: - CoqGraph_Init, Init du stuff graphique : buffer, mesh, texture...
static bool isOpenGL_3_1_ = false;
/*-- Index des variable GLSL --------*/
static GLuint GL_programId_ = 0;
static GLint  GL_frame_projectionId_ = 0;
static GLint  GL_frame_timeId_ = 0;
// IUB : Instance uniforms buffer, tempon pour passer un packet de InstanceUniforms.
#define IUB_MaxInstances 500
static GLuint IUB_uniformBlockId_ = 0;
static GLuint IUB_bindingPoint_ = 0;
static GLuint IUB_bufferId_ = 0;

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

void   CoqGraph_opengl_init(MeshInit const* drawableSpriteInitOpt,
                            MeshInit const* renderingQuadInitOpt)
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

    // IDs des variable GLSL
    GL_frame_projectionId_ = glGetUniformLocation(GL_programId_, "frame_projection");
    GL_frame_timeId_ =       glGetUniformLocation(GL_programId_, "frame_time");

    // Instance uniforms buffer
    IUB_uniformBlockId_ = glGetUniformBlockIndex(GL_programId_, "InstanceBufferType");
    glUniformBlockBinding(GL_programId_, IUB_uniformBlockId_, IUB_bindingPoint_);
    glGenBuffers(1, &IUB_bufferId_);
    glBindBuffer(GL_UNIFORM_BUFFER, IUB_bufferId_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(InstanceUniforms)*IUB_MaxInstances, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, IUB_bindingPoint_, IUB_bufferId_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // Meshes de bases
    Mesh_opengl_initVertexAttributeLocations(GL_programId_);
    Mesh_initDefaultMeshes_(drawableSpriteInitOpt, renderingQuadInitOpt);
    // Textures
    Texture_init_();
}
void   CoqGraph_opengl_deinit(void) {
    glDeleteBuffers(1, &IUB_bufferId_);
    glDeleteProgram(GL_programId_);
}

/// MARK: - Dessin d'une instance (groupe d'instances) OpenGL -
/*-- Mesh et texture presentement utilisees --*/
static Texture      *currentTexture_ = NULL;
static TextureToDraw currentTextureToDraw_;
static Mesh         *currentMesh_ = NULL;
static MeshToDraw    currentMeshToDraw_;
static size_t        currentInstanceCount_ = 1;
static InstanceUniforms const* currentIUs_ = NULL;

void  rendering_opengl_initForDrawing(void) {
    currentMesh_ = NULL;
    currentTexture_ = NULL;
    currentInstanceCount_ = 1;
}
void rendering_opengl_setCurrentMesh(Mesh*const mesh) {
    if(mesh == currentMesh_) return;
    currentMesh_ = mesh;
    mesh_render_tryToUpdateVerticesAndIndiceCount(currentMesh_);
    currentMeshToDraw_ = mesh_render_getMeshToDraw(currentMesh_);
    glBindVertexArray(currentMeshToDraw_.glVertexArrayId);
}
void rendering_opengl_setCurrentTexture(Texture*const tex) {
    if(tex == currentTexture_) return;
    currentTexture_ = tex;
    currentTextureToDraw_ = texture_engine_touchAndGetToDraw(currentTexture_);
    glBindTexture(GL_TEXTURE_2D, currentTextureToDraw_.glTextureId);
}
void rendering_opengl_setIU(InstanceUniforms const* iu) {
    glBindBuffer(GL_UNIFORM_BUFFER, IUB_bufferId_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(InstanceUniforms), iu);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    currentInstanceCount_ = 1;
}
void rendering_opengl_setIUs(IUsToDraw const iusToDraw) {
    currentInstanceCount_ = iusToDraw.count;
    currentIUs_ = iusToDraw.iusOpt;
    if(currentInstanceCount_ > IUB_MaxInstances) {
        printwarning("To many instance %zu, max is %d.", currentInstanceCount_, IUB_MaxInstances);
        currentInstanceCount_ = IUB_MaxInstances;
    }
    if(!iusToDraw.count || !iusToDraw.iusOpt) {
        printwarning("IUs missing.");
        currentInstanceCount_ = 0;
        currentIUs_ = NULL;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, IUB_bufferId_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
        currentInstanceCount_ * sizeof(InstanceUniforms), currentIUs_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void rendering_opengl_drawWithCurrents(void) {
    if(0 == currentInstanceCount_) return;
    if(1 == currentInstanceCount_) {
        if(currentMeshToDraw_.indexCount) {
            // (Indices déjà settés...)
            glDrawElements(GL_TRIANGLES, currentMeshToDraw_.indexCount,
                GL_UNSIGNED_SHORT, NULL);
        } else {
            glDrawArrays(currentMeshToDraw_.primitive_type, 0,
                currentMeshToDraw_.vertexCount);
        }
        return;
    }
    // Cas multi-instances...
    // (au moins OpenGL 3.1 pour les instances ??)
    if(isOpenGL_3_1_) {
        if(currentMeshToDraw_.indexCount) {
            glDrawElementsInstanced(GL_TRIANGLES, currentMeshToDraw_.indexCount,
                    GL_UNSIGNED_SHORT, NULL, (GLsizei)currentInstanceCount_);
        } else {
            glDrawArraysInstanced(currentMeshToDraw_.primitive_type, 0,
                    currentMeshToDraw_.vertexCount, (GLsizei)currentInstanceCount_);
        }
        return;
    }
    printerror("TODO : implementer multi-instances pour OpenGL < 3.1 ...");
    // Sinon boucle pour dessiner toutes les instances... :(  ??
    // Il devrait il y avoir une meilleure solution... ?
    // const InstanceUniforms *const end = &currentIUs_[currentInstanceCount_];
    // for(InstanceUniforms const* iu = currentIUs_; iu < end; iu++) {
    //     set instances uniforms... model, color, etc.
    //     if(currentMeshToDraw_.indexCount) {
    //         // (Indices déjà settés...)
    //         glDrawElements(GL_TRIANGLES, currentMeshToDraw_.indexCount,
    //             GL_UNSIGNED_SHORT, NULL);
    //     } else {
    //         glDrawArrays(currentMeshToDraw_.primitive_type, 0,
    //             currentMeshToDraw_.vertexCount);
    //     }
    // }
}


#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
