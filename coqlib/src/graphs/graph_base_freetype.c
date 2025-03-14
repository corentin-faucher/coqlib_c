//
//  graph_base_freetype.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2025-03-11.
//
#include "graph_base.h"
#include "../utils/util_base.h"
#include "../utils/util_language.h"
#include <dirent.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#if __APPLE__
#include <sys/syslimits.h>
#endif
#ifdef __linux__
#include <limits.h>
#endif

typedef struct coq_Font {
//    union {
        // Version Apple -> Dictionnaire d'attributes (dont la NSFont/UIFont)
//        void const*const _nsdict_attributes;
        // Version Freetype -> FT_Face
        void const*const _ft_face;
//    };
    PixelBGR const color;
    bool const     nearest; // Pixelisé ou smooth ?
    /// Hauteur total requise pour dessiner les glyphes en pixels *avec* les fioritures qui depassent,
    ///  i.e. `ascender - descender`.
    size_t const   fullHeight;
    /// Hauteur de la hitbox en pixels sans les fioritures, i.e. `capHeight - descender`
    /// (et on a `solidHeight < glyphHeight`).
    /// ** -> Les autres dimensions sont normalisé par rapport à `solidHeight` **,
    float const    solidHeight;
    float const    deltaY;  // Décalage pour centre les glyphes, i.e. (ascender - capHeight)/2.
//    float const  _drawDeltaY;  ("Privé", décalage pour dessiner...)
//    float const  _maxDesc;    // ("Privé" distance positive entre bas et baseline, un peu plus que descender)
} CoqFont;

#pragma mark - Dessin de fonts/glyphs dans buffer de pixels -
static char  Font_dir_[PATH_MAX-64] = {};
static char  Font_tmpFontPath_[PATH_MAX] = {};
static FT_Library ft_library_ = NULL;
static FT_Face    ft_defaultFont_ = NULL;
//static TTF_Font* Font_defaultFont_ = NULL;
const char* CoqFont_getFontPath_(const char* fileName) {
    memset(Font_tmpFontPath_, 0, PATH_MAX);
    sprintf(Font_tmpFontPath_, "%s/%s", Font_dir_, fileName);
    return Font_tmpFontPath_;
}
void  CoqFont_freetype_init(const char* fontsPath, const char* defaultFontFileName) {
    if(!fontsPath || !defaultFontFileName) {
        printerror("No fonts path or default font."); return;
    }
    if(ft_library_) {
        printwarning("Fonts path already set."); return;
    }
    strcpy(Font_dir_, fontsPath);
    if(FT_Init_FreeType(&ft_library_)) {
        printerror("Could not init freetype."); return;
    }
    const char* defaultFontPath = CoqFont_getFontPath_(defaultFontFileName);
    if(FT_New_Face(ft_library_, defaultFontPath, 0, &ft_defaultFont_)) {
        printerror("could not load default font %s.", defaultFontFileName); return;
    }
    FT_Set_Pixel_Sizes(ft_defaultFont_, 0, 48);
}
void  CoqFont_freetype_quit(void) {
    FT_Done_Face(ft_defaultFont_);
    FT_Done_FreeType(ft_library_);
    ft_defaultFont_ = NULL;
    ft_library_ = NULL;
    memset(Font_dir_, 0, PATH_MAX-64);
}

CoqFont* CoqFont_engine_create(CoqFontInit const init)
{
    if(!ft_library_ || !ft_defaultFont_) { printerror("Freetype not init."); return NULL;}
    double fontSize = init.sizeOpt ? init.sizeOpt : 32;
    if(!fontSize) fontSize = 32; // (Default)
    if(fontSize < 12) { printwarning("Font too small."); fontSize = 12; }
    if(fontSize > 200) { printwarning("Font too big.");  fontSize = 200; }
    // Obtenir une font
    FT_Face ft_font = NULL;
    if(init.fileNameOpt) {
        int error = FT_New_Face(ft_library_, CoqFont_getFontPath_(init.fileNameOpt), 0, &ft_font);
        if(error) {
            printerror("Could not load font %s : %d, %s.", 
                       init.nameOpt, error, FT_Error_String(error)); 
        } else {
            FT_Set_Pixel_Sizes(ft_font, 0, fontSize);
        }
    }
    if(!ft_font) {
        ft_font = ft_defaultFont_;
    }
    
    float asc = ft_font->size->metrics.ascender / 64.f;
    float des = ft_font->size->metrics.descender / 64.f;
    float hei = ft_font->size->metrics.height / 64.f;
    
    double const solidHeight = asc + fabsf(des);
                            // [font capHeight] + descender;
    double const glyphHeight = hei;
                            // [font ascender] + descender;
    LanguageFontInfo const *info = NULL; //LanguageFont_getFontInfoOpt(init.nameOpt);
    double const topMargin = FONT_extra_margin(init.nearest) 
            + (info ? info->topMargin*solidHeight : 0.f);
    double const bottomMargin = FONT_extra_margin(init.nearest)
            + (info ? info->bottomMargin*solidHeight : 0.f);
    size_t const fullHeight = ceil(glyphHeight + topMargin + bottomMargin);
    
    // TODO: Ajuster fullHeight et solidHeight au cas par cas avec une liste des fonts...
    
    // Décalage pour recentrer la texture.
//    double const deltaY = 0.5*(topMargin - bottomMargin);
//    printdebug("Info of font %s : size %f, Font fullHeight %zu, solidHeight %f, top %f, bottom %f.",
//               ft_font->family_name, fontSize, fullHeight, solidHeight, topMargin, bottomMargin);
//    printdebug("Font asc %f, des %f, hei %f.", asc, des, hei);
//    printdebug("Dy %f.", deltaY);
    
    // Save data
    CoqFont*const cf = coq_callocTyped(CoqFont);
    *(const void**)&cf->_ft_face = ft_font;
    *(PixelBGR*)&cf->color = init.color;
    bool_initConst(&cf->nearest, init.nearest);
    size_initConst(&cf->fullHeight, fullHeight);
    float_initConst(&cf->solidHeight, solidHeight);
    float_initConst(&cf->deltaY, deltaY);
//    float_initConst(&cf->_drawDeltaY, drawDeltaY);
    return cf;
}
void     coqfont_engine_destroy(CoqFont**const cfRef) {
    CoqFont*const cf = *cfRef;
    *cfRef = (CoqFont*)NULL;
    FT_Face const font = (FT_Face)cf->_ft_face;
    *(const void**)&cf->_ft_face = NULL;
    if(font && (font != ft_defaultFont_))
        FT_Done_Face(font);
    coq_free(cf);
}

CoqFontDims coqfont_dims(CoqFont const* cf) {
    return (CoqFontDims) {
        .deltaY = cf->deltaY,
        .fullHeight = cf->fullHeight,
        .solidHeight = cf->solidHeight,
        .relFullHeihgt = cf->fullHeight / cf->solidHeight,
        .relDeltaY = cf->deltaY / cf->solidHeight,
        .nearest = cf->nearest,
    };
}

PixelBGRAArray* Pixels_createDummy_(void) {
    static uint32_t const test_pixels_[4] = {
        0x00FFFFFF, 0xFFFFFFFF, 0x1F00FFFF, 0xFFFF00FF,
    };
    PixelBGRAArray* pa = coq_callocArray(PixelBGRAArray, PixelBGRA, 4);
    size_initConst(&pa->width, 2);
    size_initConst(&pa->height, 2);
    pa->solidWidth = 2; pa->solidHeight = 2;
    memcpy(pa->pixels, test_pixels_, 32);
    return pa;
}
PixelBGRAArray* Pixels_engine_createArrayFromCharacter(Character c, CoqFont const* coqFont) {
    guard_let(FT_Face, font, (FT_Face)coqFont->_ft_face, printerror("Font not init."), Pixels_createDummy_())
    uint32_t c_u32 = character_toUnicode32(c);
    int error = FT_Load_Char(font, c_u32, FT_LOAD_RENDER); // FT_LOAD_COLOR...
    if(error) {
        printerror("Could not load character %s %d: %d %s.",
            c.c_str, c_u32, error, FT_Error_String(error));
        return Pixels_createDummy_();
    } 
    FT_GlyphSlot g = font->glyph;
    int64_t const x = (g->metrics.horiBearingX + 0.5*g->metrics.width - 0.5*g->metrics.horiAdvance) / 64;
    int64_t const hBy = g->metrics.horiBearingY / 64;
    int64_t const gh =  g->metrics.height / 64;
    int64_t const gh_u = g->bitmap.rows;
    int64_t const des = gh - hBy;
    int64_t const maxDes = labs(g->face->size->metrics.descender / 64);
    int64_t Dy = maxDes - des;
    float const y = (g->metrics.horiBearingY - g->metrics.height)/64.f; // + coqFont->_maxDesc ?
//    printdebug("Printing char %s : hBy %lld, h %lld, h_u %lld, des %lld, Dy %lld, bit_top %i, bit_left %i.",
//        c.c_str, hBy, gh, gh_u, des, Dy, g->bitmap_top, g->bitmap_left
//    );
    
    // Init du PixelArray à exporter
    size_t const extra_margin = FONT_extra_margin(coqFont->nearest);
    size_t const pa_width = g->bitmap.width + 2*extra_margin;
    size_t const pa_height = coqFont->fullHeight;
    size_t const pa_pixelCount = pa_width * pa_height;
    PixelBGRAArray* pa = coq_callocArray(PixelBGRAArray, PixelBGRA, pa_pixelCount);
    size_initConst(&pa->width,  pa_width);
    size_initConst(&pa->height, pa_height);
    pa->deltaX = x;
    pa->deltaY = y;
    pa->solidWidth = g->metrics.horiAdvance / 64;
    pa->solidHeight = coqFont->solidHeight;
    
//    printdebug("Pixel Array: %zu x %zu, %f x %f.",
//               pa->width, pa->height, pa->solidWidth, pa->solidHeight);
               
    // Filling pixels loop
    PixelBGR const fontColor = coqFont->color;
    int64_t dst_firstRow = (int64_t)pa_height - (int64_t)g->bitmap.rows - Dy;
    if(dst_firstRow < 0) {
        printwarning("Need %lld more y margin to draw char %s.", -dst_firstRow, c.c_str);
        dst_firstRow = 0;
    }
    PixelBGRA (*dst_row)[pa_width] =      (void*)&pa->pixels[pa_width * dst_firstRow]; 
    PixelBGRA (*const dst_row_end)[pa_width] = (void*)&pa->pixels[pa_pixelCount];
    size_t const bitmap_pixelCount = g->bitmap.width * g->bitmap.rows;
    uint8_t (*src_row)[g->bitmap.width] = (void*)&g->bitmap.buffer[0];
    uint8_t (*const src_row_end)[g->bitmap.width] = (void*)&g->bitmap.buffer[bitmap_pixelCount]; 
    for(; (dst_row < dst_row_end) && (src_row < src_row_end); dst_row++, src_row++) {
        PixelBGRA* p =          &(*dst_row)[extra_margin]; // (décalage pour marge en x)
        PixelBGRA*const p_end = *(dst_row + 1);
        uint8_t* gp =          *src_row;
        uint8_t*const gp_end = *(src_row + 1);
        for(; (p < p_end) && (gp < gp_end); p++, gp++) {
            *p = (PixelBGRA) { .bgr = fontColor, .a = *gp };
        }
    }
    return pa;
}
PixelBGRAArray* Pixels_engine_test_createArrayFromString_(const char* c_str, CoqFont const* coqFont)
{
    printerror("Undefined with freetype...");
    return Pixels_createDummy_();
}

void CoqFont_test_printAvailableFonts(void) {
    guard_let(DIR*, d, opendir(Font_dir_), printerror("Cannot open font dir."), )
    for(struct dirent* dir = readdir(d); dir != NULL; dir = readdir(d)) {
        if(dir->d_type != DT_REG) continue;
        printf("Font file : %s.\n", dir->d_name);
    }
    closedir(d);
}
