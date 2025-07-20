//  graph__opengl.h
//  Pour utiliser OpenGL (au lieu de Metal)
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef COQ_GRAPH__OPENGL_H
#define COQ_GRAPH__OPENGL_H

#include "graph_texture.h"
#include "graph_mesh.h"
#include "graph_iusbuffer.h"
#include "graph_base.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
#ifdef __linux__
//#include <GL/glew.h>
#include <glad/glad.h>
#endif

void   CoqGraph_opengl_init(MeshInit const* drawableSpriteInitOpt,
                            MeshInit const* renderingQuadInitOpt);
void   CoqGraph_opengl_deinit(void);

// MARK: - Renderer
extern void (*Renderer_opengl_drawView)(void);

/*-- Mesh --*/
// Setter les "location" des "position", "uv", "color" dans un vertex.
// (Customizable, peut être overridé)
extern void (*Mesh_opengl_initVertexAttributeLocations)(GLuint program);
extern void (*mesh_opengl_setVertexAttributes)(Mesh const* mesh);


/// MARK: - Dessiner avec OpenGL: Setter mesh, texture, instance uniforms, -> dessiner les triangles...
void rendering_opengl_initForDrawing(void);

void rendering_opengl_setCurrentMesh(Mesh* mesh);
void rendering_opengl_setCurrentTexture(Texture* tex);
void rendering_opengl_setIU(InstanceUniforms const* iu);
void rendering_opengl_setIUs(IUsBuffer const* ius);

void rendering_opengl_drawWithCurrents(void);

#endif
