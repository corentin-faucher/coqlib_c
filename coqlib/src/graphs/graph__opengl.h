//  graph__opengl.h
//  Fonctions diverses utiles pour OpenGL.
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef COQ_GRAPH__OPENGL_H
#define COQ_GRAPH__OPENGL_H

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
#ifdef __linux__
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#define COQGL_OPENGL_MAJOR 3
#define COQGL_OPENGL_MINOR 3
#define COQGL_COLATT_MAXCOUNT 16
#include "graph_texture.h"
#include "graph_mesh.h"
#include "graph_iusbuffer.h"
#include "graph_base.h"

// MARK: Fonction pratique pour un main avec GLFW.
GLFWwindow* CoqGLFW_and_Coqlib_initAndGetWindow(int width, int height,
                    char const* fontsPathOpt, char const* defaultFontNameOpt,
                    char const* emojiFontNameOpt);
void        CoqGLFW_setEventsAndStart(GLFWwindow* window);
void        CoqGL_render_checksAfterRendererDraw(void);
void        CoqGLFW_endAndDestroyWindow(GLFWwindow* window);

// MARK: Création d'un programme OpenGL (pipeline)
GLuint CoqGLProgram_createFromShaders(const char* vertexShaderFileName,
                                   const char* fragmentShaderFileName);
                                   
// MARK: Création d'un frame buffer
// Pour rendering multi passes.
typedef struct FramebufferWithColors {
    GLuint   framebufferId;
    GLuint   colorAttachmentIds[COQGL_COLATT_MAXCOUNT];
    uint32_t colorAttachmentCount;
} FramebufferWithColors;
FramebufferWithColors CoqGLFramebuffer_create(
    uint32_t colorAttachmentCount,
    uint32_t width, uint32_t height);
void CoqGLFrameBuffer_delete(FramebufferWithColors fbwc);

// Les fonction suivantes sont un peu superflues...
// il faut juste pas oublier de les mettre dans le renderer.
// 1. À l'init, binder les locations des textures de la deuxième passe.
// 2. Avant de renderer la deuxième passe, binder les textures (color attch.) à dessiner.
void CoqGLframebuffer_initColorAttProgramLocations(GLuint programId, 
                    char const** textureNames, uint32_t colorAttCount);
void CoqGLframebuffer_bindColorAttachsTex(FramebufferWithColors fbwc);
    
// MARK: - Quad de rendering (pour multi passes)
// Simple quad "cadre" pour la deuxième passe.
// (Redessinage à partir d'un framebuffer.) 
// Init par `CoqGL_initGLFWAndGetWindow`.
//    [-1, 1] x [-1, 1] en (x, y)
// et [ 0, 0] x [ 1, 1] en (u, v). 
// (Vertex avec seulement (x,y), (u,v).)
void CoqGL_glBindRenderQuadVertexArray(void);

// MARK: Instance uniforms buffer
// Init avec opengl. A lier avec la première passe 
// (où les Node Drawables sont dessinés).
void IUsBuffer_opengl_init(GLuint programId);
void IUsBuffer_opengl_deinit(void);


// MARK: Mesh
// Fonction à utiliser pour initialiser les décalages des composantes
// de vertex utilisés dans le projet (les Node Drawable).
// glVertexAttribPointer, glEnableVertexAttribArray, ...
void   Mesh_opengl_initSetVertexAttributes(void (*setVertexAttributesOpt)(void));


// MARK: - Dessiner les drawable avec OpenGL
void rendering_opengl_initForDrawing(void);
void rendering_opengl_drawingEnded(void);
// 1. Setter la mesh
// 2. Setter la texture
// 3. Setter les instance uniforms
// 4. Dessiner les triangles...
void rendering_opengl_setCurrentMesh(Mesh* mesh);
/// Si on sait que la mesh est déjà init (gen, fill), 
/// on peut skipper les checks.
void rendering_opengl_setCurrentMeshNoCheck(Mesh const* mesh);
void rendering_opengl_setCurrentTexture(Texture* tex);
/// Si on sait que la texture est déjà init, on peut skipper les checks.
void rendering_opengl_setCurrentTextureNoCheck(Texture* tex);
void rendering_opengl_setIU(InstanceUniforms const* iu);
void rendering_opengl_setIUs(IUsBuffer const* ius);
void rendering_opengl_drawWithCurrents(void);

#endif
