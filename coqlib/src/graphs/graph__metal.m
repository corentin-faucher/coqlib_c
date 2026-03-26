//
//  graph__metal.m
//  Fonction graphique général pour Metal.
//  Pour meshes et texture voir `graph_mesh_apple.m` et `graph_texture_apple.m`.
//
//  Created by Corentin Faucher on 5/6/24.
//

#import <MetalKit/MetalKit.h>

#include "graph__metal.h"

#include "utils/util_base.h"
#include "util_apple.h"
#include "systems/system_language.h"
#include "graphs/graph_mesh_private.h"

// MARK: - Metal globals

id<MTLDevice>       CoqMtl_device = nil;
MTLPixelFormat      CoqMtl_pixelFormat = MTLPixelFormatRGBA8Unorm; // MTLPixelFormatBGRA8Unorm;
MTLPixelFormat      CoqMtl_depthPixelFormat = MTLPixelFormatDepth16Unorm;
id<MTLSamplerState> CoqMtl_samplerNearest;
id<MTLSamplerState> CoqMtl_samplerNearestClamp;
id<MTLSamplerState> CoqMtl_samplerLinear;
static id<MTLBuffer> CoqMtl_renderQuadVerticesBuffer = nil;

void CoqMtl_init(id<MTLDevice> const device, MTLPixelFormat pixelFormatOpt, 
                         MTLPixelFormat depthPixelFormatOpt)
{
    CoqMtl_device = device;
    // Pixel format
    if(pixelFormatOpt != MTLPixelFormatInvalid) // (invalide est 0, i.e. on modifie la valeur par défaut si valide)
        CoqMtl_pixelFormat = pixelFormatOpt;
    if(depthPixelFormatOpt != MTLPixelFormatInvalid)
        CoqMtl_depthPixelFormat = depthPixelFormatOpt;
    // Sampler (smooth/linear ou pixelisé/nearest)
    MTLSamplerDescriptor *sd = [MTLSamplerDescriptor new];
    sd.magFilter = MTLSamplerMinMagFilterNearest;
    sd.minFilter = MTLSamplerMinMagFilterNearest;
    CoqMtl_samplerNearestClamp = [device newSamplerStateWithDescriptor:sd];
    [sd setRAddressMode:MTLSamplerAddressModeRepeat];
    [sd setSAddressMode:MTLSamplerAddressModeRepeat];
    [sd setTAddressMode:MTLSamplerAddressModeRepeat];
    CoqMtl_samplerNearest = [device newSamplerStateWithDescriptor:sd];
    sd.magFilter = MTLSamplerMinMagFilterLinear;
    sd.minFilter = MTLSamplerMinMagFilterLinear;
    CoqMtl_samplerLinear = [device newSamplerStateWithDescriptor:sd];
    
    // Textures et mesh init.
    Texture_metal_init_(device);
    Texture_init_();
    
    Mesh_engineIsMetal_ = true;
    // (x,y), (u,v).
    const float vertices[] = {
        -1.0, 1.0, 0.0, 0.0,
        -1.0,-1.0, 0.0, 1.0,
         1.0, 1.0, 1.0, 0.0,
         1.0,-1.0, 1.0, 1.0,
//        -1.0,-1.0, 0.0, 0.0,
//        -1.0, 1.0, 0.0, 1.0,
//         1.0,-1.0, 1.0, 0.0,
//         1.0,-1.0, 1.0, 0.0,
//         1.0, 1.0, 1.0, 1.0,
//        -1.0, 1.0, 0.0, 1.0,
    };
    CoqMtl_renderQuadVerticesBuffer = [device newBufferWithBytes:vertices
                                        length:sizeof(vertices) options:0];
}

// MARK: - Operation sur les couleurs dans le pipeline (shaders)

void colorattachment_initDefaultBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment) {
    [colorAttachment setPixelFormat:CoqMtl_pixelFormat];
    [colorAttachment setBlendingEnabled:YES];
    // NewPixel = (1-alpha)*old + alpha*added;
    [colorAttachment setRgbBlendOperation:MTLBlendOperationAdd];
    [colorAttachment setDestinationRGBBlendFactor:MTLBlendFactorOneMinusSourceAlpha];
#if TARGET_OS_OSX == 1
    [colorAttachment setSourceRGBBlendFactor:MTLBlendFactorSourceAlpha];
#else
    if(@available(iOS 14.0, *)) {
        [colorAttachment setSourceRGBBlendFactor:MTLBlendFactorSourceAlpha];
    } else { // ??
        [colorAttachment setSourceRGBBlendFactor:MTLBlendFactorOne];
    }
#endif
    
    [colorAttachment setAlphaBlendOperation:MTLBlendOperationAdd];
    [colorAttachment setDestinationAlphaBlendFactor:MTLBlendFactorOne];
    [colorAttachment setSourceAlphaBlendFactor:MTLBlendFactorOne];
}

void colorattachment_initLightBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment) {
    [colorAttachment setPixelFormat:CoqMtl_pixelFormat];
    [colorAttachment setBlendingEnabled:YES];
    // NewPixel = (1-alpha)*old + alpha*added;
    [colorAttachment setRgbBlendOperation:MTLBlendOperationAdd];
    [colorAttachment setDestinationRGBBlendFactor:MTLBlendFactorOne];
    [colorAttachment setSourceRGBBlendFactor:MTLBlendFactorOne];

    [colorAttachment setAlphaBlendOperation:MTLBlendOperationAdd];
    [colorAttachment setDestinationAlphaBlendFactor:MTLBlendFactorOne];
    [colorAttachment setSourceAlphaBlendFactor:MTLBlendFactorOne];
}
void colorattachment_initFourFloat(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment) {
    [colorAttachment setPixelFormat:MTLPixelFormatRGBA32Float];
    [colorAttachment setBlendingEnabled:YES];
    [colorAttachment setRgbBlendOperation:MTLBlendOperationAdd];
    [colorAttachment setDestinationRGBBlendFactor:MTLBlendFactorOne];
    [colorAttachment setSourceRGBBlendFactor:MTLBlendFactorOne];

    [colorAttachment setAlphaBlendOperation:MTLBlendOperationAdd];
    [colorAttachment setDestinationAlphaBlendFactor:MTLBlendFactorOne];
    [colorAttachment setSourceAlphaBlendFactor:MTLBlendFactorOne];
}

// MARK: Render Pipeline State
/// Création de la pipeline, équivalent du `program` dans OpenGL : vertex shader -> fragment shader.
/// Les color attachments sont initialisés avec des pixels par défaut `CoqMtl_pixelFormat`
/// et le blending standard (transparence  `NewPixel = (1-alpha)*old + alpha*added`).
/// On peut modifier les cas particulier de `colorAttachments` au besoin 
/// (voir les fonctions `colorattachment_init...`).
MTLRenderPipelineDescriptor* MTLRenderPipelineDescriptor_create(
                        id<MTLDevice> const device,
                        const char*const vertexFunctionName,
                        const char*const fragmentFunctionName,
                        uint32_t const colorAttachmentCount)
{
    if(!device) { printerror("no device."); return nil; }
    id<MTLLibrary> library = [device newDefaultLibrary];
    if(library == nil) { printerror("no library."); return nil; }
    MTLRenderPipelineDescriptor *descr = [[MTLRenderPipelineDescriptor alloc] init];
    // Shaders functions
    descr.vertexFunction = [library newFunctionWithName:
            [NSString stringWithUTF8String:vertexFunctionName]];
    descr.fragmentFunction = [library newFunctionWithName:
            [NSString stringWithUTF8String:fragmentFunctionName]];
    if(descr.vertexFunction == nil || descr.fragmentFunction == nil) {
        printerror("no vertex or fragment function."); return nil;
    }
    // Init color attachments
    for(int colorId = 0; colorId < colorAttachmentCount; colorId++) {
        colorattachment_initDefaultBlending(descr.colorAttachments[colorId]);
    }
    
    return descr;
}

// MARK: Render pass descriptor / Framebuffer
// Equivalent du framebuffer dans OpenGL, là où on dessine (avec color attachments). 
// (Par défaut on prend la RenderPassDescr de la window view).
void coqframebuffer_setColorAttTex_(CoqFramebuffer* fb, id<MTLDevice> const device)
{
    if(device == nil) { printerror("No device"); return; }
    // Refaire des texture avec la nouvelle taille.
    
    MTLClearColor clearColorMtl = vector4_toMTLClearColor(fb->clearColor);
    // Créer les textures "color attachments".
    for(int i = 0; i < fb->colorAttachmentCount; i++) {
        MTLPixelFormat pixelFormat = fb->pipelineDescr.colorAttachments[i].pixelFormat;
        MTLTextureDescriptor* texDescr = [MTLTextureDescriptor
                    texture2DDescriptorWithPixelFormat:pixelFormat 
                    width:fb->width height:fb->height mipmapped:NO];
        [texDescr setUsage:(MTLTextureUsageRenderTarget|MTLTextureUsageShaderRead)];
        id<MTLTexture> colorTex = [device newTextureWithDescriptor:texDescr];
        fb->renderPassDescr.colorAttachments[i].texture = colorTex;
        [fb->renderPassDescr.colorAttachments[i] setLoadAction:MTLLoadActionClear];
        [fb->renderPassDescr.colorAttachments[i] setClearColor:clearColorMtl];
        // Setter dans Coq Texture pour utliser l'"écran" comme une texture... (Amusant, utile ?)
        Texture_metal_setFrameBufferToMTLTexture(i, colorTex);
    }
        // Texture de profondeur (depth texture)
//    MTLTextureDescriptor* depthTexDescriptor = [MTLTextureDescriptor
//                        texture2DDescriptorWithPixelFormat:CoqMtl_depthPixelFormat
//                        width:width height:height mipmapped:NO];
//    [depthTexDescriptor setUsage:MTLTextureUsageRenderTarget];
//    [depthTexDescriptor setStorageMode:MTLStorageModePrivate];
//    passDescr.depthAttachment.texture = [device newTextureWithDescriptor:depthTexDescriptor];
}
CoqFramebuffer CoqFramebuffer_create(
        id<MTLDevice> const device,
        uint32_t colorAttachmentCount,
        uint32_t width, uint32_t height,
        MTLRenderPipelineDescriptor* descr) 
{
    CoqFramebuffer fb = {
        .renderPassDescr = [MTLRenderPassDescriptor renderPassDescriptor],
        .colorAttachmentCount = colorAttachmentCount,
        .width = width, .height = height,
        .pipelineDescr = descr,
    };
    coqframebuffer_setColorAttTex_(&fb, device);

    return fb;
}
void coqframebuffer_resize(CoqFramebuffer*const fb,
    id<MTLDevice> const device, 
    uint32_t width, uint32_t height) 
{

    if(width == fb->width && height == fb->height) return;
    fb->width = width; fb->height = height;
    coqframebuffer_setColorAttTex_(fb, device);
}
void coqframebuffer_updateClearColor(CoqFramebuffer*const fb, Vector4 const clearColor) {
    fb->clearColor = clearColor;
    MTLClearColor clearColorMtl = vector4_toMTLClearColor(clearColor);
    [fb->renderPassDescr.colorAttachments[0] setClearColor:clearColorMtl];
}
void coqframebuffer_deinit(CoqFramebuffer*const fb) {
    fb->pipelineDescr = nil;
    fb->renderPassDescr = nil;
}



// MARK: - Dessiner avec le command encoder ----------
static Texture const   *currentTex;
static bool             lastIsNearest;

static Mesh const      *currentMesh;
static MeshToDraw       currentMeshToDraw;
static MTLCullMode      lastCullMode = MTLCullModeNone;

static size_t           currentIUS_instanceCount = 1;

void commandencoder_initForDrawing(id<MTLRenderCommandEncoder> encoder) {
    //-- Sampler state pour les textures (pixelisé ou non) --/
    [encoder setFragmentSamplerState:(lastIsNearest ? CoqMtl_samplerNearest : CoqMtl_samplerLinear)
                             atIndex:0];
    [encoder setCullMode:lastCullMode];
    currentTex = NULL;
    currentMesh = NULL;
}
void commandencoder_setCurrentMeshNoCheck(id<MTLRenderCommandEncoder> const encoder, Mesh const *const newMesh) {
    if(newMesh == currentMesh) { return; }
    currentMesh = newMesh;
    currentMeshToDraw = mesh_render_getToDraw(newMesh);
    if((MTLCullMode)currentMeshToDraw.cull_mode != lastCullMode) {
        lastCullMode = (MTLCullMode)currentMeshToDraw.cull_mode;
        [encoder setCullMode:(MTLCullMode)currentMeshToDraw.cull_mode];
    }
    if(currentMeshToDraw.metal_verticesMTLBufferOpt) {
        [encoder setVertexBuffer:(__bridge id<MTLBuffer>)currentMeshToDraw.metal_verticesMTLBufferOpt offset:0 atIndex:0];
        return;
    }
    if(!currentMeshToDraw.metal_verticesOpt) { printerror("Mesh without buffer or verticesRead."); return; }
    [encoder setVertexBytes:currentMeshToDraw.metal_verticesOpt length:currentMeshToDraw.verticesSize atIndex:0];
}
void commandencoder_setCurrentMesh(id<MTLRenderCommandEncoder> const encoder, Mesh *const newMesh) {
    if(newMesh == currentMesh) { return; }
    currentMesh = newMesh;
    mesh_render_checkMeshInit(newMesh);
    mesh_render_checkMeshUpdate(newMesh);
    currentMeshToDraw = mesh_render_getToDraw(newMesh);
    if((MTLCullMode)currentMeshToDraw.cull_mode != lastCullMode) {
        lastCullMode = (MTLCullMode)currentMeshToDraw.cull_mode;
        [encoder setCullMode:(MTLCullMode)currentMeshToDraw.cull_mode];
    }
    if(currentMeshToDraw.metal_verticesMTLBufferOpt) {
        [encoder setVertexBuffer:(__bridge id<MTLBuffer>)currentMeshToDraw.metal_verticesMTLBufferOpt offset:0 atIndex:0];
        return;
    }
    if(!currentMeshToDraw.metal_verticesOpt) { printerror("Mesh without buffer or verticesRead."); return; }
    [encoder setVertexBytes:currentMeshToDraw.metal_verticesOpt length:currentMeshToDraw.verticesSize atIndex:0];
}
void commandencoder_setCurrentTexture(id<MTLRenderCommandEncoder> encoder, Texture *const newTex) {
    if(currentTex == newTex) { return; }
    currentTex = newTex;
    texture_render_checkTexture(newTex);
    TextureToDraw const currentTexToDraw = texture_render_getToDraw(newTex);
    if(currentTexToDraw.isNearest != lastIsNearest) {
        lastIsNearest = currentTexToDraw.isNearest;
        [encoder setFragmentSamplerState:(currentTexToDraw.isNearest ? CoqMtl_samplerNearest : CoqMtl_samplerLinear)
                                 atIndex:0];
    }
    id<MTLTexture> mtlTexture = (__bridge id<MTLTexture>)currentTexToDraw.mtlTexture;
    [encoder setFragmentTexture:mtlTexture atIndex:0];
}
void commandencoder_setCurrentTextureNoCheck(id<MTLRenderCommandEncoder> const encoder, Texture*const newTex) {
    if(currentTex == newTex) { return; }
    currentTex = newTex;
    TextureToDraw currentTexToDraw = texture_render_getToDraw(newTex);
    if(currentTexToDraw.isNearest != lastIsNearest) {
        lastIsNearest = currentTexToDraw.isNearest;
        [encoder setFragmentSamplerState:(currentTexToDraw.isNearest ? CoqMtl_samplerNearest : CoqMtl_samplerLinear)
                                 atIndex:0];
    }
    id<MTLTexture> mtlTexture = (__bridge id<MTLTexture>)currentTexToDraw.mtlTexture;
    [encoder setFragmentTexture:mtlTexture atIndex:0];
}
void commandencoder_setIU(id<MTLRenderCommandEncoder> encoder, InstanceUniforms const*const iu) {
    currentIUS_instanceCount = 1;
    [encoder setVertexBytes:iu length:sizeof(InstanceUniforms) atIndex:1];
}
void commandencoder_setIUs(id<MTLRenderCommandEncoder> encoder, IUsBuffer const*const ius) {
    currentIUS_instanceCount = ius->actual_count;
    if(ius->mtlBufferOpt) {
        [encoder setVertexBuffer:(__bridge id<MTLBuffer>)ius->mtlBufferOpt offset:0 atIndex:1];
        return;
    }
    if(!ius->iusOpt) { printerror("IUSBuffer without buffer or ius."); return; }
    [encoder setVertexBytes:ius->iusOpt length:ius->actual_size atIndex:1];
}
void commandencoder_drawWithCurrents(id<MTLRenderCommandEncoder> encoder) {
    if(!currentIUS_instanceCount) return;
    if(currentMeshToDraw.metal_indicesMTLBufersOpt) {
        [encoder drawIndexedPrimitives:currentMeshToDraw.metal_primitiveType indexCount:currentMeshToDraw.indexCount
            indexType:MTLIndexTypeUInt16 indexBuffer:(__bridge id<MTLBuffer>)currentMeshToDraw.metal_indicesMTLBufersOpt 
            indexBufferOffset:0 instanceCount:currentIUS_instanceCount];
    } else {
        [encoder drawPrimitives:currentMeshToDraw.metal_primitiveType vertexStart:0 
            vertexCount:currentMeshToDraw.vertexCount instanceCount:currentIUS_instanceCount];
    }
}

void commandencoder_drawRenderQuad(id<MTLRenderCommandEncoder> encoder) {
    [encoder setCullMode:MTLCullModeNone];
    [encoder setVertexBuffer:CoqMtl_renderQuadVerticesBuffer offset:0 atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 
                       vertexCount:4];
}

// Equivalent de :
// commandencoder_setCurrentTexture(encoder, d->_tex);
// commandencoder_setCurrentMesh(encoder, d->_mesh);
// commandencoder_setIU(encoder, &d->n.renderIU);
// commandencoder_drawWithCurrents(encoder);
//void commandencoder_drawDrawable(id<MTLRenderCommandEncoder> encoder, Drawable*const d) 
//{
//
//}
