//
//  graph_texture_apple.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef COQ_GRAPH__OPENGL_H
#define COQ_GRAPH__OPENGL_H
// #ifdef WITH_OPENGL

#include "graph_texture.h"
#include "graph_mesh.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
#ifdef __linux__
#include <GL/glew.h>
#endif

/*-- Texture --*/
void   Texture_GLinit(GLuint program, const char* font_dir, const char* default_font_name);
void   texture_glBind(Texture* tex);

/*-- Mesh --*/
void   Mesh_init(GLuint program);
void   mesh_glBind(Mesh* mesh);

/*-- Uniforms Buffer --*/
void   PIUsBuffer_GLinit(GLuint program);
void   piusbuffer_glBind(const PIUsBuffer* piusbuffer);
void   perinstanceuniform_glBind(const PerInstanceUniforms* piu);

#else
// #warning N inclure que si on utilise OpenGL (avec -DWITH_OPENGL)
#endif
// #endif

/* graph_texture_apple_h */
