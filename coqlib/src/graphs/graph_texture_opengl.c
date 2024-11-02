//
//  graph_texture_opengl_apple.h
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-03.
//
#include "graph__opengl.h"

#include "../maths/math_base.h"
#include "../utils/util_base.h"
#include "../utils/util_file.h"
#include "../coq_map.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"


#pragma mark - OpenGL specific -
void   Texture_opengl_init_(GLuint program) {
    if(program == 0) {
        printerror("No GL program.");
        return;
    }
}
void   texture_glBind(Texture *const tex) {
    tex->touchTime = CR_elapsedMS_;
    GLuint glTexId;
    if(tex->glTexId) {
        glTexId = tex->glTexId;
        goto bind_glTexture;
    }
    // Il y a des texture en demande pas encore "fully drawn"...
    Texture_needToFullyDraw_ = true;
    if(tex->glTexTmpId) {
        glTexId = tex->glTexTmpId;
        goto bind_glTexture;
    }
    printwarning("Texture %s has not been init.", tex->string);
    return;
bind_glTexture:
    glBindTexture(GL_TEXTURE_2D, glTexId);
//    glUniform2f(_Texture_tex_wh_id, tex->ptu.width, tex->ptu.height);
//    glUniform2f(_Texture_tex_mn_id, (float)tex->m,     (float)tex->n);
}

#pragma mark - Methode engine dependant... (Metal ou OpenGL)
// Dépend de l'engine (Metal / OpenGL)...
/// Copier un array de pixels dans la texture.
void           texture_engine_writeAllPixels(Texture* tex, const void* pixelsBGRA8) {
    glBindTexture(GL_TEXTURE_2D, tex->glTexId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int)tex->sizes.w, (int)tex->sizes.h, GL_BGRA, GL_UNSIGNED_BYTE, pixelsBGRA8);
}
/// Copier un array de pixels dans la texture aux coordonées de `region`.
void           texture_engine_writePixelsToRegion(Texture* tex, const void* pixelsBGRA8, RectangleUint region) {
    glBindTexture(GL_TEXTURE_2D, tex->glTexId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, region.o_x, region.o_y, region.w, region.h, GL_BGRA, GL_UNSIGNED_BYTE, pixelsBGRA8);
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

void           texture_engine_tryToLoadAsPng_(Texture* tex, bool isMini) {   
    // Chargement des pixels...
    const char* pngPath = FileManager_getPngPathOpt(tex->string, tex->flags & tex_flag_png_coqlib, isMini);
    PixelBGRAArray* pa = Pixels_engine_createOptFromPng(pngPath);
    if(!pa) {
        if(!isMini) printerror("No png for %s.", tex->string);
        return; 
    }
    //    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    GLint filter = (tex->flags & tex_flag_nearest) ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLsizei width = (GLsizei)pa->width, height = (GLsizei)pa->height;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pa->pixels);
    coq_free(pa);
    if(!textureId) { printerror("Failed to init OpenGL texture %s.", tex->string); return; }
    
    // Ok, remplacer (s'il y a lieu) la texture précédente et mettre la nouvelle.
    if(isMini) {
        if(tex->glTexTmpId) { glDeleteTextures(1, &tex->glTexTmpId); }
        tex->glTexTmpId = textureId; 
    } else {
        if(tex->glTexId) {    glDeleteTextures(1, &tex->glTexId);    }
        tex->glTexId = textureId;
    }
    tex->flags |= isMini ? tex_flag_tmpDrawn_ : tex_flag_fullyDrawn_;
    // Mise à jour des dimensions
    tex->sizes.w =  (float)width;
    tex->sizes.h = (float)height;
}
// Editions des pixels d'une texture...
/// Crée une texture OpenGL/Metal vide (quand même considéré comme `fullyDrawn`.
void           texture_engine_loadEmptyWithSize_(Texture* tex, size_t width, size_t height) {
    if(tex->glTexId) {
        printwarning("Texture already set.");
        glDeleteTextures(1, &tex->glTexId);    tex->glTexId = 0;
    }
    
    glGenTextures(1, &tex->glTexId);
    glBindTexture(GL_TEXTURE_2D, tex->glTexId);
    
    GLint filter = (tex->flags & tex_flag_nearest) ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    tex->flags |= tex_flag_fullyDrawn_;
    // Mise à jour des dimensions
    tex->width = (float)width;
    tex->height = (float)height;
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
