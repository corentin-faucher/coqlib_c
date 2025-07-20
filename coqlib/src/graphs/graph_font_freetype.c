//
//  graph_font_freetype.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2025-04-20.
//
#include "graph_font.h"

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
#include "../coq__buildConfig.h"
#include "../utils/util_base.h"
#include "../utils/util_map.h"

typedef struct {
    // Infos supplémentaire manquante avec freetype... 
    float heightRef; // Hauteur de référence `face->metrics.height` (pour normaliser).
    float bottom; // Différence entre `face->metrics.descender` et descender de `j`.
    float top;    // Top de `A`. (voir calcul plus bas)
    float bottomMargin; // Marge supplémentaire en bas.
    float topMargin; // Marge suppplémentaire en haut.
} FontExtraInfo_;
typedef struct {
    char           key_name[40];
    FontExtraInfo_ value_extraInfo;
} ExtraInfoMapEntry_;
static const ExtraInfoMapEntry_ fontInfo_arr_[] = {
// Ici, les infos sont généré avec fontSize = 60.
//   height, bottom, top, bottomMargin, topMargin (voir FontExtraInfo_)
    {"Arial Unicode MS",    { 80,  5, 60, 0, 0, }, },
    {"American Typewriter", { 69,  3, 56, 0, 0, },},
    {"Chalkduster",         { 77, -7, 61, 16, 3, },}, // Dépasse pas mal... e.g. `q`, `p`.
    {"Comic Sans MS",       { 84,  1, 61, 0, 0, },},
    {"Futura",              { 80,  1, 64, 0, 0, },},
    {"Snell Roundhand",     { 76,  2, 63, 0, 0, },},
    {"Brush Script MT",     { 74,  7, 60, 0, 0, },},
    {"Bradley Hand",        { 75, 3, 58, 0, 0, },},
    {"Chalkboard",          { 77, 7, 58, 0, 0, },},
    {"Courier",             { 60, 3, 56, 0, 0, },},
    // ... continuer...
};


typedef struct coq_Font {
    FT_Face const  ft_face;
    PixelBGR const color;
    bool const     nearest; // Pixelisé ou smooth ?
    /// Hauteur total requise pour dessiner les glyphes en pixels *avec* les fioritures qui depassent,
    ///  i.e. `ascender - descender`.
    size_t const   fullHeight;
    /// Hauteur de la hitbox en pixels sans les fioritures, i.e. `capHeight - descender`
    /// (et on a `solidHeight < glyphHeight`).
    /// ** -> Les autres dimensions sont normalisé par rapport à `solidHeight` **,
    float const    solidHeight;
    float const    deltaY;  // Décalage pour centre les glyphes.
    float const    _bottomExtraMargin;
} CoqFont;

#pragma mark - Dessin de fonts/glyphs dans buffer de pixels -
static char  Font_dir_[PATH_MAX-64] = {};
static char  Font_tmpFontPath_[PATH_MAX] = {};
static FT_Library ft_library_ = NULL;
static FT_Face    ft_defaultFont_ = NULL;
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
    FT_Set_Pixel_Sizes(ft_defaultFont_, 0, 60);
}
void  CoqFont_freetype_quit(void) {
    FT_Done_Face(ft_defaultFont_);
    FT_Done_FreeType(ft_library_);
    ft_defaultFont_ = NULL;
    ft_library_ = NULL;
    memset(Font_dir_, 0, PATH_MAX-64);
}

FontExtraInfo_ CoqFont_getExtraInfo_(char const* fontName);
void CoqFont_test_calculateFontExtraInfo_(CoqFont const* font);
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
    float fontHeight = ft_font->size->metrics.height / 64.f;
    // Malheureusement, freetype ne semble pas fournir les infos utiles sur les fonts... ??
    // Il faut donc les noter manuellement, voir liste `ExtraInfoMapEntry_` plus bas.
    FontExtraInfo_ const info = CoqFont_getExtraInfo_(ft_font->family_name);
    // Calcul des dimensions
    double solidHeight; // descender + capheight
    double fullHeight;  // Tout l'espace en y nécessaire pour dessiner un glyph.
    double bottom;      // coord y de baseLine - descender
    double top;         // coord y de baseline + capheight
    double bottomExtra, topExtra;
    double topMargin;   // fullHeight - top (espace restant en haut pour fioritures) 
    if(info.heightRef) {
        top =    info.top / info.heightRef * fontHeight;
        bottom = info.bottom / info.heightRef * fontHeight;
        solidHeight = top - bottom;
        bottomExtra = info.bottomMargin / info.heightRef * fontHeight + FONT_extra_margin(init.nearest);
        topExtra = info.topMargin / info.heightRef * fontHeight + FONT_extra_margin(init.nearest);
        bottom += bottomExtra;
        top += bottomExtra;
    }  else { // Valeurs par défaut si manque d'info finetuned...
        printwarning("Need to define extra info for font %s (set COQ_TEST_FONT true).", ft_font->family_name);
        solidHeight = asc + fabsf(des);
        bottomExtra = FONT_extra_margin(init.nearest);
        topExtra = FONT_extra_margin(init.nearest);
        bottom = bottomExtra;
        top = fmin(solidHeight, fontHeight) + bottomExtra;
    }
    fullHeight = fontHeight + topExtra + bottomExtra;
    topMargin = fullHeight - top;
    // Décalage pour recentrer la texture.
    double const deltaY = 0.5*(topMargin - bottom);
    
    if(COQ_TEST_FONT) {
        printf("🐷 Info pour font %s: size %d, ascender %d, descender %d, height %d.\n"
               "   └ calculées: fullHeight %d, solidHeight %d, bottom %d, top %d.\n",
                ft_font->family_name, (int)fontSize, (int)asc, (int)des, (int)fontHeight,
                (int)fullHeight, (int)solidHeight, (int)bottom, (int)top 
        );
    }
    // Save data
    CoqFont*const cf = coq_callocTyped(CoqFont);
    *(FT_Face*)&cf->ft_face = ft_font;
    *(PixelBGR*)&cf->color = init.color;
    bool_initConst(&cf->nearest, init.nearest);
    size_initConst(&cf->fullHeight, fullHeight);
    float_initConst(&cf->solidHeight, solidHeight);
    float_initConst(&cf->deltaY, deltaY);
    float_initConst(&cf->_bottomExtraMargin, bottomExtra);
    
    if(COQ_TEST_FONT)
        CoqFont_test_calculateFontExtraInfo_(cf);
        
    return cf;
}
void     coqfont_engine_destroy(CoqFont**const cfRef) {
    CoqFont*const cf = *cfRef;
    *cfRef = (CoqFont*)NULL;
    FT_Face const face = cf->ft_face;
    *(FT_Face*)&cf->ft_face = (FT_Face)NULL;
    if(face && (face != ft_defaultFont_))
        FT_Done_Face(face);
    coq_free(cf);
}

CoqFontDims coqfont_dims(CoqFont const* cf) {
    return (CoqFontDims) {
        .deltaY = cf->deltaY,
        .fullHeight = cf->fullHeight,
        .solidHeight = cf->solidHeight,
//        .relFullHeihgt = cf->fullHeight / cf->solidHeight,
//        .relDeltaY = cf->deltaY / cf->solidHeight,
        .nearest = cf->nearest,
    };
}


typedef struct {
    int64_t deltaX, deltaY;
    int64_t bottom, top;
    uint64_t solidWidth;
    uint64_t bitmapWidth, bitmapHeight;
    int64_t  bitmapFirstRow, bitmapLastRow; 
    uint8_t* bitmapPixels;
} GlyphInfos_;
GlyphInfos_ freetype_generateCharAndGetDims_(CoqFont const*const font, Character const c) {
    guard_let(FT_Face, face, font->ft_face, , (GlyphInfos_) {})
    uint32_t c_u32 = character_toUnicode32(c);
    int error = FT_Load_Char(face, c_u32, FT_LOAD_RENDER); // FT_LOAD_COLOR...
    if(error) {
        printerror("Could not load character %s %d: %d %s.",
            c.c_str, c_u32, error, FT_Error_String(error));
        return (GlyphInfos_) {};
    } 
    FT_GlyphSlot const g = face->glyph;
    int64_t const gh =  g->metrics.height / 64;
    int64_t const deltaY = g->metrics.horiBearingY / 64 - gh;
    int64_t const maxDes = labs(g->face->size->metrics.descender / 64);
    int64_t const bottom = maxDes + deltaY;
    int64_t const drawBottom = bottom + (int64_t)font->_bottomExtraMargin;
    int64_t firstRow = (int64_t)font->fullHeight - (int64_t)g->bitmap.rows - drawBottom;
    int64_t lastRow = (int64_t)font->fullHeight - drawBottom;
    
    
    return (GlyphInfos_) {
        .deltaX = (g->metrics.horiBearingX - 0.5*g->metrics.horiAdvance + 0.5*g->metrics.width) / 64,
        .deltaY = deltaY,
        .bottom = bottom,
        .top =    bottom + gh,
        .solidWidth =   g->metrics.horiAdvance / 64,
        .bitmapWidth =  g->bitmap.width,
        .bitmapHeight = g->bitmap.rows,
        .bitmapFirstRow = firstRow,
        .bitmapLastRow = lastRow,
        .bitmapPixels = g->bitmap.buffer,
    };
}
PixelBGRAArray* Pixels_createDummy_(void) {
    static uint32_t const test_pixels_[4] = {
        0x00FFFFFF, 0xFFFFFFFF, 0x1F00FFFF, 0xFFFF00FF,
    };
    PixelBGRAArray* pa = PixelBGRAArray_createEmpty(2, 2);
    memcpy(pa->pixels, test_pixels_, 16);
    return pa;
}
PixelBGRAArray* Pixels_engine_createArrayFromCharacter(Character const c, 
                    CoqFont const*const coqFont) 
{
    guard_let(FT_Face, font, coqFont->ft_face, printerror("Font not init."), Pixels_createDummy_())
    GlyphInfos_ const glyph = freetype_generateCharAndGetDims_(coqFont, c);
    // Init du PixelArray à exporter
    size_t const extra_margin = FONT_extra_margin(coqFont->nearest);
    size_t const pa_width = glyph.bitmapWidth + 2*extra_margin;
    size_t const pa_height = coqFont->fullHeight;
    size_t const pa_pixelCount = pa_width * pa_height;
    PixelBGRAArray* pa = PixelBGRAArray_createEmpty(pa_width, pa_height);
    pa->deltaX = glyph.deltaX;
    pa->deltaY = glyph.deltaY;
    pa->solidWidth = glyph.solidWidth;
    pa->solidHeight = coqFont->solidHeight;
               
    // Filling pixels loop
    int64_t firstRow = glyph.bitmapFirstRow;
    if(firstRow < 0) {
        printwarning("Missing %d y top margin...", -(int)firstRow);
        firstRow = 0;
    }
    PixelBGRA (*dst_row)[pa_width] =      (void*)&pa->pixels[pa_width * firstRow]; 
    PixelBGRA (*const dst_row_end)[pa_width] = (void*)&pa->pixels[pa_pixelCount];
    size_t const bitmap_pixelCount = glyph.bitmapWidth * glyph.bitmapHeight;
    uint8_t (*src_row)[glyph.bitmapWidth] = (void*)&glyph.bitmapPixels[0];
    uint8_t (*const src_row_end)[glyph.bitmapWidth] = (void*)&glyph.bitmapPixels[bitmap_pixelCount];
    PixelBGR const fontColor = coqFont->color; 
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

FontExtraInfo_ CoqFont_getExtraInfo_(char const*const fontName) {
    if(!fontName) return (FontExtraInfo_) {};
    // Sans doute superflu de faire une hash map juste pour retrouver les extra info ?
    // Juste un `strncmp` ?
    static StringMap* fontInfoOfNamed = NULL;
    // Init de la map.
    if(fontInfoOfNamed == NULL) {
        fontInfoOfNamed = Map_create(40, sizeof(FontExtraInfo_));
        const ExtraInfoMapEntry_* p =   fontInfo_arr_;
        const ExtraInfoMapEntry_* end = coq_simpleArrayEnd(fontInfo_arr_, ExtraInfoMapEntry_);
        while(p < end) {
            map_put(fontInfoOfNamed, p->key_name, &p->value_extraInfo);
            p++;
        }
    }
    
    FontExtraInfo_ const* extra = (const FontExtraInfo_*)map_valueRefOptOfKey(fontInfoOfNamed, fontName);
    if(extra) return *extra;
    return (FontExtraInfo_) {};
}

void CoqFont_test_printAvailableFonts(void) {
    guard_let(DIR*, d, opendir(Font_dir_), printerror("Cannot open font dir."), )
    for(struct dirent* dir = readdir(d); dir != NULL; dir = readdir(d)) {
        if(dir->d_type != DT_REG) continue;
        printf("Font file : %s.\n", dir->d_name);
    }
    closedir(d);
}

void glyph_printinfo(Character c, GlyphInfos_ glyph) {
    printf("🐷 %s bottom %d, top %d, first row (top) %d, last row (bottom) %d.\n",
        c.c_str, (int)glyph.bottom, (int)glyph.top,
        (int)glyph.bitmapFirstRow, 
        (int)(glyph.bitmapFirstRow + glyph.bitmapHeight)
    );
}

void CoqFont_test_calculateFontExtraInfo_(CoqFont const*const font) {
    guard_let(FT_Face, face, font->ft_face, printerror("Font not init."), )
    long int fontHeight = face->size->metrics.height / 64;
    
    Character c = {.c_str = "j" };
    GlyphInfos_ glyph = freetype_generateCharAndGetDims_(font, c);
//    glyph_printinfo(c, glyph);
    int64_t bottom = glyph.bottom;
    int64_t lastRow = glyph.bitmapLastRow;
    
    strcpy(c.c_str, "p");
    glyph = freetype_generateCharAndGetDims_(font, c);
//    glyph_printinfo(c, glyph);
    if(glyph.bitmapLastRow > lastRow) lastRow = glyph.bitmapLastRow;
    
    strcpy(c.c_str, "A");
    glyph = freetype_generateCharAndGetDims_(font, c);
//    glyph_printinfo(c, glyph);
    int64_t top = glyph.top;
    
    strcpy(c.c_str, "À");
    glyph = freetype_generateCharAndGetDims_(font, c);
//    glyph_printinfo(c, glyph);
    int64_t firstRow = glyph.bitmapFirstRow;
    int64_t extraTopMargin = (firstRow < 0) ? -firstRow : 0;
    int64_t extraBottomMargin = (lastRow > font->fullHeight) ? lastRow - font->fullHeight : 0;
    printf("🐷 Font %s : height %d, bottom %d, top %d, extraBottom %d, extraTop %d.\n",
        face->family_name, (int)fontHeight,
        (int)bottom, (int)top, 
        (int)extraBottomMargin, (int)extraTopMargin 
    );
    printf(" └->  {\"%s\",     { %d, %d, %d, %d, %d, },},\n",
        face->family_name, (int)fontHeight,
        (int)bottom, (int)top, 
        (int)extraBottomMargin, (int)extraTopMargin 
    );
    printf("(put in graph_font_freetype.c -> fontInfo_arr_.)\n");
}
