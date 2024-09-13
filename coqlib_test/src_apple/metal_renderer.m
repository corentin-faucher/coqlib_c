//
//  MyMTKViewDelegate.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import "metal_renderer.h"
#import "metal_view.h"

#include "graph__apple.h"
#include "util_apple.h"

#include "my_enums.h"

#define WITH_FRAME_BUFFER 1

/// Init des shaders et du blending pour la première passe
id<MTLRenderPipelineState> RenderPipelineState_createFirst_(void) {
    id<MTLDevice> device = CoqGraph_getMTLDevice();
    if(!device) { printerror("no device."); return nil; }
    id<MTLLibrary> library = [device newDefaultLibrary];
    if(library == nil) { printerror("no library."); return nil; }
    MTLRenderPipelineDescriptor *pipeline_descr = [[MTLRenderPipelineDescriptor alloc] init];
    // 1. Set shaders
    #if TARGET_OS_OSX == 1
    pipeline_descr.vertexFunction = [library newFunctionWithName:@"vertex_function"];
#else
    if(@available(iOS 14.0, *)) {
        rpd.vertexFunction = [library newFunctionWithName:@"vertex_function"];
    } else {
        rpd.vertexFunction = [library newFunctionWithName:@"vertex_function_ios13"];
    }
#endif
    pipeline_descr.fragmentFunction = [library newFunctionWithName:@"fragment_function"];
    if(pipeline_descr.vertexFunction == nil || pipeline_descr.fragmentFunction == nil) {
        printerror("no vertex or fragment function."); return nil;
    }
    pipeline_descr.depthAttachmentPixelFormat = CoqGraph_getDepthPixelFormat();
    
    colorattachment_initDefaultBlending(pipeline_descr.colorAttachments[0]);
    
    return [device newRenderPipelineStateWithDescriptor:pipeline_descr error:nil];
}
/// Infos et texture pour la premiere passe (dessinnage ordinaire sur texture de couleur, lumière,...)
MTLRenderPassDescriptor* RenderPassDescriptor_createFirst_(CGSize size, MTLClearColor clearColor) {
    MTLRenderPassDescriptor* passDescr = [MTLRenderPassDescriptor renderPassDescriptor];
    id<MTLDevice> device = CoqGraph_getMTLDevice();
    int32_t height = (int32_t)fminf(size.height, 1080);
    int32_t width =  (int32_t)((size.width/size.height) * (float)height);
    if(device == nil) { printerror("No device"); return nil; }
    MTLPixelFormat pixelFormat = CoqGraph_getPixelFormat();
    
    // 0 : Texture Couleur, où on dessine la couleur
    MTLTextureDescriptor* texDescr = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                        width:width height:height mipmapped:NO];
    [texDescr setUsage:(MTLTextureUsageRenderTarget|MTLTextureUsageShaderRead)];
    id<MTLTexture> colorTex = [device newTextureWithDescriptor:texDescr];
    passDescr.colorAttachments[0].texture = colorTex;
    [passDescr.colorAttachments[0] setLoadAction:MTLLoadActionClear];
    [passDescr.colorAttachments[0] setClearColor:clearColor];
    // Setter dans Coq Texture pour utliser l'"écran" comme une texture... (Amusant, utile ?)
    Texture_setFrameBufferWithMTLTexture(colorTex, 0);
    
//    
//    // Texture Lumière : où on dessine les lumières (permet de voir la couleur)
//    id<MTLTexture> lightTex = [device newTextureWithDescriptor:texDescr];
//    passDescr.colorAttachments[1].texture = lightTex;
//    [passDescr.colorAttachments[1] setLoadAction:MTLLoadActionClear];
//    [passDescr.colorAttachments[1] setClearColor:MTLClearColorMake(0, 0, 0, 1)];
//    Texture_setFrameBufferWithMTLTexture(lightTex, 1);
//    
//    // Texture Effect
//    id<MTLTexture> eff1Tex = [device newTextureWithDescriptor:texDescr];
//    passDescr.colorAttachments[2].texture = eff1Tex;
//    [passDescr.colorAttachments[2] setLoadAction:MTLLoadActionClear];
//    Texture_setFrameBufferWithMTLTexture(eff1Tex, 2);
//    
//    // Texture Effect2
//    id<MTLTexture> eff2Tex = [device newTextureWithDescriptor:texDescr];
//    passDescr.colorAttachments[3].texture = eff2Tex;
//    [passDescr.colorAttachments[3] setLoadAction:MTLLoadActionClear];
//    Texture_setFrameBufferWithMTLTexture(eff2Tex, 3);
    
    // Texture de profondeur (depth texture)
    MTLTextureDescriptor* depthTexDescriptor = [MTLTextureDescriptor
                        texture2DDescriptorWithPixelFormat:CoqGraph_getDepthPixelFormat()
                        width:size.width height:size.height mipmapped:NO];
    [depthTexDescriptor setUsage:MTLTextureUsageRenderTarget];
    [depthTexDescriptor setStorageMode:MTLStorageModePrivate];
    passDescr.depthAttachment.texture = [device newTextureWithDescriptor:depthTexDescriptor];

    
    return passDescr;
}

/// Init des shaders pour la deuxième passe (pas de blending)
id<MTLRenderPipelineState> RenderPipelineState_createSecond_(void) {
    id<MTLDevice> device = CoqGraph_getMTLDevice();
    if(!device) { printerror("no device."); return nil; }
    id<MTLLibrary> library = [device newDefaultLibrary];
    if(library == nil) { printerror("no library."); return nil; }
    MTLRenderPipelineDescriptor *pipeline_descr = [[MTLRenderPipelineDescriptor alloc] init];
    // 1. Set shaders
    pipeline_descr.vertexFunction = [library newFunctionWithName:@"second_vertex_function"];
    pipeline_descr.fragmentFunction = [library newFunctionWithName:@"second_fragment_function"];
    pipeline_descr.colorAttachments[0].pixelFormat = CoqGraph_getPixelFormat();
    
    return [device newRenderPipelineStateWithDescriptor:pipeline_descr error:nil];
}

@implementation Renderer

- (instancetype)initWithView:(MTKView*)view {
    self = [super init];
    id<MTLDevice> device = view.device;
    if(device == nil) {
        printerror("no device.");
        return self;
    }
    
    chrono.isRendering = true;
    chrono_start(&chrono);
    clearColor = MTLClearColorMake(0.5, 0.5, 0.5, 1);
    
    // Set de la view pour le renderer (pixel format frame buffer)
    view.colorPixelFormat = CoqGraph_getPixelFormat();
//    view.depthStencilPixelFormat = MTL_depthPixelFormat_;
    if(WITH_FRAME_BUFFER)
        [view setFramebufferOnly:false];

    //-- Command queue / Liste des commande pour dessiner sur une texture --
    queue = [device newCommandQueue];
    
    //-- Init du pipeline / shaders --
    firstPipelineState =    RenderPipelineState_createFirst_();
    firstRenderPassDescr =  RenderPassDescriptor_createFirst_(view.drawableSize, clearColor);
    
    secondPipelineState =   RenderPipelineState_createSecond_();
    
    //-- Depth (si besoin) --/
    depthStencilState = nil;
//    MTLDepthStencilDescriptor *dsd = [MTLDepthStencilDescriptor new];
//    dsd.depthCompareFunction = MTLCompareFunctionLess;
//    [dsd setDepthWriteEnabled:YES];
//    depthStencilState = [device newDepthStencilStateWithDescriptor:dsd];

    // Texture de bruit et spot pour effet spéciaux.
    
    return self;
}

- (void)firstRenderPass:(id<MTLCommandBuffer>)commandBuffer withRoot:(Root*)root {
    // (Defaut descriptor est : view.currentRenderPassDescriptor)
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:firstRenderPassDescr];
   
    [commandEncoder setRenderPipelineState:firstPipelineState];
    if(depthStencilState != nil)
        [commandEncoder setDepthStencilState:depthStencilState];
    
    // Temps
    float timeAngle = ChronoRender_elapsedAngleSec();
    [commandEncoder setFragmentBytes:&timeAngle length:sizeof(float) atIndex:0];

    // Clear color
    clearColor = vector4_toMTLClearColor(fl_array_toVec4(root->back_RGBA));
    [firstRenderPassDescr.colorAttachments[0] setClearColor:clearColor];
    
    // Prepare pour dessiner
    commandencoder_initForDrawing(commandEncoder);
    // Drawing
    Squirrel sq;
    // (Préparation des fragment uniform de la passe finale.)
    fu = (FinalFragmentUniforms){ 0 };
    fu.extra0 = fl_pos(&root->ambiantLight);
    fu.extra1 = timeAngle;
    sq_init(&sq, (Node*)root, sq_scale_ones);
    do {
        sq.pos->updateModel(sq.pos);
        const Drawable* d = node_asDrawableOpt(sq.pos); 
        if(d && (d->n.flags & flag_drawableActive)) {
            commandencoder_drawDrawable(commandEncoder, d);
            continue;
        }
    } while(sq_goToNextToDisplay(&sq));
    
    // Fin du dessinage.
    [commandEncoder endEncoding];
}
- (void)secondRenderPass:(id<MTLCommandBuffer>)commandBuffer toView:(MTKView*)view {
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:view.currentRenderPassDescriptor];
    [view.currentRenderPassDescriptor.colorAttachments[0] setClearColor:MTLClearColorMake(0, 1, 0, 1)];
    [commandEncoder setRenderPipelineState:secondPipelineState];
    [commandEncoder setFragmentTexture:firstRenderPassDescr.colorAttachments[0].texture atIndex:0];
    [commandEncoder setFragmentSamplerState:CoqGraph_getSampler(true) atIndex:0];
    [commandEncoder setCullMode:(MTLCullMode)mesh_shaderQuad_->cull_mode];
    [commandEncoder setVertexBuffer:mesh_MTLVerticesBuffer(mesh_shaderQuad_) offset:0 atIndex:0];
    [commandEncoder drawPrimitives:(MTLPrimitiveType)mesh_shaderQuad_->primitive_type vertexStart:0 vertexCount:mesh_shaderQuad_->vertex_count];
    [commandEncoder endEncoding];
}


- (void)drawInMTKView:(nonnull MTKView *)view {
    if(view.isPaused) {
        return; 
    }
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
#if TARGET_OS_OSX != 1
    if(metalView.transitioning) {
        CALayer* presentation = [[metalView layer] presentationLayer];
        if(!metalView.didTransition && (presentation != nil)) {
            Vector2 frameSizePt = CGSize_toVector2([presentation bounds].size);
//#warning Fix frame size plutôt ?
            root_justSetFrameSize_(root, frameSizePt);
        } else {
            metalView.transitioning = NO;
            metalView.didTransition = NO;
            [metalView updateRootFrame: [view frame].size dontFix:NO];
        }
    }
#endif
    // Command buffer, command encoder... (a chaque frame)
    if(queue == nil || firstPipelineState == nil) return;
    id<MTLCommandBuffer> cb = [queue commandBuffer];
    if(cb == nil) { printerror("no command buffer."); return; }
    
    // 1. Temps de frame.
    float deltaTMS = fminf(60.f, fmaxf(3.f, (float)chrono_elapsedMS(&chronoDeltaT)));
    chrono_start(&chronoDeltaT);
    fl_set(&smDeltaT, deltaTMS);
    deltaTMS = fl_pos(&smDeltaT);
    int64_t deltaTMSint = rand_float_toInt(deltaTMS);
    ChronoRender_update(deltaTMSint);
    
    [self firstRenderPass:cb withRoot:root];
    [self secondRenderPass:cb toView:view];
    
    // Pour copier dans l'écran.
//    id<MTLBlitCommandEncoder> blitEncoder = [cb blitCommandEncoder];
//    Texture* tex = texture_frameBuffers[4];
//    MTLSize size = MTLSizeMake(tex->sizes.w, tex->sizes.h, 1);
//    [blitEncoder copyFromTexture:texture_MTLTexture(texture_frameBuffers[4])  sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(0, 0, 0) sourceSize:size toTexture:view.currentDrawable.texture destinationSlice:0 destinationLevel:0 destinationOrigin:MTLOriginMake(0, 0, 0)];
//    [blitEncoder endEncoding];
        
    // Présenter la drawable de la view.
    [cb presentDrawable:[view currentDrawable]];
    [cb commit];
    cb = nil;
    
    // Mettre en pause ? Ne plus caller draw (cette function)
    if(ChronoRender_shouldSleep() && !noSleep)
        [view setPaused:YES];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    if(![view isKindOfClass:[CoqMetalView class]]) {
        printerror("MTKView is not a custom MetalView.");
        return;
    }
    
    firstRenderPassDescr = RenderPassDescriptor_createFirst_(size, clearColor);
    
    CoqMetalView* metalView = (CoqMetalView*)view;
    if(!metalView.transitioning) [metalView updateRootFrame:size dontFix:NO];
}


@end
