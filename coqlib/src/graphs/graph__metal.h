//
//  graph__metal.h
//  Functions pratiques pour le Renderer Metal.
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef graph_texture_apple_h
#define graph_texture_apple_h

#import <Metal/Metal.h>
#include "graphs/graph_texture.h"
#include "graphs/graph_mesh.h"
#include "nodes/node_drawable_multi.h"

/*-- Général --*/
// Init de Metal, call Texture_MTLinit.
void            CoqGraph_metal_init(id<MTLDevice> device, 
                            MTLPixelFormat pixelFormatOpt, MTLPixelFormat depthPixelFormatOpt,
                            bool loadCoqlibPngs);
id<MTLDevice>   CoqGraph_metal_getDevice(void);
MTLPixelFormat  CoqGraph_metal_getPixelFormat(void);
MTLPixelFormat  CoqGraph_metal_getDepthPixelFormat(void);
id<MTLSamplerState> CoqGraph_metal_getSampler(bool nearest);

/*-- Texture --*/
void           Texture_metal_init_(void);

id<MTLTexture> texture_metal_asMTLTexture(Texture* texOpt);
void           Texture_metal_setFrameBufferToMTLTexture(uint32_t bufferIndex, id<MTLTexture> mtlTex);

/*-- Mesh --*/
id<MTLBuffer>  mesh_metal_indicesMTLBufferOpt(Mesh const* mesh);
id<MTLBuffer>  mesh_metal_verticesMTLBuffer(Mesh const* mesh);

/*-- Uniforms Buffer --*/
id<MTLBuffer>  iusbuffer_metal_asMTLBuffer(const IUsBuffer* piusbuffer);

#pragma mark - Color attachment et rendering avec command encoder -
/// Set les opération par défaut sur les couleurs en mode "blending",
/// i.e. NewPixel = (1-alpha)*old + alpha*added;
void colorattachment_initDefaultBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment);
void colorattachment_initLightBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment);

#pragma mark - Dessin avec Metal Command encoder
void commandencoder_initForDrawing(id<MTLRenderCommandEncoder> encoder);
void commandencoder_draw(id<MTLRenderCommandEncoder> encoder, 
                         Mesh const* mesh, Texture *tex, InstanceUniforms const *iu);
void commandencoder_drawMulti(id<MTLRenderCommandEncoder> encoder, 
                              Mesh const* mesh, Texture *tex, IUsBuffer const *iusBuffer);
__attribute__((deprecated("utiliser `commandencoder_draw` ou `commandencoder_drawMulti`.")))
void commandencoder_drawDrawable(id<MTLRenderCommandEncoder> encoder, const Drawable* const d);


#endif /* graph_texture_apple_h */
