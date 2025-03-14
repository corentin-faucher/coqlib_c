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

// MARK: - CoqGraph Metal Général
void CoqGraph_metal_init(id<MTLDevice> device, MTLPixelFormat pixelFormatOpt,
                         MTLPixelFormat depthPixelFormatOpt,
                         MeshInit const* drawableSpriteInitOpt,
                         MeshInit const* renderingQuadInitOpt);

extern id<MTLDevice>       CoqGraph_metal_device;
extern MTLPixelFormat      CoqGraph_metal_pixelFormat;
extern MTLPixelFormat      CoqGraph_metal_depthPixelFormat;
extern id<MTLSamplerState> CoqGraph_metal_samplerNearest;
extern id<MTLSamplerState> CoqGraph_metal_samplerNearestClamp;
extern id<MTLSamplerState> CoqGraph_metal_samplerLinear;

// MARK: - Texture
void           Texture_metal_init_(void);
void           Texture_metal_deinit_(void);
void           Texture_metal_setFrameBufferToMTLTexture(uint32_t bufferIndex, id<MTLTexture> mtlTex);
id<MTLTexture> Texture_metal_defaultWhiteMTLTexture(void);

// MARK: - Color attachment / blending
/// Set les opération par défaut sur les couleurs en mode "blending",
/// i.e. NewPixel = (1-alpha)*old + alpha*added;
void colorattachment_initDefaultBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment);
void colorattachment_initLightBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment);
void colorattachment_initFourFloat(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment);

// MARK: - Dessin avec Command encoder de Metal
// Init render pass.
void commandencoder_initForDrawing(id<MTLRenderCommandEncoder> encoder);
// Mise à jour de la mesh et texture.
void commandencoder_setCurrentMesh(id<MTLRenderCommandEncoder> encoder, Mesh * newMesh);
void commandencoder_setCurrentTexture(id<MTLRenderCommandEncoder> encoder, Texture * newTex);
// Mise à jour des Instance uniforms.
void commandencoder_setIU(id<MTLRenderCommandEncoder> encoder, InstanceUniforms const* iu);
void commandencoder_setIUs(id<MTLRenderCommandEncoder> encoder, IUsToDraw iusToDraw);
// Dessiner avec mesh, texture, iu settés précédemment.
void commandencoder_drawWithCurrents(id<MTLRenderCommandEncoder> encoder);


#endif /* graph_texture_apple_h */
