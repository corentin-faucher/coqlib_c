//
//  graph__metal.h
//  Functions pratiques pour le Renderer Metal.
//
//  Created by Corentin Faucher on 2023-10-12.
//
#ifndef graph_texture_apple_h
#define graph_texture_apple_h

#import <Metal/Metal.h>
#include "graph_texture.h"
#include "graph_mesh.h"
#include "graph_iusbuffer.h"
#include "../nodes/node_drawable_multi.h"

// MARK: - Init et variables générales de Metal
// Pour les sprites par défaut -> voir `Mesh_initDefaultMeshes_`...
void CoqMtl_init(id<MTLDevice> device, MTLPixelFormat pixelFormatOpt,
                         MTLPixelFormat depthPixelFormatOpt);

extern id<MTLDevice>       CoqMtl_device;
extern MTLPixelFormat      CoqMtl_pixelFormat;
extern MTLPixelFormat      CoqMtl_depthPixelFormat;
extern id<MTLSamplerState> CoqMtl_samplerNearest;
extern id<MTLSamplerState> CoqMtl_samplerNearestClamp;
extern id<MTLSamplerState> CoqMtl_samplerLinear;
//extern id<MTLBuffer>       CoqMtl_renderQuadVerticesBuffer;

// MARK: - Texture
void           Texture_metal_init_(id<MTLDevice> device);
void           Texture_metal_deinit_(void);
void           Texture_metal_setFrameBufferToMTLTexture(uint32_t bufferIndex, id<MTLTexture> mtlTex);

// MARK: - Color attachment pour le blending des pixels à l'écran.
/// Set les opération par défaut sur les couleurs en mode "blending",
/// i.e. NewPixel = (1-alpha)*old + alpha*added;
void colorattachment_initDefaultBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment);
void colorattachment_initLightBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment);
void colorattachment_initFourFloat(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment);

// MARK: Render Pipeline State
// La pipeline, équivalent du `program` dans OpenGL : vertex shader -> fragment shader.
MTLRenderPipelineDescriptor* MTLRenderPipelineDescriptor_create(
            id<MTLDevice> const device,
            const char*const vertexFunctionName,
            const char*const fragmentFunctionName,
            uint32_t const colorAttachmentCount);

// MARK: Render pass descriptor
/// Equivalent du framebuffer dans OpenGL, 
/// là où on dessine si on ne dessine pas
/// directement à l'écran.
typedef struct CoqFramebuffer {
    MTLRenderPassDescriptor* renderPassDescr;
    uint32_t width, height;
    uint32_t colorAttachmentCount;
    Vector4  clearColor;
    MTLRenderPipelineDescriptor* pipelineDescr;
} CoqFramebuffer;
CoqFramebuffer CoqFramebuffer_create(
        id<MTLDevice> device,
        uint32_t colorAttachmentCount,
        uint32_t width, uint32_t height,
        MTLRenderPipelineDescriptor* descr);
void coqframebuffer_resize(CoqFramebuffer *fb, id<MTLDevice> device, uint32_t width, uint32_t height);
void coqframebuffer_updateClearColor(CoqFramebuffer *fb, Vector4 clearColor);
void coqframebuffer_deinit(CoqFramebuffer* fb);

// MARK: - CommandEncoder
// MARK: Dessin de drawables.
// Init render pass.
void commandencoder_initForDrawing(id<MTLRenderCommandEncoder> encoder);
// Mise à jour de la mesh et texture.
void commandencoder_setCurrentMesh(id<MTLRenderCommandEncoder> encoder, Mesh * newMesh);
void commandencoder_setCurrentMeshNoCheck(id<MTLRenderCommandEncoder> encoder, Mesh const* newMesh);
void commandencoder_setCurrentTexture(id<MTLRenderCommandEncoder> encoder, Texture * newTex);
void commandencoder_setCurrentTextureNoCheck(id<MTLRenderCommandEncoder> encoder, Texture* newTex);
// Mise à jour des Instance uniforms.
void commandencoder_setIU(id<MTLRenderCommandEncoder> encoder, InstanceUniforms const* iu);
void commandencoder_setIUs(id<MTLRenderCommandEncoder> encoder, IUsBuffer const* ius);
// Dessiner avec mesh, texture, iu settés précédemment.
void commandencoder_drawWithCurrents(id<MTLRenderCommandEncoder> encoder);

// MARK: Quad de rendering (pour multi passes)
// Simple quad "cadre" pour la deuxième passe.
// (Redessinage à partir d'un framebuffer.) 
// Init par `CoqMtl_init`.
//    [-1, 1] x [-1, 1] en (x, y)
// et [ 0, 0] x [ 1, 1] en (u, v). 
// (Vertex avec seulement (x,y), (u,v).)
void commandencoder_drawRenderQuad(id<MTLRenderCommandEncoder> encoder);


// Conveniences pour noeud Drawables
// Superflu...
// Equivalent de :
// commandencoder_setCurrentTexture(encoder, d->_tex);
// commandencoder_setCurrentMesh(encoder, d->_mesh);
// commandencoder_setIU(encoder, &d->n.renderIU);
// commandencoder_drawWithCurrents(encoder);
//void commandencoder_drawDrawable(id<MTLRenderCommandEncoder> encoder, Drawable* d);

// Equivalent de :
// commandencoder_setCurrentTexture(encoder, dm->d._tex);
// commandencoder_setCurrentMesh(encoder, dm->d._mesh);
// commandencoder_setIUs(encoder, &dm->iusBuffer);
// commandencoder_drawWithCurrents(encoder);
//void commandencoder_drawDrawableMulti(id<MTLRenderCommandEncoder> encoder, Drawable* d);

#endif /* graph_texture_apple_h */
