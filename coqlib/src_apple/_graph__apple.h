//
//  graph_texture_apple.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef graph_texture_apple_h
#define graph_texture_apple_h

#import <Metal/Metal.h>
#include "_graph/_graph_texture.h"
#include "_graph/_graph_mesh.h"

/*-- Texture --*/
void           _Texture_init(id<MTLDevice> const device);
id<MTLTexture> texture_MTLTexture(Texture* texOpt);

/*-- Mesh --*/
void           Mesh_init(id<MTLDevice> const device);
id<MTLBuffer>  mesh_MTLIndicesBuffer(Mesh* mesh);

#endif /* graph_texture_apple_h */
