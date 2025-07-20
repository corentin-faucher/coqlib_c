//
//  graph_font_apple.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2025-04-19.
//

#include "graph_font.h"
#include "../utils/util_base.h"
#include "../utils/util_string.h"
#include "../coq__buildConfig.h"
#include "../utils/util_map.h"
#import <AppKit/AppKit.h>

// MARK: - Dessin de glyph (fonts)
#if TARGET_OS_OSX == 1
typedef NSFont Font;
#else
typedef UIFont Font;
#endif
/// Référence pour la théorie (cap height, baseline, descender, ...) : 
/// https://en.wikipedia.org/wiki/Typeface_anatomy

typedef struct coq_Font {
//    union {
        // Version Apple -> Dictionnaire d'attributes (dont la NSFont/UIFont)
        void const*const _nsdict_attributes;
        // Version Freetype -> FT_Face
//        void const*const _ft_face;
//    };
    bool const   nearest; // Pixelisé ou smooth ?
    float const  fullHeight;
    float const  solidHeight;
    float const  fontHeight; // `ascender - descender`.
    float const  deltaY;  // Décalage pour centre les glyphes, i.e. (ascender - capHeight)/2.
    float const  _bottomExtraMargin;
    // float const  _maxDesc;    // ("Privé" distance positive entre bas et baseline, un peu plus que descender)
} CoqFont;

// Les infos supplémentaire pour les fonts à "ajuster"...
typedef struct {
    // Infos supplémentaire manquante avec freetype...
    /// Hauteur de référence `face->metrics.height` (pour normaliser), devrait être ascender - descender.
    float heightRef; 
    /// bottom : différence entre `face->metrics.descender` et descender de `j` (0 en théorie).
    float bottom;
    /// top : top de `A` (en théorie `bottom + capHeight - descender`, où `descender < 0`). 
    float top;
    // Marges extras pour font problématiques... 
    // (descender ou ascender qui n'inclus pas les fioritures...)
    float bottomMargin;
    float topMargin;
} FontExtraInfo_;
void CoqFont_test_calculateFontExtraInfo_(CoqFont const* coqfont);
FontExtraInfo_ CoqFont_getExtraInfo_(char const* fontName);
CoqFont* CoqFont_engine_create(CoqFontInit const init) {
    double fontSize = init.sizeOpt ? init.sizeOpt : 32; // (Default)
    if(fontSize < 12) { printwarning("Font too small."); fontSize = 12; }
    if(fontSize > 200) { printwarning("Font too big.");  fontSize = 200; }
    // Set font
    Font* font;
    if(init.nameOpt)
        font = [Font fontWithName:[NSString stringWithUTF8String:init.nameOpt]
                                 size:fontSize];
    else
        font = [Font systemFontOfSize:fontSize];
    if(font == nil) { 
        printwarning("Font %s not found, taking system font.", init.nameOpt);
        font = [Font systemFontOfSize:fontSize];
    }
    // Style... Ajouter la couleur comme paramètre d'init ?
    NSMutableParagraphStyle* paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    paragraphStyle.alignment = NSTextAlignmentCenter;
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
    NSDictionary* attributes = [NSMutableDictionary 
        dictionaryWithObjects:@[font,  paragraphStyle,                [NSColor blackColor]         ]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName]];
    
    // Dimensions
    double const fontHeight = [font ascender] - [font descender];
    FontExtraInfo_ const info = CoqFont_getExtraInfo_([[font fontName] UTF8String]);
    // Calcul des dimensions
    double bottom;      // Bas de descender, i.e. `baseLine + descender`, où `descender < 0`.
    double top;         // Entre capHeight et ascender, i.e. `baseline + 0.5*(capheight+ascender)`.
    double solidHeight; // L'espace "occupé" en y, i.e. `top - bottom`.
    double fullHeight;  // Tout l'espace en y nécessaire pour dessiner un glyph,
                        // typiquement `ascender - descender + extraYmargin`.
    
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
        printwarning("Need to define extra info for font %s.", [[font fontName] UTF8String]);
        solidHeight = 0.5*([font ascender] + [font capHeight]) - [font descender];
        bottomExtra = FONT_extra_margin(init.nearest);
        topExtra = FONT_extra_margin(init.nearest);
        bottom = bottomExtra;
        top = fmin(solidHeight, fontHeight) + bottomExtra;
    }
    fullHeight = fontHeight + topExtra + bottomExtra;
    topMargin = fullHeight - top;
    // Décalage pour recentrer la texture.
    double const deltaY = 0.5*(topMargin - bottom);
    
    // Save data
    CoqFont*const cf = coq_callocTyped(CoqFont);
    *(const void**)&cf->_nsdict_attributes = CFBridgingRetain(attributes);
    
    bool_initConst(&cf->nearest, init.nearest);
    float_initConst(&cf->fullHeight, fullHeight);
    float_initConst(&cf->solidHeight, solidHeight);
    float_initConst(&cf->fontHeight, fontHeight);
    float_initConst(&cf->deltaY, deltaY);
    float_initConst(&cf->_bottomExtraMargin, bottomExtra);
    
    if(COQ_TEST_FONT)
        CoqFont_test_calculateFontExtraInfo_(cf);
    
    return cf;
}
void    coqfont_engine_destroy(CoqFont**const cfRef) {
    CoqFont*const cf = *cfRef;
    *cfRef = (CoqFont*)NULL;
    CFRelease(cf->_nsdict_attributes);
    coq_free(cf);
}

CoqFontDims coqfont_dims(CoqFont const* cf) {
    return (CoqFontDims) {
        .deltaY = cf->deltaY,
        .fullHeight = cf->fullHeight,
        .solidHeight = cf->solidHeight,
//        .relGlyphHeihgt = cf->fullHeight / cf->solidHeight,
//        .relGlyphY = cf->deltaY / cf->solidHeight,
        .nearest = cf->nearest,
    };
}



typedef struct {
    Character c;
    double   deltaX, deltaY;
    double   solidWidth, solidHeight;
    NSRect   drawRect; // Rectangle de dessin CoreGraphics dans context NSGraphics.
    double   bottom, top; // Pour ajuster les marges supplémentaire en bas et en haut. 
} GlyphInfo_;
GlyphInfo_ character_getGlyphInfo_(Character const c, CoqFont const*const coqfont)
{
    NSDictionary* attributes = (__bridge NSDictionary*)coqfont->_nsdict_attributes;
    Font* font = [attributes valueForKey:NSFontAttributeName];
    NSString* string = [NSString stringWithUTF8String:c.c_str];
    if(stringUTF8_isSingleEmoji(c.c_str)) {
        font = [Font systemFontOfSize:[font pointSize]];
    }
    CTFontRef const font_ct = (__bridge CTFontRef)font;
    // Obetnir les dimensions d'un char avec AppKit `sizeWithAttributes`
    // et CoreText `CTFontGetGlyphsForCharacters`...
    CGSize const string_size = [string sizeWithAttributes:attributes];
    NSRect glyphRect = ({
        NSUInteger const string_length = [string length];
        unichar characters[string_length + 1];
        [string getCharacters:characters range:NSMakeRange(0, string_length)];
        characters[string_length] = 0;
        CGGlyph glyphs[string_length];
        CTFontGetGlyphsForCharacters(font_ct, characters, glyphs, string_length);
        [font boundingRectForCGGlyph:glyphs[0]];
    });
    if(glyphRect.size.width < string_size.width)
        glyphRect.size.width = string_size.width;
    double extra_margin = FONT_extra_margin(coqfont->nearest);
    double deltaX = glyphRect.origin.x - 0.5*string_size.width + 0.5*glyphRect.size.width;
    double marginX = fmaxf(0.5*(string_size.width - glyphRect.size.width) + fabs(deltaX), 0) + extra_margin;
//    printf("🐷 %s : x %.1f, g_w %.1f, s_w %.1f, Dx %.1f, m_x %.1f.\n",
//           c.c_str, glyphRect.origin.x, glyphRect.size.width, string_size.width, deltaX, marginX);
           
    return (GlyphInfo_) {
        .c = c,
        // Décalage x/y pour centrer le char.
        .deltaX = deltaX,
        .deltaY = coqfont->deltaY,  // (constant pour tout char)
        .solidWidth = string_size.width, // Espace occupé par le char.
        .solidHeight = coqfont->solidHeight, // (constant pour tout char)
        .drawRect = { // Positionnement pour dessiner avec CoreGraphics.
            .origin = { -glyphRect.origin.x + marginX, //  ,
                        coqfont->_bottomExtraMargin 
            },
            .size = { glyphRect.size.width + 2*marginX, coqfont->fullHeight },
        },
        .bottom = glyphRect.origin.y - [font descender], // bas du glyph par rapport à descender.
        .top =    glyphRect.origin.y - [font descender] + glyphRect.size.height,
    };
}

PixelBGRAArray* Pixels_engine_createArrayFromCharacter(Character const c, CoqFont const* coqFont) 
{
    NSDictionary* attributes = (__bridge NSDictionary*)coqFont->_nsdict_attributes;
    GlyphInfo_ info = character_getGlyphInfo_(c, coqFont);
    NSString* string = [NSString stringWithUTF8String:c.c_str];
    // 1. Setter la Destination : Pixel Array
    PixelBGRAArray*const pa = PixelBGRAArray_createEmpty(info.drawRect.size.width, 
                                                         info.drawRect.size.height);
    pa->deltaX = info.deltaX;
    pa->deltaY = info.deltaY;
    pa->solidWidth = info.solidWidth;
    pa->solidHeight = info.solidHeight;
    
    // 2. Init CoreGraphics context
    CGColorSpaceRef const colorSpace = CGColorSpaceCreateDeviceRGB(); // ? pas de BGR ?
    NSUInteger const bytesPerRow = pa->width * sizeof(PixelBGRA);
    NSUInteger const bitsPerComponent = 8;
    CGContextRef const context = CGBitmapContextCreate(pa->pixels, pa->width, pa->height,
                    bitsPerComponent, bytesPerRow, colorSpace,
                    kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);
    CGContextSetTextDrawingMode(context, kCGTextFillStroke);
    
    // 3. Dessiner la string dans le context
    //    CTFontDrawGlyphs(font_ct, glyphs, &drawPoint, 1, context);
    // (set context CoreGraphics dans context NSGraphics pour dessiner la NSString.)
    @autoreleasepool {
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithCGContext:context flipped:false]];
        [string drawAtPoint:info.drawRect.origin withAttributes:attributes];
        [NSGraphicsContext setCurrentContext:nil];
        [NSGraphicsContext restoreGraphicsState];
    }
    // 4. Relâcher le context CoreGraphic
    CGContextRelease(context);
    // 5. Swapper Red et Blue pour rgb -> bgr...
    PixelBGRA*const end = &pa->pixels[pa->width*pa->height];
    for(PixelBGRA* p = pa->pixels; p < end; p++) {
        *p = (PixelBGRA) {.b = p->r, .g = p->g, .r = p->b, .a = p->a, };
    }
    
    return pa;
}

typedef struct {
    char           key_name[40];
    FontExtraInfo_ value_extraInfo;
} ExtraInfoMapEntry_;
static const ExtraInfoMapEntry_ fontInfo_arr_[] = {
// Ici, les infos sont généré avec fontSize = 60.
//   height, bottom, top, bottomMargin, topMargin (voir FontExtraInfo_)
    {".AppleSystemUIFont",     { 71, 2, 61, 0, 0, },},
    {"AmericanTypewriter",     { 70, 2, 62, 0, 0, },},
    {"ChalkboardSE-Regular",   { 85, 7, 66, 0, 0, },},
    {"Chalkduster",           { 76, -7, 71, 17, 5, },},
    {"ComicSansMS",        { 84, 0, 71, 0, 0, },},
    {"Courier",            { 60, 3, 57, 0, 4, },},
    {"Futura-Medium",      { 78, 0, 71, 0, 0, },},
    {"Helvetica",          { 60, 1, 63, 0, 9, },},
    {"JCfg",               { 61, 0, 58, 0, 8, },},
    {"NanumGothic",        { 69, 1, 62, 0, 0, },},
    {"SnellRoundhand",     { 76, 1, 68, 0, 0, },},
    {"TimesNewRomanPSMT",  { 67, 0, 60, 0, 0, },},
    {"Verdana",            { 73, 0, 65, 0, 0, },},
    // TODO: continuer...
//    {"Luciole", "Luciole", },
//    {"Nanum Myeongjo", "NanumMyeongjo", 1.f, 1.2f},
//    {"Nanum Pen Script", },
//    {"BM Kirang Haerang",   },
//    {"GungSeo",          },
//    {"Hiragino Maru Gothic ProN", "Hiragino MGP",},
//    {"Klee", "Klee", },
//    {"OpenDyslexic3", "Op. Dyslex3", },
//    {"Osaka", "Osaka", },
//    {"Toppan Bunkyu Gothic", "Toppan BG", },
//    {"Toppan Bunkyu Midashi Gothic", "Toppan BMG", },
//    {"Toppan Bunkyu Midashi Mincho", "Toppan BMM", },
//    {"Toppan Bunkyu Mincho", "Toppan BM", },
//    {"Tsukushi A Round Gothic", "Tsukushi A", },
//    {"Tsukushi B Round Gothic", "Tsukushi B", },
//    {"YuKyokasho Yoko", "YuKyokasho", },
//    {"YuMincho", "YuMincho", },
//    {"Apple SD Gothic Neo", "Apple SD Goth.N.", },
//    {"Apple LiSung", "Apple LiSung", },
//    {"Baoli SC", "Baoli", },
//    {"GB18030 Bitmap", "Bitmap", },
//    {"HanziPen SC", "HanziPen", },
//    {"Hei", "Hei", },
//    {"LingWai TC", "LingWai", },
//    {"AppleMyungjo", "Myungjo", },
//    {"PingFang SC", "PingFang", },
//    {"Weibei SC", "Weibei", },
//    {"Farah", "Farah", },
};
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

void printCharInfo_(GlyphInfo_ const g) {
    printf("🐷   info of %s : bottom %f, top %f.\n",
           g.c.c_str, g.bottom, g.top
    );
}
void CoqFont_test_calculateFontExtraInfo_(CoqFont const* coqfont) {
    NSDictionary* attributes = (__bridge NSDictionary*)coqfont->_nsdict_attributes;
    Font* font = [attributes valueForKey:NSFontAttributeName];
    float const fontHeight = coqfont->fontHeight;
    
    Character c = {.c_str = "j"};
    GlyphInfo_ info = character_getGlyphInfo_(c, coqfont);
    printCharInfo_(info);
    float bottom = info.bottom; 
    c = (Character){.c_str = "p"};
    info = character_getGlyphInfo_(c, coqfont);
    printCharInfo_(info);
    float bottomExtra = fmaxf(0, -fminf(info.bottom, bottom));
    
    c = (Character){.c_str = "A"};
    info = character_getGlyphInfo_(c, coqfont);
    printCharInfo_(info);
    float top = info.top;
    
    c = (Character){.c_str = "À"};
    info = character_getGlyphInfo_(c, coqfont);
    printCharInfo_(info);
    top = 0.5f*(info.top + top);
    float topExtra = info.top > coqfont->fontHeight ? info.top - coqfont->fontHeight : 0;
    
    printf("🐷 Testing font family %s, display %s, name %s : capH %.1f, asc %.1f, desc %.1f, x %.1f.\n", 
           [[font familyName] UTF8String], [[font displayName] UTF8String], [[font fontName] UTF8String], 
        [font capHeight], [font ascender], [font descender], [font xHeight]);
    printf("    └-> fullHeight %.1f, solidHeight %.1f, fontHeight %.1f, bottom %.1f, top %.1f.\n",
           coqfont->fullHeight, coqfont->solidHeight, fontHeight, bottom, top);
    printf(" └->  {\"%s\",     { %d, %d, %d, %d, %d, },},\n",
           [[font fontName] UTF8String], (int)ceilf(fontHeight),
           (int)floor(bottom), (int)ceilf(top), (int)ceilf(bottomExtra), (int)ceilf(topExtra) 
    );
}


void CoqFont_test_printAvailableFonts(void) {
#if TARGET_OS_OSX == 1
    for(NSString *family in [[NSFontManager sharedFontManager] availableFonts]) {
        printf("Font : %s.\n", [family UTF8String]);
    }
#else
    for(NSString *family in [[UIFont sharedFontManager] familyNames]) {
        printf("Font : %s.\n", [family UTF8String]);
    }
#endif
}

PixelBGRAArray* Pixels_engine_test_createArrayFromString_(const char* c_str, CoqFont const* coqFont) 
{
    NSDictionary* attributes = (__bridge NSDictionary*)coqFont->_nsdict_attributes;
    Font* font = [attributes valueForKey:NSFontAttributeName];
    NSString* string = [NSString stringWithUTF8String:c_str];
    if(stringUTF8_isSingleEmoji(c_str)) {
        font = [Font systemFontOfSize:[font pointSize]];
    }
    CTFontRef font_ct = (__bridge CTFontRef)font;
    CGSize const string_size = [string sizeWithAttributes:attributes];
    
    NSUInteger string_length = [string length];
    unichar characters[string_length + 1];
    [string getCharacters:characters range:NSMakeRange(0, string_length)];
    characters[string_length] = 0;
    CGGlyph glyphs[string_length];
    CTFontGetGlyphsForCharacters(font_ct, characters, glyphs, string_length);
//    NSRect glyphRect = [font boundingRectForCGGlyph:glyphs[0]];
    // Décalage en x du centre du glyph.
//    double deltaX = glyphRect.origin.x - 0.5*string_size.width + 0.5*glyphRect.size.width;
    
    // Pixels et Context CoreGraphics pour dessiner la string.
    size_t const width = string_size.width + coqFont->solidHeight;
    size_t const height = coqFont->fullHeight;
    printdebug("String width %zu, height %zu.", width, height);
    PixelBGRAArray* pa = PixelBGRAArray_createEmpty(width, height);
    pa->solidWidth = string_size.width;
    pa->solidHeight = coqFont->solidHeight;
    
    CGPoint const drawPoint = CGPointMake(0.5*(double)coqFont->solidHeight, 
//        coqFont->_drawDeltaY
         + coqFont->deltaY
    );
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    NSUInteger const bytesPerRow = width * 4;
    NSUInteger const bitsPerComponent = 8;
    CGContextRef context = CGBitmapContextCreate(pa->pixels, pa->width, pa->height,
                    bitsPerComponent, bytesPerRow, colorSpace,
                    kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);
    CGContextSetTextDrawingMode(context, kCGTextFillStroke);
    
    // 3. Dessiner la string dans le context
    //    CTFontDrawGlyphs(font_ct, glyphs, &drawPoint, 1, context);
    // (set context CoreGraphics dans context NSGraphics pour dessiner la NSString.)
    @autoreleasepool {
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithCGContext:context flipped:false]];
        [string drawAtPoint:drawPoint withAttributes:attributes];
        [NSGraphicsContext setCurrentContext:nil];
        [NSGraphicsContext restoreGraphicsState];
    }
    // 4. Relâcher le context CoreGraphic
    CGContextRelease(context);
    
    return pa;
}
