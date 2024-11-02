//
//  graph_base_sdl.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-31.
//
#include "graph_base.h"
#include "../utils/util_base.h"
#include "../utils/util_language.h"
#include <string.h>
#if __APPLE__
#include <sys/syslimits.h>
#endif
#ifdef __linux__
#include <limits.h>
#endif

#pragma mark - Dessin de png, char, string
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
// #include <SDL_image.h> // <SDL2_image/SDL_image.h>
// #include <SDL_ttf.h>
static uint32_t test_pixels_[4] = {
        0x00FFFFFF, 0xFFFFFFFF, 0x1F00FFFF, 0xFFFF00FF,
    };
PixelBGRAArray* Pixels_engine_createOptFromPng(const char* pngPath) {
    // Chargement avec SDL2 et SDL2_Image
    SDL_Surface* sdl_surf = IMG_Load(pngPath);
    if(!sdl_surf) { return NULL; }
    SDL_LockSurface(sdl_surf);
    size_t pixelCount = sdl_surf->w * sdl_surf->h;
    PixelBGRAArray *pa = coq_callocArray(PixelBGRAArray, PixelBGRA, pixelCount);
    pa->width = sdl_surf->w; pa->height = sdl_surf->h;
    pa->solidWidth = sdl_surf->w;
    memcpy(pa->pixels, sdl_surf->pixels, pixelCount * sizeof(PixelBGRA));
    SDL_UnlockSurface(sdl_surf);
    SDL_FreeSurface(sdl_surf);
    return pa;
    // Ou bien avec LodePNG de Lode Vandevenne... ?
    // unsigned char* pixels;
    // unsigned int width, height;
    // lodepng_decode32_file(&pixels, &width, &height, png_path);
    // if(!pixels) { if(!asMini) {printerror("Cannot load png %s.", tex->string); } return; }
    // free(pixels);
//    return NULL;
}
#pragma mark - Dessin de fonts/glyphs dans buffer de pixels -
static char  Font_dir_[PATH_MAX-64] = {};
static char  Font_tmpFontPath_[PATH_MAX] = {};
static TTF_Font* Font_defaultFont_ = NULL;
const char* CoqFont_getFontPath_(const char* fontName) {
    memset(Font_tmpFontPath_, 0, PATH_MAX);
    sprintf(Font_tmpFontPath_, "%s/%s.ttf", Font_dir_, fontName);
    return Font_tmpFontPath_;
}
void     CoqFont_sdlttf_setFontsPath(const char* fontsPath, const char* defaultFontName) {
    if(!fontsPath || !defaultFontName) {
        printerror("No fonts path or default font."); return;
    }
    if(Font_defaultFont_) {
        printwarning("Fonts path already set."); return;
    }
    strcpy(Font_dir_, fontsPath);
    Font_defaultFont_ = TTF_OpenFont(CoqFont_getFontPath_(defaultFontName), 20);
    if(!Font_defaultFont_) printerror("Failed to load default font %s.", defaultFontName);
}
void  CoqFont_sdlttf_quit(void) {
    if(Font_defaultFont_) TTF_CloseFont(Font_defaultFont_);
    Font_defaultFont_ = NULL;
    memset(Font_dir_, 0, PATH_MAX-64);
}

void  coqfont_engine_init(coq_Font const* cf, const char *fontNameOpt, double fontSize, bool nearest) {
    // Checks...
    if(!Font_dir_[0]) { printerror("Fonts path not init."); return ;}
    if(cf->_ttfFont) { printwarning("Font already init."); return; }
    if(!fontSize) fontSize = 32; // (Default)
    if(fontSize < 12) { printwarning("Font too small."); fontSize = 12; }
    if(fontSize > 200) { printwarning("Font too big.");  fontSize = 200; }
    // Obtenir une font
    TTF_Font *font = NULL;
    if(fontNameOpt) {
        font = TTF_OpenFont(CoqFont_getFontPath_(fontNameOpt), fontSize);
        if(!font) printerror("Failed to load font %s.", fontNameOpt);
    }
    if(!font) {
        font = Font_defaultFont_;
    }
    if(!font) { printerror("No font available?"); return; }

    // Hauteur de Ref (capHeight hauteur max standard de la plupart des lettres)
    // ? pas de cap Height pour ttf ??
    float const solidHeight = TTF_FontHeight(font);
    // Dimensions
    LanguageFontInfo const *info = LanguageFont_getFontInfoOpt(fontNameOpt);
    double const extraDeltaY = -solidHeight * 0.03 + (info ? info->deltaYAdj*solidHeight : 0.f);
    double const extraDesc = info ? info->extraDesc*solidHeight : 0.f;
    double const descender = -TTF_FontDescent(font) + extraDesc; // (-> rendu positif...)
    double const ascender = TTF_FontAscent(font);
    size_t const extra_margin = nearest ? 1 : 2;
    // (Ascender vrai hauteur max avec fioriture qui dépassent, e.g. le haut de `f`.)
    size_t const glyphHeight = ceil(ascender + descender) + 2*extra_margin;
    // Décalage en y pour centrer les caractères
    double const deltaY = 0.5*( -extraDesc) + extraDeltaY;

    // Save data
    *(const void**)&cf->_ttfFont = font;
    *(bool*)&cf->nearest = nearest;
    size_initConst(&cf->glyphHeight, glyphHeight);
    float_initConst(&cf->solidHeight, solidHeight);
    float_initConst(&cf->deltaY, deltaY);
    float_initConst(&cf->drawDeltaY, extraDesc);
}
void  coqfont_engine_deinit(coq_Font const* cf) {
    TTF_Font*const font = (TTF_Font*)cf->_ttfFont;
    *(const void**)&cf->_ttfFont = NULL;
    if(font != Font_defaultFont_)
        TTF_CloseFont((TTF_Font*)cf->_ttfFont);
}

PixelBGRAArray* Pixels_engine_createArrayFromCharacter(Character c, coq_Font const coqFont) {
    TTF_Font*const font = (TTF_Font*)coqFont._ttfFont;
    if(!font) {
        PixelBGRAArray* pa = coq_callocArray(PixelBGRAArray, PixelBGRA, 4);
        memcpy(pa->pixels, test_pixels_, 32);
        return pa;
    }
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface* charSurf = TTF_RenderUTF8_Blended(font, c.c_str, white);
    int g_minx, g_maxx, g_miny, g_maxy, g_adv;
    TTF_GlyphMetrics32(font, c.c_data4, &g_minx, &g_maxx, &g_miny, &g_maxy, &g_adv);
    // printdebug("Info du char %s : width %d, minx %d, maxx %d, adv %d.",
    //     c.c_str, charSurf->w, g_minx, g_maxx, g_adv);
    const uint32_t* pixels;
    size_t width, height;
    if(charSurf) {
        SDL_LockSurface(charSurf);
        width =  charSurf->w;
        height = charSurf->h;
        pixels = charSurf->pixels;
    } else {
        printerror("Cannot create sdl surface for string %s.", c.c_str);
        width = 2; height = 2;
        pixels = test_pixels_;
    }
    // Pixel array
    size_t pixelCount = width * height;
    PixelBGRAArray *pa = coq_callocArray(PixelBGRAArray, PixelBGRA, pixelCount);
    pa->width = width; pa->height = height; pa->solidWidth = g_maxx - g_minx;
    pa->deltaX = g_minx;
    // On ne peut pas juste copier les pixels... la tailles des lignes est plus grande que le nombre de pixels par ligne.
    if(charSurf) {
        size_t lineSize = width * sizeof(PixelBGRA);
        PixelBGRA (*dstLigne)[width] = (void*)pa->pixels;
        PixelBGRA (* const dstLigneEnd)[width] = (void*)&pa->pixels[pixelCount];
        uint8_t   (*srcLigne)[charSurf->pitch] = (void*)pixels;
        for(; dstLigne < dstLigneEnd; dstLigne++, srcLigne++) {
            memcpy(dstLigne, srcLigne, lineSize);
        }
        SDL_UnlockSurface(charSurf);
        SDL_FreeSurface(charSurf);
    } else {
        memcpy(pa->pixels, pixels, pixelCount * sizeof(PixelBGRA));
    }
    return pa;
}
PixelBGRAArray* Pixels_engine_createArrayFromString(const char* c_str, coq_Font const coqFont) {
    TTF_Font*const font = (TTF_Font*)coqFont._ttfFont;
    if(!font) {
        PixelBGRAArray* pa = coq_callocArray(PixelBGRAArray, PixelBGRA, 4);
        memcpy(pa->pixels, test_pixels_, 32);
        return pa;
    }
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface* charSurf = TTF_RenderUTF8_Blended(font, c_str, white);
    const uint32_t* pixels;
    size_t width, height;
    if(charSurf) {
        SDL_LockSurface(charSurf);
        width =  charSurf->w;
        height = charSurf->h;
        pixels = charSurf->pixels;
    } else {
        printerror("Cannot create sdl surface for string %s.", c_str);
        width = 2; height = 2;
        pixels = test_pixels_;
    }
    // Pixel array
    size_t pixelCount = width * height;
    PixelBGRAArray *pa = coq_callocArray(PixelBGRAArray, PixelBGRA, pixelCount);
    pa->width = width; pa->height = height; pa->solidWidth = width;
    pa->deltaX = 0;
    if(charSurf) {
        size_t lineSize = width * sizeof(PixelBGRA);
        PixelBGRA (*dstLigne)[width] = (void*)pa->pixels;
        PixelBGRA (* const dstLigneEnd)[width] = (void*)&pa->pixels[pixelCount];
        uint8_t   (*srcLigne)[charSurf->pitch] = (void*)pixels;
        for(; dstLigne < dstLigneEnd; dstLigne++, srcLigne++) {
            memcpy(dstLigne, srcLigne, lineSize);
        }
        SDL_UnlockSurface(charSurf);
        SDL_FreeSurface(charSurf);
    } else {
        memcpy(pa->pixels, pixels, pixelCount * sizeof(PixelBGRA));
    }
    return pa;
}
