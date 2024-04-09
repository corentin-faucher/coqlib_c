//
//  graph_texture_apple.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef graph_texture_apple_h
#define graph_texture_apple_h

#import <Metal/Metal.h>
#include "graphs/graph_texture.h"
#include "graphs/graph_mesh.h"

/*-- Texture --*/
void           Texture_init(id<MTLDevice> const device, PngInfo const pngInfos[], const uint pngCount);
id<MTLTexture> texture_MTLTexture(Texture* texOpt);

/*-- Mesh --*/
void           Mesh_init(id<MTLDevice> const device);
id<MTLBuffer>  mesh_MTLIndicesBufferOpt(Mesh* mesh);
id<MTLBuffer>  mesh_MTLVerticesBuffer(Mesh* mesh);

/*-- Uniforms Buffer --*/
id<MTLBuffer>  uniformbuffer_MTLBuffer(UniformBuffer* ub);

id<MTLBuffer>  mtlbufferCPtr_asMTLBuffer(const void* mtlBufferCPtr);




#endif /* graph_texture_apple_h */
