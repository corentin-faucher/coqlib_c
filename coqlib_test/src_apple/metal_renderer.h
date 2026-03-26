//
//  MyMTKViewDelegate.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import <MetalKit/MetalKit.h>
#import "coqlib_apple.h"

#define EFFECTS_COUNT 100

@interface Renderer : NSObject <MTKViewDelegate> {
    id<MTLCommandQueue>        queue;
    
    // Première passe
    id<MTLRenderPipelineState> firstPipelineState;
    
    // Frame buffer, i.e. intermédiare entre 1re et 2e passe.
    CoqFramebuffer             fb;
    
    // Seconde passe
    id<MTLRenderPipelineState> secondPipelineState;
    
@public
    BOOL             noSleep;
}

- (instancetype)initWithView:(MTKView*)view;

@end
