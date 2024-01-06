//
//  MyMTKViewDelegate.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import <MetalKit/MetalKit.h>
#include "_graph_texture.h"
#include "_graph_mesh.h"
#include "_math_flpos.h"

@interface Renderer : NSObject <MTKViewDelegate> {
    id<MTLCommandQueue>        queue;
    id<MTLRenderPipelineState> pipelineState;
    id<MTLSamplerState>        samplerState;
    id<MTLDepthStencilState>   depthStencilState;
    
    Texture*         current_tex;
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
