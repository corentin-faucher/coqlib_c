//
//  graph_base_apple.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-31.
//
#include "graph_base.h"
#include "../utils/util_base.h"
#include "../utils/util_string.h"
#include "coq_map.h"

//#import <Foundation/Foundation.h>
//#import <CoreGraphics/CoreGraphics.h>
#import <AppKit/AppKit.h>

// MARK: - Dessin de png
PixelBGRAArray* Pixels_engine_createFromPngOpt(char const*const pngPathOpt, bool const showError) 
{
    if(!pngPathOpt) return NULL;
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:pngPathOpt]];
    if(image == nil) {
        if(showError) printerror("Cannot load image at %s.", pngPathOpt);
        return NULL;
    }
    NSGraphicsContext *context = [NSGraphicsContext currentContext];
    NSRect rect = { .origin = {0, 0}, .size = image.size };
    CGRect cgRect = { .origin = {0, 0}, .size = image.size };
    // Création de l'image et de l'array de pixels.
    CGImageRef cgImage = [image CGImageForProposedRect:&rect context:context hints:nil];
    size_t width =  CGImageGetWidth(cgImage);
    size_t height = CGImageGetHeight(cgImage);
    PixelBGRAArray* pa = coq_callocArray(PixelBGRAArray, PixelBGRA, width * height);
    size_initConst(&pa->width, width);
    size_initConst(&pa->height, height);
    pa->solidWidth = width;
    pa->solidHeight = height;
    // Copier l'image dans l'array de pixels.
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * width;
    NSUInteger bitsPerComponent = 8;
    CGContextRef cgcontext = CGBitmapContextCreate(pa->pixels, width, height,
                                                 bitsPerComponent, bytesPerRow, colorSpace,
                                                 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    CGContextDrawImage(cgcontext, cgRect, cgImage);
    CGContextRelease(cgcontext);
    return pa;
}

// MARK: - Dessin de fonts/glyphs -
#if TARGET_OS_OSX == 1
typedef NSFont Font;
#else
typedef UIFont Font;
#endif
typedef struct coq_Font {
//    union {
        // Version Apple -> Dictionnaire d'attributes (dont la NSFont/UIFont)
        void const*const _nsdict_attributes;
        // Version Freetype -> FT_Face
//        void const*const _ft_face;
//    };
    bool const   nearest; // Pixelisé ou smooth ?
    /// Hauteur total requise pour dessiner les glyphes en pixels *avec* les fioritures qui depassent,
    ///  i.e. `ascender - descender`.
    size_t const fullHeight;
    /// Hauteur de la hitbox en pixels sans les fioritures, i.e. `capHeight - descender`
    /// (et on a `solidHeight < glyphHeight`).
    /// ** -> Les autres dimensions sont normalisé par rapport à `solidHeight` **,
    float const  solidHeight;
    float const  deltaY;  // Décalage pour centre les glyphes, i.e. (ascender - capHeight)/2.
    float const  _drawDeltaY; // ("Privé", décalage pour dessiner...)
    // float const  _maxDesc;    // ("Privé" distance positive entre bas et baseline, un peu plus que descender)
} CoqFont;
// Les infos supplémentaire pour les fonts à "ajuster"...

Vector2 CoqFont_getExtraYMargins_(char const* fontName);
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
    double const descender =  -[font descender];
    double const solidHeight = [font capHeight] + descender;
    double const glyphHeight = [font ascender] + descender;
    Vector2 const extraYMargins = CoqFont_getExtraYMargins_(init.nameOpt);
    double const topMargin = FONT_extra_margin(init.nearest) + extraYMargins.x;
    double const bottomMargin = FONT_extra_margin(init.nearest) + extraYMargins.y;
    size_t const fullHeight = ceil(glyphHeight + topMargin + bottomMargin);
    
    // Décalage en y pour dessiner les caractères (a priori le point y0 est sur descender)
    double const drawDeltaY = 
        0.5*glyphHeight - 0.5*[font xHeight] - descender + bottomMargin;
    // Décalage pour recentrer la texture.
    double const deltaY = 0.5*(topMargin - bottomMargin);
//    printdebug("Font fullHeight %zu, solidHeight %f, top %f, bottom %f.",
//               fullHeight, solidHeight, topMargin, bottomMargin);
//    printdebug("Info of font %s: capH %f, asc %f, desc %f, x %f.", init.nameOpt,  [font capHeight], [font ascender], [font descender], [font xHeight]);
//    printdebug("drawDy %f, Dy %f.", drawDeltaY, deltaY);
    // Save data
    CoqFont*const cf = coq_callocTyped(CoqFont);
    *(const void**)&cf->_nsdict_attributes = CFBridgingRetain(attributes);
    bool_initConst(&cf->nearest, init.nearest);
    size_initConst(&cf->fullHeight, fullHeight);
    float_initConst(&cf->solidHeight, solidHeight);
    float_initConst(&cf->deltaY, deltaY);
    float_initConst(&cf->_drawDeltaY, drawDeltaY);
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
        .relFullHeihgt = cf->fullHeight / cf->solidHeight,
        .relDeltaY = cf->deltaY / cf->solidHeight,
        .nearest = cf->nearest,
    };
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
    PixelBGRAArray* pa = coq_callocArray(PixelBGRAArray, PixelBGRA, width * height);
    size_initConst(&pa->width, width);
    size_initConst(&pa->height, height);
    pa->solidWidth = string_size.width;
    pa->solidHeight = coqFont->solidHeight;
    
    CGPoint const drawPoint = CGPointMake(0.5*(double)coqFont->solidHeight, 
        coqFont->_drawDeltaY
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

PixelBGRAArray* Pixels_engine_createArrayFromCharacter(Character const c, CoqFont const* coqFont) 
{
    NSDictionary* attributes = (__bridge NSDictionary*)coqFont->_nsdict_attributes;
    Font* font = [attributes valueForKey:NSFontAttributeName];
    NSString* string = [NSString stringWithUTF8String:c.c_str];
    if(stringUTF8_isSingleEmoji(c.c_str)) {
        font = [Font systemFontOfSize:[font pointSize]];
    }
    CTFontRef const font_ct = (__bridge CTFontRef)font;
    CGSize const string_size = [string sizeWithAttributes:attributes];
    NSUInteger const string_length = [string length];
    unichar characters[string_length + 1];
    [string getCharacters:characters range:NSMakeRange(0, string_length)];
    characters[string_length] = 0;
    CGGlyph glyphs[string_length];
    CTFontGetGlyphsForCharacters(font_ct, characters, glyphs, string_length);
    NSRect glyphRect = [font boundingRectForCGGlyph:glyphs[0]];
    // Décalage en x du centre du glyph.
    double const deltaX = glyphRect.origin.x - 0.5*string_size.width + 0.5*glyphRect.size.width;
    // Pixels et Context CoreGraphics pour dessiner la string.
    size_t const extra_margin = FONT_extra_margin(coqFont->nearest);
    // Array de pixels (Pixel Array)
    size_t const pa_width = glyphRect.size.width + 2*extra_margin;
    size_t const pa_height = coqFont->fullHeight;
    size_t const pa_count = pa_width * pa_height;
    PixelBGRAArray*const pa = coq_callocArray(PixelBGRAArray, PixelBGRA, pa_count);
    size_initConst(&pa->width, pa_width);
    size_initConst(&pa->height, pa_height);
    pa->deltaX = deltaX;
    pa->solidWidth = string_size.width;
    pa->solidHeight = coqFont->solidHeight;
    
    CGPoint const drawPoint = CGPointMake(-glyphRect.origin.x + extra_margin,
                                          coqFont->_drawDeltaY
    );
    CGColorSpaceRef const colorSpace = CGColorSpaceCreateDeviceRGB();
    NSUInteger const bytesPerRow = pa_width * sizeof(PixelBGRA);
    NSUInteger const bitsPerComponent = 8;
    CGContextRef const context = CGBitmapContextCreate(pa->pixels, pa_width, pa_height,
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

typedef struct {
    char    name[40];
    float   topMargin, bottomMargin;
} FontExtraInfo_;
static const FontExtraInfo_ fontInfo_arr_[] = {
    {"American Typewriter", 0.05,   },
//    {"Chalkboard SE", },
    {"Chalkduster", 0.15, 0.05 },
    {"Comic Sans MS", 0.07, },
    {"Courier", 0.10, },
    {"Futura", 0.13, },
    {"Helvetica", 0.17, },
    {"Snell Roundhand", .1, },
    {"Times New Roman", 0.13, },
    {"Verdana", 0.13, },
    {"PilGi",   0.25,   },
    // TODO: continuer...
//    {"Nanum Gothic", "NanumGothic",},
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
Vector2 CoqFont_getExtraYMargins_(char const*const fontName) {
    if(!fontName) return vector2_zeros;
    static StringMap* fontInfoOfNamed = NULL;
    // Init de la map.
    if(fontInfoOfNamed == NULL) {
        fontInfoOfNamed = Map_create(40, sizeof(FontExtraInfo_));
        size_t const count = sizeof(fontInfo_arr_)/sizeof(FontExtraInfo_);
        const FontExtraInfo_* p =   &fontInfo_arr_[0];
        const FontExtraInfo_* end = &fontInfo_arr_[count];
        while(p < end) {
            map_put(fontInfoOfNamed, p->name, p);
            p++;
        }
    }
    
    FontExtraInfo_ const* extra = (const FontExtraInfo_*)map_valueRefOptOfKey(fontInfoOfNamed, fontName);
    if(extra) return (Vector2) {{ extra->topMargin, extra->bottomMargin, }};
    return vector2_zeros;
}

