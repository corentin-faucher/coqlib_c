//
//  MetalView.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//
#import <MetalKit/MetalKit.h>
#import "Renderer.h"
#include "node_root.h"

@interface MetalView : MTKView {
    Renderer* renderer;
@public
    NodeRoot* root;
}

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device;

@end
