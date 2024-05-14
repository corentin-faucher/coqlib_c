//
//  graph_texture_opengl.m
//  Version OpenGL pour les texture.
//  On utilise lodepng de Lode Vandevenne pour charger les pngs.
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "../coq_map.h"
#include "../graphs/graph__opengl.h"
#include "../utils/util_base.h"
#include "../utils/util_file.h"
#include "../utils/util_language.h"


#include <SDL_image.h> // <SDL2_image/SDL_image.h> 
#include <SDL_ttf.h> // <SDL2_ttf/SDL_ttf.h> // 



typedef TTF_Font Font;
static Font*                Font_current_ = NULL;
static Font*                Font_currentMini_ = NULL;
static double               Font_currentSize_ = 48;
static double               Font_miniSize_ =    12;
// OpenGL attribute locations id
//static GLint _Texture_tex_wh_id = 0;
//static GLint _Texture_tex_mn_id = 0;
//static GLint Texture_tex_ptuId_ = 0;
//static GLuint Texture_ptuBufferId_ = 0;
//static GLuint Texture_bindingPoint_ = 0;

#pragma mark - Drawing de strings ------------------

static const uint32_t pixels_default_[] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
Vector2  glTexture_drawAndGetSize_(GLuint* glTexRef, const char* string, 
            Font* font, Vector4 color, bool neareast)
{
    if(*glTexRef != 0) { glDeleteTextures(1, glTexRef); *glTexRef = 0; }
    // Texture OpenGL
    glGenTextures(1, glTexRef);
    glBindTexture(GL_TEXTURE_2D, *glTexRef);
    GLint filter = neareast ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    SDL_Color color_sdl = {
        255*color.r,
        255*color.g,
        255*color.b
      };
    SDL_Surface* sdl_surf = TTF_RenderUTF8_Blended(font, string, color_sdl);
    const unsigned char* pixels;
    unsigned int width, height;
    if(sdl_surf) {
      SDL_LockSurface(sdl_surf);
      width =  sdl_surf->w;
      height = sdl_surf->h;
      pixels = sdl_surf->pixels;
    } else {
      printerror("Cannot create sdl surface for string %s.", string);
      width = 2; height = 2;
      pixels = (unsigned char*)pixels_default_;
    }
    // Pourquoi il faut setter ca ??
    int row_length = sdl_surf->pitch / sdl_surf->format->BytesPerPixel;
    glPixelStorei(GL_UNPACK_ROW_LENGTH, row_length);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    if(sdl_surf) {
      SDL_UnlockSurface(sdl_surf);
      SDL_FreeSurface(sdl_surf);
    }
    return (Vector2) {{ width, height }};
}

#pragma mark - Methods texture ----------------------

void  texture_engine_releaseAll_(Texture* tex) {
    if(tex->glTexId) {    glDeleteTextures(1, &tex->glTexId);    tex->glTexId = 0;}
    if(tex->glTexTmpId) { glDeleteTextures(1, &tex->glTexTmpId); tex->glTexTmpId = 0; }
    tex->flags &= ~ tex_flags_drawn_;
}
void  texture_engine_releasePartly_(Texture* tex) {
    if(tex->glTexId) { glDeleteTextures(1, &tex->glTexId); tex->glTexId = 0; }
    tex->flags &= ~ tex_flag_fullyDrawn_;
}
// Transfer la mtlTex -> mtlTexTmp (pour redessiner la mtlTex)
void  texture_engine_setStringAsToRedraw_(Texture* tex) {
    if(tex->glTexTmpId) { glDeleteTextures(1, &tex->glTexTmpId); tex->glTexTmpId = 0; }
    tex->glTexTmpId = tex->glTexId;
    if(tex->glTexTmpId) {
        tex->flags |= tex_flag_tmpDrawn_;
    } else {
        tex->flags &= ~tex_flag_tmpDrawn_;
    }
    tex->glTexId = 0;
    tex->flags &= ~tex_flag_fullyDrawn_;
}
void  texture_engine_tryToLoadAsPng_(Texture* tex, bool const isMini) {
    GLuint* glTexIdRef = isMini ? &tex->glTexTmpId : &tex->glTexId;
    if(*glTexIdRef != 0) { glDeleteTextures(1, glTexIdRef); *glTexIdRef = 0; }
    const char* png_dir;
    if(tex->flags & tex_flag_png_coqlib) {
        png_dir = isMini ? "pngs_coqlib/minis" : "pngs_coqlib";
    } else {
        png_dir = isMini ? "pngs/minis" : "pngs";
    }
    if(!tex->string) { printerror("Texture without name."); return; }
    const char* png_path = FileManager_getResourcePathOpt(tex->string, "png", png_dir);

    // Chargement avec SDL2 et SDL2_Image
    {
    SDL_Surface* sdl_surf = IMG_Load(png_path);
    if(!sdl_surf) { if(!isMini) {printerror("Cannot load png %s.", tex->string); } return; }

    // Ou bien avec LodePNG de Lode Vandevenne... ?
    // unsigned char* pixels;
    // unsigned int width, height;
    // lodepng_decode32_file(&pixels, &width, &height, png_path);
    // if(!pixels) { if(!asMini) {printerror("Cannot load png %s.", tex->string); } return; }
    // free(pixels);

    // Texture OpenGL
    glGenTextures(1, glTexIdRef);
    glBindTexture(GL_TEXTURE_2D, *glTexIdRef);
    GLint filter = (tex->flags & tex_flag_nearest) ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    SDL_LockSurface(sdl_surf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sdl_surf->w, sdl_surf->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, sdl_surf->pixels);
    SDL_UnlockSurface(sdl_surf);

    // Mettre à jour les info de la texture.
        tex->_width =  sdl_surf->w;
        tex->_height = sdl_surf->h;
        SDL_FreeSurface(sdl_surf);
    }
    tex->ratio = (float)tex->_width / (float)tex->_height * (float)tex->n / (float)tex->m;
    tex->flags |= isMini ? tex_flag_tmpDrawn_ : tex_flag_fullyDrawn_;
}
void           texture_engine_loadWithPixels_(Texture* tex, const void* pixelsBGRA8, uint32_t width, uint32_t height) {
    glGenTextures(1, &tex->glTexId);
    glBindTexture(GL_TEXTURE_2D, tex->glTexId);
    GLint filter = (tex->flags & tex_flag_nearest) ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                 0, GL_BGRA, GL_UNSIGNED_BYTE, pixelsBGRA8);
                 
    tex->flags |= tex_flag_fullyDrawn_;
    // Mise à jour des dimensions
    tex->_width = width;
    tex->_height = height;
    tex->ratio = (float)width / (float)height 
                 * (float)tex->n / (float)tex->m;
}
void           texture_engine_updatePixels(Texture* tex, const void* pixelsBGRA8) {
    glBindTexture(GL_TEXTURE_2D, tex->glTexId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->_width, tex->_height, GL_BGRA, GL_UNSIGNED_BYTE, pixelsBGRA8);
}
void  texture_engine_tryToLoadAsString_(Texture* tex, bool isMini) {
    if(!(tex->flags & tex_flag_string)) {
         printerror("Not a string."); return;
    }
    GLuint* glTexIdRef = isMini ? &tex->glTexTmpId : &tex->glTexId;
    if(*glTexIdRef != 0) { glDeleteTextures(1, glTexIdRef); *glTexIdRef = 0; }
    Vector2 sizes;
    if(tex->flags & tex_flag_string_localized) {
        char* localized = String_createLocalized(tex->string);
        sizes = glTexture_drawAndGetSize_(glTexIdRef, localized, Font_current_, 
                                tex->string_color, tex->flags & tex_flag_nearest);
        coq_free(localized);
    } else {
        sizes = glTexture_drawAndGetSize_(glTexIdRef, tex->string, Font_current_, 
                                tex->string_color, tex->flags & tex_flag_nearest);
    }
    tex->flags |= isMini ? tex_flag_tmpDrawn_ : tex_flag_fullyDrawn_;
    // Mise à jour des dimensions
    tex->_width =  sizes.w;
    tex->_height = sizes.h;
    tex->ratio = sizes.w / sizes.h * (float)tex->n / (float)tex->m;
    tex->alpha = 1.f;
    tex->beta = 1.f;
    // Release tmp.
    if(!isMini) {
        if(tex->glTexTmpId) { glDeleteTextures(1, &tex->glTexTmpId); tex->glTexTmpId = 0; }
        tex->flags &= ~ tex_flag_tmpDrawn_;
    }
}
void  texture_engine_justSetSizeAsString_(Texture* tex) {
    texture_engine_tryToLoadAsString_(tex, true);
}
void  texture_glBind(Texture* tex) {
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

#pragma mark - Fonts -----------------------------

static char  _Font_dir[PATH_MAX-64];
static char  _Font_path[PATH_MAX];
void     Texture_resetCurrentFont_(bool andMini) {
    Font* newFont = TTF_OpenFont(_Font_path, Font_currentSize_);
    if(newFont == NULL) {
        printerror("Font in %s not found.", _Font_path);
        return;
    }
    if(Font_current_) {
        TTF_CloseFont(Font_current_);
        Font_current_ = NULL;
    }
    if(Font_currentMini_ && andMini) {
        TTF_CloseFont(Font_currentMini_);
        Font_currentMini_ = NULL;
    }
    Font_current_ = newFont;
    if(andMini)
        Font_currentMini_ = TTF_OpenFont(_Font_path, Font_miniSize_);
}
void     Texture_setCurrentFont(const char* fontName) {
    if(fontName == NULL) {
        printerror("No fontName.");
        return;
    }
    // Mettre a jour la path du font
    sprintf(_Font_path, "%s/%s.ttf",
            _Font_dir, fontName);

    Texture_resetCurrentFont_(true);
}
void     Texture_setCurrentFontSize(double newSize) {
    Font_currentSize_ = newSize;
    if(Font_current_ == NULL) {
        return;
    }
    Texture_resetCurrentFont_(false);
}
double   Texture_currentFontSize(void) {
    return Font_currentSize_;
}

/*-- Init, constructors... ----------------------------------------------------------*/

void   Texture_GLinit(GLuint program, const char* font_dir, const char* default_font_name) {
    strcpy(_Font_dir, font_dir);
    Texture_setCurrentFont(default_font_name);
    
    if(program == 0) {
        printerror("No GL program.");
        return;
    }
//    _Texture_tex_wh_id = glGetUniformLocation(program, "tex_wh");
//    _Texture_tex_mn_id = glGetUniformLocation(program, "tex_mn");
}

