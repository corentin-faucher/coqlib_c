//
//  MyMTKViewDelegate.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import "metal_renderer.h"

#include "my_enums.h"

#define WITH_FRAME_BUFFER 1
#define RENDERER_HEIGHT 640

@implementation Renderer

- (instancetype)initWithView:(MTKView*)view {
    self = [super init];
    guard_let(id<MTLDevice>, device, view.device, printerror("No device."), self)
    
    // Set de la view pour le renderer (pixel format frame buffer)
    view.colorPixelFormat = CoqMtl_pixelFormat;
    if(WITH_FRAME_BUFFER)
        [view setFramebufferOnly:false];

    //-- Command queue / Liste des commande pour dessiner sur une texture --
    queue = [device newCommandQueue];
    
    //-- Init du pipeline (shaders) --
    // First pass
    MTLRenderPipelineDescriptor* descr = MTLRenderPipelineDescriptor_create(device, "vertex_function", "fragment_function", 1);
    firstPipelineState = [device newRenderPipelineStateWithDescriptor:descr error:nil];
    
    float ratio = view.drawableSize.width / view.drawableSize.height;
    fb = CoqFramebuffer_create(device, 1, ratio * RENDERER_HEIGHT, RENDERER_HEIGHT, descr);
    
    // Second pass
    descr = MTLRenderPipelineDescriptor_create(device, "second_vertex_function", "second_fragment_function", 1);
    secondPipelineState = [device newRenderPipelineStateWithDescriptor:descr error:nil];
    
    return self;
}

-(void)dealloc {
    coqframebuffer_deinit(&fb);
}

- (void)firstRenderPass:(id<MTLCommandBuffer>)commandBuffer withRoot:(Root *const)root {
    // (Defaut descriptor est : view.currentRenderPassDescriptor)
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:fb.renderPassDescr];
   
    [commandEncoder setRenderPipelineState:firstPipelineState];
//    if(depthStencilState != nil)
//        [commandEncoder setDepthStencilState:depthStencilState];
    
    // Temps
    float timeAngle = RendererTimeCapture_elapsedAngleSec();
    [commandEncoder setFragmentBytes:&timeAngle length:sizeof(float) atIndex:0];

    // Clear color
    coqframebuffer_updateClearColor(&fb, fl_array_toVec4(root->back_RGBA));
    // Si "one pass", on set la clear color directement dans view.clearColor.
    // view.clearColor = clearColor;
    
    // Prepare pour dessiner
    commandencoder_initForDrawing(commandEncoder);
    // Drawing
    Node* sq = &root->n;
    do {
        sq->renderer_updateInstanceUniforms(sq);
        if(!(sq->renderIU.flags & renderflag_toDraw)) {
            continue;
        }
        if_let(DrawableMulti const*, dm, node_asDrawableMultiOpt(sq))
            commandencoder_setCurrentMesh(commandEncoder, dm->d._mesh);
            commandencoder_setCurrentTexture(commandEncoder, dm->d._tex);
            commandencoder_setIUs(commandEncoder, &dm->iusBuffer);
            commandencoder_drawWithCurrents(commandEncoder);
            continue;
        if_let_end
        if_let(Drawable const*, d, node_asDrawableOpt(sq))
            commandencoder_setCurrentMesh(commandEncoder, d->_mesh);
            commandencoder_setCurrentTexture(commandEncoder, d->_tex);
            commandencoder_setIU(commandEncoder, &d->n.renderIU);
            commandencoder_drawWithCurrents(commandEncoder);
            continue;
        if_let_end
    } while(nodeptr_renderer_goToNextToDisplay(&sq));
    // Fin du dessinage.
    [commandEncoder endEncoding];
}

- (void)secondRenderPass:(id<MTLCommandBuffer>)commandBuffer toView:(MTKView*)view {
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:view.currentRenderPassDescriptor];
    [commandEncoder setRenderPipelineState:secondPipelineState];
    [commandEncoder setFragmentTexture:fb.renderPassDescr.colorAttachments[0].texture atIndex:0];
    [commandEncoder setFragmentSamplerState:CoqMtl_samplerNearest atIndex:0];
    
    commandencoder_drawRenderQuad(commandEncoder);

    [commandEncoder endEncoding];
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    if(view.isPaused) {
        return; 
    }
    guard_let(Root*, root, CoqEvent_rootOpt, printerror("Root not set."), )
    guard_let(CoqMetalView*, metalView, mtkView_asCoqMetalViewOpt(view), , )
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
    
    RendererTimeCapture_update();
    
    [self firstRenderPass:cb withRoot:root];
    [self secondRenderPass:cb toView:view];
        
    // Présenter la drawable de la view.
    [cb presentDrawable:[view currentDrawable]];
    [cb commit];
    cb = nil;
    
    // Faire les vérifications de garbage nodes et textures...
    [metalView checksAfterRendererDraw];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size 
{
    guard_let(CoqMetalView*, metalView, mtkView_asCoqMetalViewOpt(view), , )
    float ratio = size.width / size.height;
    coqframebuffer_resize(&fb, view.device, ratio * RENDERER_HEIGHT, RENDERER_HEIGHT);
    if(!metalView.transitioning) [metalView updateRootFrame:size dontFix:NO];
}

@end
