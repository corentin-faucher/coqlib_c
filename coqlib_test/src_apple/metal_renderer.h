//
//  MyMTKViewDelegate.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import <MetalKit/MetalKit.h>

#define EFFECTS_COUNT 100

@interface Renderer : NSObject <MTKViewDelegate> {
    id<MTLCommandQueue>        queue;
    
    id<MTLDepthStencilState>   depthStencilState;
    MTLClearColor              clearColor;
    
    // Première passe
    id<MTLRenderPipelineState> firstPipelineState;
    MTLRenderPassDescriptor*   firstRenderPassDescr;
    id<MTLRenderPipelineState> secondPipelineState;
    
@public
    BOOL             noSleep;
}

- (instancetype)initWithView:(MTKView*)view;

@end
