//
//  graph_texture_apple.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef _coq_graph__opengl_h
#define _coq_graph__opengl_h

#include "_graph_texture.h"
#include "_graph_mesh.h"
#include "_math_camera.h"

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

/*-- Texture --*/
void   Texture_init(GLuint program);
void   texture_glBind(Texture* tex);

/*-- Mesh --*/
void   Mesh_init(GLuint program);
void   mesh_glBind(Mesh* mesh);

#endif /* graph_texture_apple_h */
