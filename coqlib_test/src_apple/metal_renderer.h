//
//  MyMTKViewDelegate.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import <MetalKit/MetalKit.h>
#include "coq_graphs.h"
#include "coq_nodes.h"

#define EFFECTS_COUNT 100

typedef struct FinalFragmentUniforms {
    float       extra0;
    float       extra1;
    float       extra2;
    uint32_t    effect_count;
    AfterEffect effects[EFFECTS_COUNT];
} FinalFragmentUniforms;

@interface Renderer : NSObject <MTKViewDelegate> {
    id<MTLCommandQueue>        queue;
    
    id<MTLDepthStencilState>   depthStencilState;
    MTLClearColor              clearColor;
    
    // Première passe
    id<MTLRenderPipelineState> firstPipelineState;
    MTLRenderPassDescriptor*   firstRenderPassDescr;
    id<MTLRenderPipelineState> secondPipelineState;
    
    FinalFragmentUniforms fu;
    
    FluidPos         smDeltaT;     // Calcul du temps entre les frame.
    Chrono           chronoDeltaT; 
    Chrono           chrono;      // Temps écoulé (pour shader)
@public
    BOOL             noSleep;
}

- (instancetype)initWithView:(MTKView*)view;

@end
