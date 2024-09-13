//
//  graph__apple.h
//  Helpers functions pour le Renderer Metal.
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
void            CoqGraph_MTLinit(id<MTLDevice> device, MTLPixelFormat pixelFormatOpt, 
                                                  MTLPixelFormat depthPixelFormatOpt);
id<MTLDevice>   CoqGraph_getMTLDevice(void);
MTLPixelFormat  CoqGraph_getPixelFormat(void);
MTLPixelFormat  CoqGraph_getDepthPixelFormat(void);
id<MTLSamplerState> CoqGraph_getSampler(bool nearest);

/*-- Texture --*/
void           Texture_MTLinit_(void);

id<MTLTexture> texture_MTLTexture(Texture* texOpt);
void           Texture_setFrameBufferWithMTLTexture(id<MTLTexture> mtlTex, uint32_t index);

/*-- Mesh --*/
id<MTLBuffer>  mesh_MTLIndicesBufferOpt(Mesh* mesh);
id<MTLBuffer>  mesh_MTLVerticesBuffer(Mesh* mesh);

/*-- Uniforms Buffer --*/
id<MTLBuffer>  piusbuffer_asMTLBuffer(const IUsBuffer* piusbuffer);

/*-- Color attachment --*/
/// Set les opération par défaut sur les couleurs en mode "blending",
/// i.e. NewPixel = (1-alpha)*old + alpha*added;
void colorattachment_initDefaultBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment);
void colorattachment_initLightBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment);

/*-- Renderer Command encoder --*/ 
void commandencoder_initForDrawing(id<MTLRenderCommandEncoder> encoder);
void commandencoder_drawDrawable(id<MTLRenderCommandEncoder> encoder, const Drawable* const d);


#endif /* graph_texture_apple_h */
