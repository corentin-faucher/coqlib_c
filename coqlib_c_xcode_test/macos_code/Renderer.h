//
//  MyMTKViewDelegate.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import <MetalKit/MetalKit.h>
#include "graph.h"

@interface Renderer : NSObject <MTKViewDelegate> {
    id<MTLCommandQueue>        queue;
    id<MTLRenderPipelineState> pipelineState;
    id<MTLSamplerState>        samplerState;
    id<MTLDepthStencilState>   depthStencilState;
    
    Texture*         current_tex;
    Mesh*            current_mesh;
    MTLPrimitiveType current_primitive_type;
    int              current_vertex_count;
}

- (instancetype)initWithView:(MTKView*)view;

@end
