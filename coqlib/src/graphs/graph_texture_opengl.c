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

//static int Texture_dummy__ = 0;

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif


void           texture_engine_load_(Texture*const tex, size_t const width, size_t const height,
    bool const asMini, PixelBGRA const*const pixelsOpt)
{
    uint32_t* glTexIdRef = asMini ? &tex->glTexTmpId : &tex->glTexId;
    if(*glTexIdRef) {
        printwarning("Texture already set.");
        glDeleteTextures(1, glTexIdRef); *glTexIdRef = 0;
    }
    glGenTextures(1, glTexIdRef);
    glBindTexture(GL_TEXTURE_2D, *glTexIdRef);
    GLint filter = (tex->flags & tex_flag_nearest) ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsOpt);

    tex->flags |= asMini ? tex_flag__tmpDrawn : tex_flag__fullyDrawn;
    // Mise à jour des dimensions
    tex->dims.width =  (float)width;
    tex->dims.height = (float)height;
    tex->dims.tileRatio =  (float)width / (float)height * (float)tex->dims.n / (float)tex->dims.m;
}
// La texture en "lecture" ou "live" pour lire les pixels officiellement "à jour".
static inline GLuint texture_readableGLTextureId_(Texture const*const tex) {
    if(!(tex->flags & tex_flag_doubleBuffer))
        return tex->glTexId;
    if(!tex->glTex2Id) { printerror("Missing mtlTex2."); 
        return tex->glTexId;
    }
    if(tex->flags & tex_flag__dbSecondLive)
        return tex->glTex2Id;
    return tex->glTexId;
}
// La texture en "écriture" (pas live) pour éditer les pixels.
static inline GLuint texture_editableGLTextureId_(Texture*const tex) {
    if(!(tex->flags & tex_flag_doubleBuffer))
        return tex->glTexId;
    if(!tex->glTex2Id) { printerror("Missing mtlTex2."); 
        return tex->glTexId;
    }
    if(tex->flags & tex_flag__dbSecondLive)
        return tex->glTexId;
    return tex->glTex2Id;
}

// MARK: - Optenir les pixels
PixelBGRAArray* PixelBGRAArray_engine_createFromTextureOpt(Texture* tex) {
    if(!tex->glTexId) { printerror("Texture not drawn."); return NULL; }
    GLuint glTexId = texture_readableGLTextureId_(tex);
    PixelBGRAArray*const pa = PixelBGRAArray_createEmpty(tex->dims.width, tex->dims.height);
    glBindTexture(GL_TEXTURE_2D, glTexId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, pa->pixels);
    return pa;
}
PixelBGRAArray* PixelBGRAArray_engine_createFromTextureSubRegionOpt(Texture*const tex, RectangleUint const region)
{
    if(!tex->glTexId) { printerror("Texture not drawn."); return NULL; }
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
    
    if(!tex->glTexId || !pa) { printerror("Texture not drawn or no pixels."); return; }
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
    if(!texSrc->glTexId || !texDst->glTexId) { printerror("Texture not drawn."); return; }
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
                             
// MARK: - Dessin (renderer)
/// Info pour dessiner par le renderer (et empêche de libérer)
TextureToDraw texture_render_getTextureToDraw(Texture*const tex) {
    if(tex->glTexId) {
        if(tex->glTex2Id && (tex->flags & tex_flag__dbSecondLive))
            return (TextureToDraw) {
                .glTextureId = tex->glTex2Id,
                .isNearest = tex->flags & tex_flag_nearest,
            };
        return (TextureToDraw) {
            .glTextureId = tex->glTexId,
            .isNearest = tex->flags & tex_flag_nearest,
        };
    }
    if(tex->glTexTmpId) {
        return (TextureToDraw) {
            .glTextureId = tex->glTexTmpId,
            .isNearest = tex->flags & tex_flag_nearest,
        };
    }
//    #warning Utile ?
    return (TextureToDraw) {
        .glTextureId = 0,
        .isNearest = true,
    };
}

// MARK: - Private stuff...
void  texture_engine_tryToLoadAsPng_(Texture* tex, bool isMini) {
    // Chargement des pixels...
    const char* pngPath = FileManager_getPngPathOpt(tex->string, 
                tex->flags & tex_flag__png_coqlib, isMini);
    if(!pngPath) {
        if(!isMini) printerror("No path for png %s.", tex->string);
        return;
    }
    guard_let(PixelBGRAArray const*, pa, 
              PixelBGRAArray_engine_createFromPngFileOpt(pngPath, !isMini),,)
    texture_engine_load_(tex, pa->width, pa->height, isMini, pa->pixels);
    coq_free((void*)pa);
}
void  texture_engine_releaseAll_(Texture* tex) {
    if(tex->glTexId) {    glDeleteTextures(1, &tex->glTexId);    tex->glTexId = 0;}
    if(tex->glTexTmpId) { glDeleteTextures(1, &tex->glTexTmpId); tex->glTexTmpId = 0; }
    tex->flags &= ~ tex_flags__drawn;
}
void  texture_engine_releasePartly_(Texture* tex) {
    if(tex->glTexId) { glDeleteTextures(1, &tex->glTexId); tex->glTexId = 0; }
    tex->flags &= ~ tex_flag__fullyDrawn;
}

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
