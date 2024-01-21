//
//  MyMTKViewDelegate.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import <MetalKit/MetalKit.h>
#include "coq_graph.h"
#include "coq_nodes.h"

@interface Renderer : NSObject <MTKViewDelegate> {
    id<MTLCommandQueue>        queue;
    id<MTLRenderPipelineState> pipelineState;
    id<MTLSamplerState>        samplerStateLinear;
    id<MTLSamplerState>        samplerStateNearest;
    id<MTLDepthStencilState>   depthStencilState;
    
    Texture*         current_tex;
    bool             current_tex_nearest;
    Mesh*            current_mesh;
    MTLPrimitiveType current_primitive_type;
    int              current_vertex_count;
    id<MTLBuffer>    current_indicesBuffer;
    uint32_t         current_indexCount;
    MTLCullMode      current_cullMode;
    FluidPos         smDeltaT;
    Chrono           chronoDeltaT;
@public
    BOOL             noSleep;
}

- (instancetype)initWithView:(MTKView*)view;

@end
