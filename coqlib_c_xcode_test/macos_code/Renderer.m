//
//  MyMTKViewDelegate.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import "Renderer.h"
#import "MetalView.h"
#include <stdio.h>
#include "utils.h"
#include "graph_texture_apple.h"
#include "chronometers.h"
#include "node_root.h"
#include "node_squirrel.h"
#include "node_surface.h"
#include "timer.h"

@implementation Renderer

- (instancetype)initWithView:(MTKView*)view {
    self = [super init];
    id<MTLDevice> device = view.device;
    if(device == nil) {
        printerror("no device.");
        return self;
    }
    /*-- Command queue --*/
    queue = [device newCommandQueue];
    
    /*-- Init du pipeline --*/
    id<MTLLibrary> library = [device newDefaultLibrary];
    if(library == nil) { printerror("no library."); return self; }
    MTLRenderPipelineDescriptor *rpd = [MTLRenderPipelineDescriptor new];
    rpd.vertexFunction = [library newFunctionWithName:@"vertex_function"];
    rpd.fragmentFunction = [library newFunctionWithName:@"fragment_function"];
    if(rpd.vertexFunction == nil || rpd.fragmentFunction == nil) {
        printerror("no vertex or fragment function."); return self;
    }
    rpd.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
    MTLRenderPipelineColorAttachmentDescriptor *colorAtt = rpd.colorAttachments[0];
    colorAtt.pixelFormat = view.colorPixelFormat;
    [colorAtt setBlendingEnabled:YES];
    [colorAtt setRgbBlendOperation:MTLBlendOperationAdd];
    [colorAtt setSourceRGBBlendFactor:MTLBlendFactorSourceAlpha];
    [colorAtt setDestinationRGBBlendFactor:MTLBlendFactorOneMinusSourceAlpha];
    pipelineState = [device newRenderPipelineStateWithDescriptor:rpd error:nil];
    
    /*-- Sampler state pour les textures --*/
    MTLSamplerDescriptor *sd = [MTLSamplerDescriptor new];
    sd.magFilter = MTLSamplerMinMagFilterNearest;
    sd.minFilter = MTLSamplerMinMagFilterNearest;
    samplerState = [device newSamplerStateWithDescriptor:sd];
    
    /*-- Depth (si besoin) --*/
    depthStencilState = nil;
//    MTLDepthStencilDescriptor *dsd = [MTLDepthStencilDescriptor new];
//    dsd.depthCompareFunction = MTLCompareFunctionLess;
//    [dsd setDepthWriteEnabled:YES];
//    depthStencilState = [device newDepthStencilStateWithDescriptor:dsd];
    
    return self;
}

- (void)drawNodeSurf:(NodeSurf*)surf withEncoder:(id<MTLRenderCommandEncoder>)encoder {
    // 1. Mise a jour de la mesh ?
    if(current_mesh != surf->_mesh) {
        current_mesh = surf->_mesh;
        current_primitive_type = (MTLPrimitiveType)surf->_mesh->primitive_type;
        current_vertex_count = surf->_mesh->vertex_count;
        
        [encoder setCullMode:(MTLCullMode)current_mesh->cull_mode];
        [encoder setVertexBytes:current_mesh->vertices
                         length:current_mesh->vertices_size atIndex:0];
    }
    // 2. Mise a jour de la texture ?
    if(current_tex != surf->texOpt) {
        current_tex = surf->texOpt;
        [encoder setFragmentTexture:texture_MTLTexture(current_tex) atIndex:0];
        [encoder setVertexBytes:texture_ptu(current_tex)
                         length:sizeof(PerTextureUniforms) atIndex:3];
    }
    // 3. Per instance uniforms
    [encoder setVertexBytes:&surf->nd.piu length:sizeof(PerInstanceUniforms) atIndex:1];
    // 4. Dessiner
    [encoder drawPrimitives:current_primitive_type
            vertexStart:0 vertexCount:current_vertex_count];
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    if(![view isKindOfClass:[MetalView class]]) {
        printerror("MTKView is not a custom MetalView.");
        return;
    }
    MetalView* metalView = (MetalView*)view;
    NodeRoot*  root = metalView->root;
    if(root == NULL) {
        printerror("Root not init.");
        return;
    }
    // Command buffer, command encoder... (a chaque frame)
    if(queue == nil || pipelineState == nil) return;
    id<MTLCommandBuffer> cb = [queue commandBuffer];
    if(cb == nil) { printerror("no command buffer."); return; }
    MTLRenderPassDescriptor *descr = view.currentRenderPassDescriptor;
    if(descr == NULL) { printerror("no pass descriptor."); return; }
    id<MTLRenderCommandEncoder> enc = [cb renderCommandEncoderWithDescriptor:descr];
    if(enc == nil) { printerror("no command encoder."); return; }
    [enc setFragmentSamplerState:samplerState atIndex:0];
    [enc setRenderPipelineState:pipelineState];
    if(depthStencilState != nil)
        [enc setDepthStencilState:depthStencilState];
    current_mesh = NULL;
    current_tex = NULL;
    
    // Set per frame uniforms (projection matrix, time)
    ChronoRender_update((int)view.preferredFramesPerSecond);
    Timer_check();
    
    pfu_default.time = ChronoRender_elapsedAngleSec();
    matrix4_initProjectionWithNodeRoot(&pfu_default.projection, root);
    [enc setVertexBytes:&pfu_default length:sizeof(PerFrameUniforms) atIndex:2];
    
    // Drawing
    Squirrel sq;
    sq_init(&sq, (Node*)root, sq_scale_ones);
    do {
        NodeSurf* surf = node_defaultUpdateForDrawingAndGetAsSurfOpt(sq.pos);
        if(surf) {
            [self drawNodeSurf:surf withEncoder:enc];
        }
    } while(sq_goToNextToDisplay(&sq));
    
    // Fin. Soumettre au gpu.
    [enc endEncoding];
    [cb presentDrawable:[view currentDrawable]];
    [cb commit];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    if(![view isKindOfClass:[MetalView class]]) {
        printerror("MTKView is not a custom MetalView.");
        return;
    }
    MetalView* metalView = (MetalView*)view;
    NSWindow* window = view.window;
    NodeRoot* root = metalView->root;
    if(root == NULL) {
        printwarning("Still no root.");
        return;
    }
//    [metalView setIsPaused: NO];
    CGFloat headerHeight = (window.styleMask & NSWindowStyleMaskFullScreen) ?
        22 : window.frame.size.height - window.contentLayoutRect.size.height;
    root->margins = (Margins) { headerHeight, 0, 0, 0 };
    printdebug("Set root frame.");
    noderoot_setFrame(root,  view.frame.size.width, view.frame.size.height, false);
    printdebug("reshape end");
}



@end
