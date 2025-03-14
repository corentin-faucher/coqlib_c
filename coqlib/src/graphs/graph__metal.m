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
#include "utils/util_string.h"
#include "utils/util_language.h"

// MARK: - Metal globals
id<MTLDevice>       CoqGraph_metal_device = nil;
MTLPixelFormat      CoqGraph_metal_pixelFormat = MTLPixelFormatBGRA8Unorm;
MTLPixelFormat      CoqGraph_metal_depthPixelFormat = MTLPixelFormatDepth16Unorm;
id<MTLSamplerState> CoqGraph_metal_samplerNearest;
id<MTLSamplerState> CoqGraph_metal_samplerNearestClamp;
id<MTLSamplerState> CoqGraph_metal_samplerLinear;

void CoqGraph_metal_init(id<MTLDevice> device, MTLPixelFormat pixelFormatOpt, 
                         MTLPixelFormat depthPixelFormatOpt,
                         MeshInit const*const drawableSpriteInitOpt,
                         MeshInit const*const renderingQuadInitOpt)
{
    CoqGraph_metal_device = device;
    // Pixel format
    if(pixelFormatOpt != MTLPixelFormatInvalid) // (invalide est 0, i.e. on modifie la valeur par défaut si valide)
        CoqGraph_metal_pixelFormat = pixelFormatOpt;
    if(depthPixelFormatOpt != MTLPixelFormatInvalid)
        CoqGraph_metal_depthPixelFormat = depthPixelFormatOpt;
    // Sampler (smooth/linear ou pixelisé/nearest)
    MTLSamplerDescriptor *sd = [MTLSamplerDescriptor new];
    sd.magFilter = MTLSamplerMinMagFilterNearest;
    sd.minFilter = MTLSamplerMinMagFilterNearest;
    CoqGraph_metal_samplerNearestClamp = [CoqGraph_metal_device newSamplerStateWithDescriptor:sd];
    [sd setRAddressMode:MTLSamplerAddressModeRepeat];
    [sd setSAddressMode:MTLSamplerAddressModeRepeat];
    [sd setTAddressMode:MTLSamplerAddressModeRepeat];
    CoqGraph_metal_samplerNearest = [CoqGraph_metal_device newSamplerStateWithDescriptor:sd];
    sd.magFilter = MTLSamplerMinMagFilterLinear;
    sd.minFilter = MTLSamplerMinMagFilterLinear;
    CoqGraph_metal_samplerLinear = [CoqGraph_metal_device newSamplerStateWithDescriptor:sd];
    
    // Textures et mesh init.
    Texture_metal_init_();
    Texture_init_();
    Mesh_initDefaultMeshes_(drawableSpriteInitOpt, renderingQuadInitOpt);
}

// MARK: - Operation sur les couleurs dans le pipeline (shaders)

void colorattachment_initDefaultBlending(MTLRenderPipelineColorAttachmentDescriptor * colorAttachment) {
    [colorAttachment setPixelFormat:CoqGraph_metal_pixelFormat];
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
    [colorAttachment setPixelFormat:CoqGraph_metal_pixelFormat];
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


// MARK: - Dessiner avec le command encoder ----------

static Texture         *currentTex;
static TextureToDraw    currentTexToDraw;
static bool             lastIsNearest;

static Mesh            *currentMesh;
static MeshToDraw       currentMeshToDraw;
static MTLCullMode      lastCullMode = MTLCullModeNone;

static size_t           currentIUS_instanceCount = 1;

void commandencoder_initForDrawing(id<MTLRenderCommandEncoder> encoder) {
    //-- Sampler state pour les textures (pixelisé ou non) --/
    [encoder setFragmentSamplerState:(lastIsNearest ? CoqGraph_metal_samplerNearest : CoqGraph_metal_samplerLinear)
                             atIndex:0];
    [encoder setCullMode:lastCullMode];
    currentTex = NULL;
    currentMesh = NULL;
}
//bool CommandEncoder_isCurrentMesh(Mesh const*const mesh) {
//    return mesh == currentMesh;
//}
void commandencoder_setCurrentMesh(id<MTLRenderCommandEncoder> encoder, Mesh *const newMesh) {
    if(newMesh == currentMesh) { return; }
    currentMesh = newMesh;
    mesh_render_tryToUpdateVerticesAndIndiceCount(newMesh);
    currentMeshToDraw = mesh_render_getMeshToDraw(newMesh);
    if((MTLCullMode)currentMeshToDraw.cull_mode != lastCullMode) {
        lastCullMode = (MTLCullMode)currentMeshToDraw.cull_mode;
        [encoder setCullMode:(MTLCullMode)currentMeshToDraw.cull_mode];
    }
    id<MTLBuffer> verticesBufferOpt = (__bridge id<MTLBuffer>)currentMeshToDraw.metal_verticesBufferOpt_cptr;
    if(verticesBufferOpt)
        [encoder setVertexBuffer:verticesBufferOpt offset:0 atIndex:0];
    else {
        if(currentMeshToDraw.verticesOpt)
            [encoder setVertexBytes:currentMeshToDraw.verticesOpt length:currentMeshToDraw.verticesSize atIndex:0];
        else { printerror("Mesh without buffer or verticesRead."); }
    }
}
void commandencoder_setIU(id<MTLRenderCommandEncoder> encoder, InstanceUniforms const*const iu) {
    currentIUS_instanceCount = 1;
    [encoder setVertexBytes:iu length:sizeof(InstanceUniforms) atIndex:1];
}
void commandencoder_setIUs(id<MTLRenderCommandEncoder> encoder, IUsToDraw const iusToDraw) {
    currentIUS_instanceCount = iusToDraw.count;
    id<MTLBuffer> bufferOpt = (__bridge id<MTLBuffer>)(iusToDraw.metal_bufferOpt);
    if(bufferOpt) {
        [encoder setVertexBuffer:bufferOpt offset:0 atIndex:1];
    } else {
        if(iusToDraw.iusOpt) {
            [encoder setVertexBytes:iusToDraw.iusOpt length:iusToDraw.size atIndex:1];
        } else { printerror("IUSBuffer without buffer or ius."); }
    }
}
void commandencoder_setCurrentTexture(id<MTLRenderCommandEncoder> encoder, Texture *const newTex) {
    if(currentTex == newTex) { return; }
    currentTex = newTex;
    currentTexToDraw = texture_engine_touchAndGetToDraw(newTex);
    if(currentTexToDraw.isNearest != lastIsNearest) {
        lastIsNearest = currentTexToDraw.isNearest;
        [encoder setFragmentSamplerState:(currentTexToDraw.isNearest ? CoqGraph_metal_samplerNearest : CoqGraph_metal_samplerLinear)
                                 atIndex:0];
    }
    id<MTLTexture> mtlTexture = (__bridge id<MTLTexture>)currentTexToDraw.metal_texture_cptr;
    [encoder setFragmentTexture:mtlTexture atIndex:0];
}
void commandencoder_drawWithCurrents(id<MTLRenderCommandEncoder> encoder) {
    if(!currentIUS_instanceCount) return;
    if(currentMeshToDraw.metal_indicesBufersOpt_cptr) {
        [encoder drawIndexedPrimitives:currentMeshToDraw.primitive_type indexCount:currentMeshToDraw.indexCount
            indexType:MTLIndexTypeUInt16 indexBuffer:(__bridge id<MTLBuffer>)currentMeshToDraw.metal_indicesBufersOpt_cptr 
            indexBufferOffset:0 instanceCount:currentIUS_instanceCount];
    } else {
        [encoder drawPrimitives:currentMeshToDraw.primitive_type vertexStart:0 
            vertexCount:currentMeshToDraw.vertexCount instanceCount:currentIUS_instanceCount];
    }
}

//void commandencoder_defaultDraw(id<MTLRenderCommandEncoder> encoder, 
//                         Mesh *const mesh, Texture *const tex, const InstanceUniforms *const iu)
//{
//    // Vérifier la mesh
//    if(currentMesh != mesh) {
//        commandencoder_setCurrentMesh(encoder, mesh);
//    }
//    // Vérifier la texture
//    commandencoder_setCurrentTexture(encoder, tex);
//    // Dans une version custom, on pourrait passer d'autre info si on veut, par exemple les dimensions de la texture...
//    // [encoder setFragmentBytes:&tex->sizes length:sizeof(Vector2) atIndex:1];
//    
//    // Per instance uniforms
//    [encoder setVertexBytes:iu length:sizeof(InstanceUniforms) atIndex:1];
//    currentIUS_instanceCount = 1;
//    // Dessiner
//    commandencoder_justDrawWithCurrentMeshAndTexture(encoder);
//}
//void commandencoder_defaultMultiDraw(id<MTLRenderCommandEncoder> encoder, 
//                              Mesh *const mesh, Texture *const tex, const IUsBuffer *const iusBuffer)
//{
//    // 1. Mise a jour de la mesh ?
//    if(currentMesh != mesh) {
//        mesh_render_tryToUpdateVerticesAndIndiceCount(mesh);
//        commandencoder_setCurrentMesh(encoder, mesh);
//    }
//    // 2. Mise a jour de la texture ?
//    commandencoder_setCurrentTexture(encoder, tex);
//    // 3. Per instance uniforms
//    IUsBufferToDraw iusToDraw = iusbuffer_rendering_IUsToDraw(iusBuffer);
//    commandencoder_setIUSBuffer(encoder, &iusToDraw);
//    // 4. Dessiner
//    commandencoder_justDrawWithCurrentMeshAndTexture(encoder);
//}


//void commandencoder_testDraw(id<MTLRenderCommandEncoder> encoder, 
//                         Mesh *const mesh, Texture *const tex, const InstanceUniforms *const iu)
//{
//    if(!(mesh->_flags & mesh_flag_mutable)) { printerror("Not a mutable mesh."); return; }
//    // Mise a jour de la texture
//    commandencoder_setCurrentTexture(encoder, tex);
//    // Per instance uniforms
//    [encoder setVertexBytes:iu length:sizeof(InstanceUniforms) atIndex:1];
//    // Mise a jour de la mesh
//    currentMesh = mesh;
//    currentMesh_primitiveType = (MTLPrimitiveType)currentMesh->primitive_type;
//    currentMesh_vertexCount = currentMesh->vertexCount;
//    currentMesh_indicesBufferOpt = mesh_metal_indicesMTLBufferOpt(currentMesh);
//    currentMesh_indexCount = currentMesh->actualIndexCount;
//    if(currentMesh_cullMode != (MTLCullMode)currentMesh->cull_mode) {
//        currentMesh_cullMode = (MTLCullMode)currentMesh->cull_mode;
//        [encoder setCullMode:currentMesh_cullMode];
//    }
//    // Premier set des vertices
//    static Vertex mesh_vertices_[4] = {
//        {{-0.5, 0.5, 0, 0.0001, 0.0001, 0,0,1}},
//        {{-0.5,-0.5, 0, 0.0001, 0.9999, 0,0,1}},
//        {{ 0.5, 0.5, 0, 0.9999, 0.0001, 0,0,1}},
//        {{ 0.5,-0.5, 0, 0.9999, 0.9999, 0,0,1}},
//    };
//    size_t const verticesSize = mesh->vertexCount * mesh->_vertexSize;
//    static int frame = 0;
//    for(int i = 0; i < 4; i++) {
//    switch(frame) {
//        case 0: mesh_vertices_[2].pos.x = 0.5;
//                mesh_vertices_[2].pos.y = 1.5; break;
//        case 1: mesh_vertices_[2].pos.x = 1.5;
//                mesh_vertices_[2].pos.y = 1.5; break;
//        case 2: mesh_vertices_[2].pos.x = 0.5;
//                mesh_vertices_[2].pos.y = 0.5; break;
//        case 3: mesh_vertices_[2].pos.x = -0.25;
//                mesh_vertices_[2].pos.y = 0.5; break;
//        case 4: mesh_vertices_[2].pos.x = 1.25;
//                mesh_vertices_[2].pos.y = 0.25; break;
//    }
//    frame = (frame + 1) % 5;
////    id<MTLBuffer> bufferOpt = mesh_metal_verticesMTLBufferOpt(currentMesh);
////    if(bufferOpt) {
////        memcpy([bufferOpt contents], mesh_vertices_, verticesSize);
////        [encoder setVertexBuffer:bufferOpt offset:0 atIndex:0];
////    }
//    [encoder setVertexBytes:mesh_vertices_ length:verticesSize atIndex:0];
//    commandencoder_justDrawWithCurrentMeshAndTexture(encoder, 1);
//    }
//}
