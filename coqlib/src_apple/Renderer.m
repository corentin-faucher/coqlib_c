//
//  MyMTKViewDelegate.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import "Renderer.h"

#import "CoqMetalView.h"
#include "_node_squirrel.h"


@implementation Renderer

- (instancetype)initWithView:(MTKView*)view {
    self = [super init];
    id<MTLDevice> device = view.device;
    if(device == nil) {
        printerror("no device.");
        return self;
    }
    view.colorPixelFormat = MTLPixelFormatRGBA8Unorm_sRGB;
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
    samplerStateNearest = [device newSamplerStateWithDescriptor:sd];
    sd.magFilter = MTLSamplerMinMagFilterLinear;
    sd.minFilter = MTLSamplerMinMagFilterLinear;
    samplerStateLinear = [device newSamplerStateWithDescriptor:sd];
    current_tex_nearest = false;
    
    /*-- Depth (si besoin) --*/
    depthStencilState = nil;
//    MTLDepthStencilDescriptor *dsd = [MTLDepthStencilDescriptor new];
//    dsd.depthCompareFunction = MTLCompareFunctionLess;
//    [dsd setDepthWriteEnabled:YES];
//    depthStencilState = [device newDepthStencilStateWithDescriptor:dsd];
    
    return self;
}


- (void)drawDrawable:(Drawable*)d withEncoder:(id<MTLRenderCommandEncoder>)encoder {
    // 1. Mise a jour de la mesh ?
    if(current_mesh != d->mesh) {
        current_mesh = d->mesh;
        current_primitive_type = (MTLPrimitiveType)mesh_primitiveType(d->mesh);
        current_vertex_count = mesh_vertexCount(d->mesh);
        current_indicesBuffer = mesh_MTLIndicesBuffer(d->mesh);
        current_indexCount = mesh_indexCount(d->mesh);
        if(current_cullMode != (MTLCullMode)mesh_cullMode(d->mesh)) {
            [encoder setCullMode:(MTLCullMode)mesh_cullMode(current_mesh)];
            current_cullMode = (MTLCullMode)mesh_cullMode(current_mesh);
        }
        [encoder setVertexBytes:mesh_vertices(current_mesh)
                         length:mesh_verticesSize(current_mesh) atIndex:0];
    }
    // 2. Mise a jour de la texture ?
    if(current_tex != d->tex) {
        current_tex = d->tex;
        bool newNearest = texture_isNearest(current_tex);
        if(current_tex_nearest != newNearest) {
            current_tex_nearest = newNearest;
            [encoder setFragmentSamplerState:(current_tex_nearest ? samplerStateNearest : samplerStateLinear) atIndex:0];
        }
        [encoder setFragmentTexture:texture_MTLTexture(current_tex) atIndex:0];
        [encoder setVertexBytes:texture_ptu(current_tex)
                         length:sizeof(PerTextureUniforms) atIndex:3];
    }
    // 3. Per instance uniforms
    [encoder setVertexBytes:&d->n.piu length:sizeof(PerInstanceUniforms) atIndex:1];
    // 4. Dessiner
    if(current_indicesBuffer != nil) {
        [encoder drawIndexedPrimitives:current_primitive_type
                            indexCount:current_indexCount
                             indexType:MTLIndexTypeUInt16
                           indexBuffer:current_indicesBuffer
                     indexBufferOffset:0];
    } else {
        [encoder drawPrimitives:current_primitive_type
                    vertexStart:0 vertexCount:current_vertex_count];
    }
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    if(![view isKindOfClass:[CoqMetalView class]]) {
        printerror("MTKView is not a custom MetalView.");
        return;
    }
    CoqMetalView* metalView = (CoqMetalView*)view;
    Root*  root = metalView->root;
    if(root == NULL) {
        printerror("Root not init.");
        return;
    }
    if(metalView.isSuspended) { return; }
    // Command buffer, command encoder... (a chaque frame)
    if(queue == nil || pipelineState == nil) return;
    id<MTLCommandBuffer> cb = [queue commandBuffer];
    if(cb == nil) { printerror("no command buffer."); return; }
    MTLRenderPassDescriptor *descr = view.currentRenderPassDescriptor;
    if(descr == NULL) { printerror("no pass descriptor."); return; }
    id<MTLRenderCommandEncoder> enc = [cb renderCommandEncoderWithDescriptor:descr];
    descr = nil;
    if(enc == nil) { printerror("no command encoder."); return; }
    [enc setFragmentSamplerState:(current_tex_nearest ? samplerStateNearest : samplerStateLinear) atIndex:0];
    [enc setRenderPipelineState:pipelineState];
    if(depthStencilState != nil)
        [enc setDepthStencilState:depthStencilState];
    
    // 1. Check le chrono/sleep.
    float deltaTMS = fminf(60.f, fmaxf(3.f, (float)chrono_elapsedMS(&chronoDeltaT)));
    chrono_start(&chronoDeltaT);
    fl_set(&smDeltaT, deltaTMS);
    deltaTMS = fl_pos(&smDeltaT);
    int64_t deltaTMSint = rand_float_toInt(deltaTMS);
    ChronoRender_update(deltaTMSint);
    
    pfu_default.time = ChronoRender_elapsedAngleSec();
    matrix4_initProjectionWithRoot(&pfu_default.projection, root);
    [enc setVertexBytes:&pfu_default length:sizeof(PerFrameUniforms) atIndex:2];
    Vector4 cc = fl_array_toVec4(root->back_RGBA);
    [view setClearColor:MTLClearColorMake(cc.r, cc.g, cc.b, cc.a)];
    
    // Drawing
    Drawable* (*updateModel)(Node*) = root->updateModelAndGetDrawable;
    Squirrel sq;
    sq_init(&sq, (Node*)root, sq_scale_ones);
    do {
        Drawable* d = updateModel(sq.pos);
        if(d) [self drawDrawable:d withEncoder:enc];
    } while(sq_goToNextToDisplay(&sq));
    
    // Fin. Soumettre au gpu.
    [enc endEncoding];
    [cb presentDrawable:[view currentDrawable]];
    [cb commit];
    enc = nil;
    cb = nil;
    current_tex = NULL;
    current_mesh = NULL;
    current_indicesBuffer = nil;
    current_vertex_count = 0;
    current_indexCount = 0;
    
    // Mettre en pause ? Ne plus caller draw (cette function)
    if(ChronoRender_shouldSleep() && !noSleep)
        [view setPaused:YES];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    if(![view isKindOfClass:[CoqMetalView class]]) {
        printerror("MTKView is not a custom MetalView.");
        return;
    }
    CoqMetalView* metalView = (CoqMetalView*)view;
    [metalView updateRootFrame: size];
}



@end
