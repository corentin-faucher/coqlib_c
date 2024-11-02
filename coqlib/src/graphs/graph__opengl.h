//
//  graph__opengl.h
//  Pour utiliser OpenGL (au lieu de Metal)
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef COQ_GRAPH__OPENGL_H
#define COQ_GRAPH__OPENGL_H

#include "graph_texture.h"
#include "graph_mesh.h"
#include "graph_base.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
#ifdef __linux__
#include <GL/glew.h>
#endif

void   CoqGraph_opengl_init(bool loadCoqlibPngs);
void   CoqGraph_opengl_deinit(void);

/*-- Texture --*/
void   Texture_opengl_init_(GLuint program);
void   texture_glBind(Texture* tex);

/*-- Mesh --*/
void   Mesh_opengl_init_(GLuint program);
void   mesh_glBind(Mesh *mesh);


/*-- Uniforms Buffer --*/
void   iusbuffer_glBind(IUsBuffer* iusbuffer);
void   instanceuniform_glBind(InstanceUniforms const *iu);

/*-- Dessin d'une instance (groupe d'instances) --*/
void  rendering_opengl_initForDrawing(void);
void  rendering_opengl_draw(Mesh *mesh, Texture *tex, InstanceUniforms const* iu);
void  rendering_opengl_drawMulti(Mesh *mesh, Texture *tex, IUsBuffer *iusBuffer);

#endif
