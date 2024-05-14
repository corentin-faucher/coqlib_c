//
//  graph_texture_apple.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef graph_texture_apple_h
#define graph_texture_apple_h

#import <Metal/Metal.h>
#include "../src/graphs/graph_texture.h"
#include "../src/graphs/graph_mesh.h"

void           CoqGraph_MTLinit(id<MTLDevice> device);

/*-- Texture --*/
id<MTLTexture> texture_MTLTexture(Texture* texOpt);

/*-- Mesh --*/
id<MTLBuffer>  mesh_MTLIndicesBufferOpt(Mesh* mesh);
id<MTLBuffer>  mesh_MTLVerticesBuffer(Mesh* mesh);

/*-- Uniforms Buffer --*/
id<MTLBuffer>  piusbuffer_asMTLBuffer(const PIUsBuffer* piusbuffer);

/*-- Ref au gpu pour créer buffer, texture... --*/
extern id<MTLDevice>  MTL_device_;

extern const MTLPixelFormat MTL_pixelFormat_;


#endif /* graph_texture_apple_h */
