//
//  graph_texture_opengl_apple.h
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-03.
//
#include "graph__opengl.h"
#include "graph_texture_private.h"
#include "../maths/math_base.h"
#include "../utils/util_base.h"

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

// MARK: - Dessin (renderer)
GLuint GLTexture_createFromPixelArray(PixelArray*const pa, bool const nearest) {
    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    GLint filter = nearest ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)pa->width, (GLsizei)pa->height, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pa->pixels);
    return texId;
}

void texture_render_checkTexture(Texture*const tex) {
    // Cas ordinaire, déjà init, voir s'il faut updater ?
    if(tex->glTex0Id) {
        if(!(tex->flags & tex_flag_mutable) || 
            (tex->version_rend == tex->version_pixels)) return;
        goto update_tex;
    }
    if(tex->glTex0Id) return;
    // Cas on a les pixels, mais buffer pas init.
    if(tex->pixelsOpt && !tex->glTex0Id) {
        tex->glTex0Id = GLTexture_createFromPixelArray(tex->pixelsOpt, 
                                            tex->flags & tex_flag_nearest);
        // Pour les pngs, on peut libérer les pixels. 
        // (on n'a qu'à recharger l'image pour réavoir les pixels)
        if((tex->flags & tex_flag__png) && !(tex->flags & tex_flag_keepPixels)) {
            coq_free(tex->pixelsOpt);
            tex->pixelsOpt = NULL;
        }
        // Pour un mutable double buffer
        if(tex->flags & tex_flag_doubleBuffer) {
            tex->glTex1Id = GLTexture_createFromPixelArray(tex->pixelsOpt, 
                                        tex->flags & tex_flag_nearest);
        }
        tex->rendFlags |= tex_flag_rend_bufferSet;
        return;
    }
    
    // Des textures ne sont pas encore toute dessinées.
    Texture_setNeedToFullyDrawPng_();
    
    // Ok, on a la mini au moins.
    if(tex->glTexMiniId) return;
    // Sinon setter la mini.
    if(tex->miniPixelsOpt) {
        tex->glTexMiniId = GLTexture_createFromPixelArray(tex->miniPixelsOpt,
                                              tex->flags & tex_flag_nearest);
        tex->rendFlags |= tex_flag_rend_bufferMiniSet;
        return;
    }
    
    // Cas même pas la mini...
    Texture_setNeedToBareDrawPng_();
    return;

update_tex:
    tex->version_rend = tex->version_pixels;
    if(!tex->pixelsOpt || !tex->glTex0Id) {
        printerror("Missing pixels or texture buffer.");
        return;
    }
    GLuint texId;
    // Ecrire dans le buffer "1" s'il n'est pas en lecture et qu'on est en double buffer.
    if(tex->glTex1Id && !(tex->rendFlags & tex_flag_rend_readBuff1)) {
        texId = tex->glTex1Id;
    } else {
        texId = tex->glTex0Id;
    }
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
        (GLsizei)tex->pixelsOpt->width, (GLsizei)tex->pixelsOpt->height, 
        0, GL_RGBA, GL_UNSIGNED_BYTE, tex->pixelsOpt->pixels);
    // Si double buffer, swaper lecture/écriture.
    if(tex->glTex1Id) tex->rendFlags ^= tex_flag_rend_readBuff1;
}

TextureToDraw texture_render_getToDraw(Texture*const tex) {
    tex->touchTime = RendererTimeCapture.render_elapsedMS;
    // Cas par défaut.
    if(tex->glTex0Id) {
        if(tex->glTex1Id && (tex->rendFlags & tex_flag_rend_readBuff1)) {
            return (TextureToDraw) {
                .glTextureId = tex->glTex1Id,
                .isNearest = tex->flags & tex_flag_nearest,
            };
        }
        return (TextureToDraw) {
            .glTextureId = tex->glTex0Id,
            .isNearest =   tex->flags & tex_flag_nearest,
        };
    }
    // Cas au moins la mini.
    if(tex->glTexMiniId) {
        return (TextureToDraw) {
            .glTextureId = tex->glTexMiniId,
            .isNearest = tex->flags & tex_flag_nearest,
        };
    }
    // Cas même pas la mini...
    return (TextureToDraw) {
        .glTextureId = 0,
        .isNearest = true,
    };
}

// MARK: - Private stuff...
void  texture_render_releaseBuffers_(void*const tex_void) {
    Texture*const tex = tex_void;
    if(tex->glTex0Id) {    glDeleteTextures(1, &tex->glTex0Id);    tex->glTex0Id = 0;}
    if(tex->glTex1Id) {    glDeleteTextures(1, &tex->glTex1Id);    tex->glTex1Id = 0;}
    if(tex->glTexMiniId) { glDeleteTextures(1, &tex->glTexMiniId); tex->glTexMiniId = 0; }
    tex->rendFlags &= ~tex_flags_rend_buffersSet;
}

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

// Garbage
/*
// MARK: - Optenir les pixels
PixelBGRAArray* PixelBGRAArray_engine_createFromTextureOpt(Texture* tex) {
    if(!tex->glTex0Id) { printerror("Texture not drawn."); return NULL; }
    GLuint glTexId = texture_readableGLTextureId_(tex);
    PixelBGRAArray*const pa = PixelBGRAArray_createEmpty(tex->dims.width, tex->dims.height);
    glBindTexture(GL_TEXTURE_2D, glTexId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, pa->pixels);
    return pa;
}
PixelBGRAArray* PixelBGRAArray_engine_createFromTextureSubRegionOpt(Texture*const tex, RectangleUint const region)
{
    if(!tex->glTex0Id) { printerror("Texture not drawn."); return NULL; }
    size_t const tex_width = tex->dims.width;
    size_t const tex_height = tex->dims.height;
    if(!region.w || !region.h) {
        printwarning("Empty region."); 
        return NULL;
    }
    if(region.o_x + region.w > tex_width || region.o_y + region.h > tex_height) {
        printerror("Region overflow.");
        return NULL;
    }
    PixelBGRAArray* pa = NULL;
    with_beg(PixelBGRAArray, paFull, PixelBGRAArray_createEmpty(tex_width, tex_height))
    // Sans OpenGL 4.5, on doit copier toute la texture.
    // TODO: Avec OpenGL 4.5, on pourrait utiliser `glGetTexSubImage`...
    GLuint glTexId = texture_readableGLTextureId_(tex);
    glBindTexture(GL_TEXTURE_2D, glTexId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, paFull->pixels);
    pa = PixelBGRAArray_createSubRegion(paFull, region);
    with_end(paFull)
    return pa;
}
// MARK: - Edition
void texture_engine_writePixelsAt(Texture*const tex, PixelBGRAArray const*const pa, UintPair const origin) 
{
    
    if(!tex->glTex0Id || !pa) { printerror("Texture not drawn or no pixels."); return; }
    if((tex->flags & tex_flag_doubleBuffer) && (tex->flags & tex_flag__dbEdited)) {
        printwarning("Buffer already edited."); return; 
    }
    size_t const tex_width = tex->dims.width;
    size_t const tex_height = tex->dims.height;
    if(origin.uint0 + pa->width > tex_width || origin.uint1 + pa->height > tex_height) {
        printerror("Region overflow.");
        return;
    }
    
    GLuint glTexId = texture_editableGLTextureId_(tex);
    glBindTexture(GL_TEXTURE_2D, glTexId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, origin.uint0, origin.uint1, (int)pa->width, (int)pa->height,
                    GL_BGRA, GL_UNSIGNED_BYTE, pa->pixels);
    
    if(tex->flags & tex_flag_doubleBuffer)
        tex->flags |= tex_flag__dbEdited;
}
/// Copier une région d'une texture vers une autre texture.
/// Équivalent de `texture_engine_createPixelsSubRegion`, `texture_engine_writePixelsToRegion`.
void texture_engine_copyRegionTo(const Texture*const texSrc, RectangleUint const srcRect,
                                 Texture*const texDst, UintPair const destOrig) 
{
    if(!texSrc->glTex0Id || !texDst->glTex0Id) { printerror("Texture not drawn."); return; }
    if((srcRect.o_x + srcRect.w > (uint32_t)texSrc->dims.width) ||
       (srcRect.o_y + srcRect.h > (uint32_t)texSrc->dims.height) ||
       (destOrig.uint0 + srcRect.w > (uint32_t)texDst->dims.width) ||
       (destOrig.uint1 + srcRect.h > (uint32_t)texDst->dims.height)) {
        printerror("Overflow"); return;
    }
    if((texDst->flags & tex_flag_doubleBuffer) && (texDst->flags & tex_flag__dbEdited)) {
        printwarning("Already edited."); return; 
    }
    
    with_beg(PixelBGRAArray, paSrcFull, PixelBGRAArray_createEmpty(texSrc->dims.width, texSrc->dims.height))
    // Sans OpenGL 4.5, on doit copier toute la texture.
    // TODO: Avec OpenGL 4.5, on pourrait utiliser `glGetTexSubImage`...
    GLuint glTexIdSrc = texture_readableGLTextureId_(texSrc);
    glBindTexture(GL_TEXTURE_2D, glTexIdSrc);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, paSrcFull->pixels);
    with_beg(PixelBGRAArray, paSrcRegion, PixelBGRAArray_createSubRegion(paSrcFull, srcRect))
    GLuint glTexIdDst = texture_editableGLTextureId_(texDst);
    glBindTexture(GL_TEXTURE_2D, glTexIdDst);
    glTexSubImage2D(GL_TEXTURE_2D, 0, destOrig.uint0, destOrig.uint1, 
                    (int)paSrcRegion->width, (int)paSrcRegion->height,
                    GL_BGRA, GL_UNSIGNED_BYTE, paSrcRegion->pixels);
    with_end(paSrcRegion)
    with_end(paSrcFull)
    
    if(texDst->flags & tex_flag_doubleBuffer)
        texDst->flags |= tex_flag__dbEdited;
}
*/

