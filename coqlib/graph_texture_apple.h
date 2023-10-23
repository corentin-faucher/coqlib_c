//
//  graph_texture_apple.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef graph_texture_apple_h
#define graph_texture_apple_h

#import <Metal/Metal.h>
#include "graph.h"

void Texture_init(id<MTLDevice> const device);

id<MTLTexture> texture_MTLTexture(Texture* texOpt);

#endif /* graph_texture_apple_h */
