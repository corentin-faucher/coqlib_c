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


TextureToDraw  texture_engine_touchAndGetToDraw(Texture *const tex) {
    tex->touchTime = ChronosRender.render_elapsedMS;
    // Cas "OK"
    if(tex->glTexId) {
        return (TextureToDraw) {
            .glTextureId = tex->glTexId,
            .isNearest = tex->flags & tex_flag_nearest,
        };
    }
    // Il y a des texture en demande pas encore "fully drawn"...
    Texture_needToFullyDraw_ = true;
    if(tex->glTexTmpId) {
        return (TextureToDraw) {
            .glTextureId = tex->glTexTmpId,
            .isNearest = tex->flags & tex_flag_nearest,
        };
    }
   printwarning("Texture %s has not been init.", tex->string);
    return (TextureToDraw) {
        .glTextureId = 0,
        .isNearest = true,
    };
}


/// MARK: - Methode engine dependant... (Metal ou OpenGL)
// Dépend de l'engine (Metal / OpenGL)...
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

    tex->flags |= asMini ? tex_flag_tmpDrawn_ : tex_flag_fullyDrawn_;
    // Mise à jour des dimensions
    tex->dims.width =  (float)width;
    tex->dims.height = (float)height;
    tex->dims.tileRatio =  (float)width / (float)height * (float)tex->dims.n / (float)tex->dims.m;
}
void           texture_engine_tryToLoadAsPng_(Texture* tex, bool isMini) {
    // Chargement des pixels...
    const char* pngPath = FileManager_getPngPathOpt(tex->string, tex->flags & tex_flag_png_coqlib, isMini);
    guard_let(PixelBGRAArray const*, pa, Pixels_engine_createFromPngOpt(pngPath, !isMini),,)
    texture_engine_load_(tex, pa->width, pa->height, isMini, pa->pixels);
    coq_free((void*)pa);
}

/// Copier un array de pixels dans la texture.
void           texture_engine_writeAllPixels(Texture* tex, const void* pixelsBGRA8) {
    glBindTexture(GL_TEXTURE_2D, tex->glTexId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int)tex->dims.width, (int)tex->dims.height,
       GL_BGRA, GL_UNSIGNED_BYTE, pixelsBGRA8);
}
/// Copier un array de pixels dans la texture aux coordonées de `region`.
void           texture_engine_writePixelsToRegion(Texture* tex, const void* pixelsBGRA8, RectangleUint region) {
    glBindTexture(GL_TEXTURE_2D, tex->glTexId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, region.o_x, region.o_y, region.w, region.h,
        GL_BGRA, GL_UNSIGNED_BYTE, pixelsBGRA8);
}
/// Copier une partie (region) de la texture vers un array de pixels (l'array de pixels doit être de taille suffisante)
void           texture_engine_writeRegionToPixels(const Texture* tex, void* pixelBGRA8, RectangleUint region) {
    printwarning("Non implémenté pour OpenGL.");
    // glGetTextureSubImage...
}
/// Copier une région d'une texture vers une autre texture.
void           texture_engine_copyRegionTo(const Texture* tex, Texture* texDest,
                                           RectangleUint srcRect, UintPair destOrig) {
    printwarning("Non implémenté pour OpenGL.");
    // glGetTextureSubImage...
}

void  texture_engine_releaseAll_(Texture* tex) {
    if(tex->glTexId) {    glDeleteTextures(1, &tex->glTexId);    tex->glTexId = 0;}
    if(tex->glTexTmpId) { glDeleteTextures(1, &tex->glTexTmpId); tex->glTexTmpId = 0; }
    tex->flags &= ~ tex_flags_drawn_;
}
void  texture_engine_releasePartly_(Texture* tex) {
    if(tex->glTexId) { glDeleteTextures(1, &tex->glTexId); tex->glTexId = 0; }
    tex->flags &= ~ tex_flag_fullyDrawn_;
}

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
