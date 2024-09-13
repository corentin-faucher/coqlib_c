//
//  graph__apple.m
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 5/6/24.
//

#import <MetalKit/MetalKit.h>

#include "graph__apple.h"

#include "utils/util_base.h"

#pragma mark - Metal globals --------------------

static id<MTLDevice>       MTL_device_ = nil;
static MTLPixelFormat      MTL_pixelFormat_ = MTLPixelFormatBGRA8Unorm;
static MTLPixelFormat      MTL_depthPixelFormat_ = MTLPixelFormatDepth16Unorm;
static id<MTLSamplerState> MTL_samplerStateLinear_ = nil;
static id<MTLSamplerState> MTL_samplerStateNearest_ = nil;

void CoqGraph_MTLinit(id<MTLDevice> device, MTLPixelFormat pixelFormatOpt, MTLPixelFormat depthPixelFormatOpt) {
    MTL_device_ = device;
    if(pixelFormatOpt != MTLPixelFormatInvalid) // (invalide est 0, i.e. on modifie la valeur par défaut si valide)
        MTL_pixelFormat_ = pixelFormatOpt;
    if(depthPixelFormatOpt != MTLPixelFormatInvalid)
        MTL_depthPixelFormat_ = depthPixelFormatOpt;
    Texture_MTLinit_();
    
    MTLSamplerDescriptor *sd = [MTLSamplerDescriptor new];
    sd.magFilter = MTLSamplerMinMagFilterNearest;
    sd.minFilter = MTLSamplerMinMagFilterNearest;
    [sd setRAddressMode:MTLSamplerAddressModeRepeat];
    [sd setSAddressMode:MTLSamplerAddressModeRepeat];
    [sd setTAddressMode:MTLSamplerAddressModeRepeat];
    MTL_samplerStateNearest_ = [MTL_device_ newSamplerStateWithDescriptor:sd];
    sd.magFilter = MTLSamplerMinMagFilterLinear;
    sd.minFilter = MTLSamplerMinMagFilterLinear;
    MTL_samplerStateLinear_ = [MTL_device_ newSamplerStateWithDescriptor:sd];
}
id<MTLDevice> CoqGraph_getMTLDevice(void) {
    if(MTL_device_ == nil) {
        printerror("No CoqGraph_MTLInit ?"); return nil;
    }
    return MTL_device_;
}
MTLPixelFormat CoqGraph_getPixelFormat(void) {
    return MTL_pixelFormat_;
}
MTLPixelFormat CoqGraph_getDepthPixelFormat(void) {
    return MTL_depthPixelFormat_;
}
id<MTLSamplerState> CoqGraph_getSampler(bool nearest) {
    return nearest ? MTL_samplerStateNearest_ : MTL_samplerStateLinear_;
}


#pragma mark - Version Metal des Uniform Buffer -----------------------

/// Création du buffer. 
void   piusbuffer_init_(IUsBuffer* piusbuffer, uint32_t count) {
    size_t size = count * sizeof(InstanceUniforms);
    id<MTLBuffer> buffer_mtl = [MTL_device_ newBufferWithLength:size
                                    options:MTLResourceCPUCacheModeDefaultCache];
    uint_initConst(&piusbuffer->max_count, count);
    piusbuffer->actual_count = count;
    *(InstanceUniforms**)&piusbuffer->ius = [buffer_mtl contents];
    *(const void**)&piusbuffer->_mtlBuffer_cptr = CFBridgingRetain(buffer_mtl);
    buffer_mtl = nil;
}

/// Libère l'espace du buffer (et array de piu si nécessaire)
void   piusbuffer_deinit_(IUsBuffer* piusbuffer) {
    CFRelease(piusbuffer->_mtlBuffer_cptr);
    *(InstanceUniforms**)&piusbuffer->ius = NULL;
    *(const void**)&piusbuffer->_mtlBuffer_cptr = NULL;
}

id<MTLBuffer>  piusbuffer_asMTLBuffer(const IUsBuffer* piusbuffer) {
    return (__bridge id<MTLBuffer>)piusbuffer->_mtlBuffer_cptr;
}


#pragma mark - Operation sur les couleurs dans le pipeline (shaders) ------------------

void colorattachment_initDefaultBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment) {
    [colorAttachment setPixelFormat:MTL_pixelFormat_];
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
    [colorAttachment setPixelFormat:MTL_pixelFormat_];
    [colorAttachment setBlendingEnabled:YES];
    // NewPixel = (1-alpha)*old + alpha*added;
    [colorAttachment setRgbBlendOperation:MTLBlendOperationAdd];
    [colorAttachment setDestinationRGBBlendFactor:MTLBlendFactorOne];
    [colorAttachment setSourceRGBBlendFactor:MTLBlendFactorOne];

    [colorAttachment setAlphaBlendOperation:MTLBlendOperationAdd];
    [colorAttachment setDestinationAlphaBlendFactor:MTLBlendFactorOne];
    [colorAttachment setSourceAlphaBlendFactor:MTLBlendFactorOne];
}


#pragma mark - Dessiner avec le command encoder ----------

static Texture*         currentTex;
static bool             currentTex_isNearest = false;
static Mesh*            currentMesh;
static MTLPrimitiveType currentMesh_primitiveType;
static int              currentMesh_vertexCount;
static id<MTLBuffer>    currentMesh_indicesBufferOpt;
static uint32_t         currentMesh_indexCount;
static MTLCullMode      currentMesh_cullMode = MTLCullModeNone;

void commandencoder_initForDrawing(id<MTLRenderCommandEncoder> encoder) {
    //-- Sampler state pour les textures (pixelisé ou non) --/
    [encoder setFragmentSamplerState:(currentTex_isNearest ? MTL_samplerStateNearest_ : MTL_samplerStateLinear_)
                             atIndex:0];
    [encoder setCullMode:currentMesh_cullMode];
    currentTex = NULL;
    currentMesh = NULL;
}
void commandencoder_drawDrawable(id<MTLRenderCommandEncoder> encoder, const Drawable* const d) {
    // 1. Mise a jour de la mesh ?
    if(currentMesh != d->_mesh) {
        currentMesh = d->_mesh;
        currentMesh_primitiveType = (MTLPrimitiveType)currentMesh->primitive_type;
        currentMesh_vertexCount = currentMesh->vertex_count;
        currentMesh_indicesBufferOpt = mesh_MTLIndicesBufferOpt(currentMesh);
        currentMesh_indexCount = currentMesh->index_count;
        if(currentMesh_cullMode != (MTLCullMode)currentMesh->cull_mode) {
            currentMesh_cullMode = (MTLCullMode)currentMesh->cull_mode;
            [encoder setCullMode:currentMesh_cullMode];
        }
        id<MTLBuffer> buffer = mesh_MTLVerticesBuffer(currentMesh);
        [encoder setVertexBuffer:buffer offset:0 atIndex:0];
    }
    // 2. Mise a jour de la texture ?
    if(currentTex != d->_tex) {
        currentTex = d->_tex;
        bool newNearest = currentTex->flags & tex_flag_nearest;
        if(currentTex_isNearest != newNearest) {
            currentTex_isNearest = newNearest;
            [encoder setFragmentSamplerState:(currentTex_isNearest ? MTL_samplerStateNearest_ : MTL_samplerStateLinear_)
                                     atIndex:0];
        }
        [encoder setFragmentTexture:texture_MTLTexture(currentTex) atIndex:0];
        [encoder setFragmentBytes:&currentTex->sizes length:sizeof(Vector2) atIndex:1];
    }
    // 3. Per instance uniforms
    uint32_t instanceCount = 1;
    if(d->n._type & node_type_flag_drawMulti) {
        const DrawableMulti* dm = (const DrawableMulti*)d;
        instanceCount = dm->iusBuffer.actual_count;
        if(instanceCount == 0) return;
        id<MTLBuffer> mtlBuf = piusbuffer_asMTLBuffer(&dm->iusBuffer);
        [encoder setVertexBuffer:mtlBuf offset:0 atIndex:1];
    } else
        [encoder setVertexBytes:&d->n._iu length:sizeof(InstanceUniforms) atIndex:1];
    // 4. Dessiner
    if(currentMesh_indicesBufferOpt != nil) {
        [encoder drawIndexedPrimitives:currentMesh_primitiveType indexCount:currentMesh_indexCount indexType:MTLIndexTypeUInt16 indexBuffer:currentMesh_indicesBufferOpt indexBufferOffset:0 instanceCount:instanceCount];
    } else {
        [encoder drawPrimitives:currentMesh_primitiveType vertexStart:0 
            vertexCount:currentMesh_vertexCount instanceCount:instanceCount];
    }
}

