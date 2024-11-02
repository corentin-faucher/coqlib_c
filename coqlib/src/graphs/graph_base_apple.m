//
//  graph_base_apple.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-31.
//
#include "graph_base.h"

//#import <Foundation/Foundation.h>
//#import <CoreGraphics/CoreGraphics.h>
#import <AppKit/AppKit.h>

#include "../utils/util_base.h"
#include "../utils/util_language.h"

#pragma mark - Dessin de png
PixelBGRAArray* Pixels_engine_createOptFromPng(const char* pngPathOpt) {
    if(!pngPathOpt) return NULL;
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:pngPathOpt]];
    if(image == nil) {
        printerror("Cannot load image at %s.", pngPathOpt);
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
    pa->width = width; pa->height = height; pa->solidWidth = width;
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

#pragma mark - Dessin de fonts/glyphs -
#if TARGET_OS_OSX == 1
typedef NSFont Font;
#else
typedef UIFont Font;
#endif
void coqfont_engine_init(coq_Font const*const cf, const char *const fontNameOpt, double fontSize, bool const nearest) {
    // Checks...
    if(cf->_nsdict_attributes) { printerror("Font already init."); return; }
    if(!fontSize) fontSize = 32; // (Default)
    if(fontSize < 12) { printwarning("Font too small."); fontSize = 12; }
    if(fontSize > 200) { printwarning("Font too big.");  fontSize = 200; }
    // Set font
    Font* font;
    if(fontNameOpt)
        font = [Font fontWithName:[NSString stringWithUTF8String:fontNameOpt]
                                 size:fontSize];
    else
        font = [Font systemFontOfSize:fontSize];
    if(font == nil) { 
        printwarning("Font %s not found, taking system font.", fontNameOpt);
        font = [Font systemFontOfSize:fontSize];
    }
    // Style... Ajouter la couleur comme paramètre d'init ?
    NSMutableParagraphStyle* paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    paragraphStyle.alignment = NSTextAlignmentCenter;
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
    NSDictionary* attributes = [NSMutableDictionary 
        dictionaryWithObjects:@[font,  paragraphStyle,                [NSColor blackColor]         ]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName]];
    // Hauteur de Ref (capHeight hauteur max standard de la plupart des lettres)
    float const solidHeight = [font capHeight] - [font descender];
    // Dimensions
    LanguageFontInfo const *info = LanguageFont_getFontInfoOpt(fontNameOpt);
    double const extraDeltaY = -solidHeight * 0.03 + (info ? info->deltaYAdj*solidHeight : 0.f);
    double const extraDesc = info ? info->extraDesc*solidHeight : 0.f;
    double const descender = -[font descender] + extraDesc;
    size_t const extra_margin = nearest ? 1 : 2;
    // (Ascender vrai hauteur max avec fioriture qui dépassent, e.g. le haut de `f`.)
    size_t const glyphHeight = ceil([font ascender] + descender) + 2*extra_margin;
    // Décalage en y pour centrer les caractères
    double const deltaY = 0.5*([font ascender] - [font capHeight] - extraDesc) + extraDeltaY;
    printdebug("Info of font %s: capH %f, asc %f, desc %f ", fontNameOpt,  [font capHeight], [font ascender], [font descender]);
    
    // Save data
    *(const void**)&cf->_nsdict_attributes = CFBridgingRetain(attributes);
    *(bool*)&cf->nearest = nearest;
    size_initConst(&cf->glyphHeight, glyphHeight);
    float_initConst(&cf->solidHeight, solidHeight);
    float_initConst(&cf->deltaY, deltaY);
    float_initConst(&cf->drawDeltaY, extraDesc);
}
void coqfont_engine_deinit(coq_Font const *const cf) {
    CFRelease(cf->_nsdict_attributes);
    *(const void**)&cf->_nsdict_attributes = NULL;
}

PixelBGRAArray* Pixels_engine_createArrayFromCharacter(Character const c, coq_Font const coqFont) 
{
    NSDictionary* attributes = (__bridge NSDictionary*)coqFont._nsdict_attributes;
    Font* font = [attributes valueForKey:NSFontAttributeName];
    NSString* string = [NSString stringWithUTF8String:c.c_str];
    if(stringUTF8_isSingleEmoji(c.c_str)) {
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
    NSRect glyphRect = [font boundingRectForCGGlyph:glyphs[0]];
    // Décalage en x du centre du glyph.
    double deltaX = glyphRect.origin.x - 0.5*string_size.width + 0.5*glyphRect.size.width;
    
    // Pixels et Context CoreGraphics pour dessiner la string.
    size_t const extra_margin = coqFont.nearest ? 1 : 2;
    size_t const glyphWidth_pixels = glyphRect.size.width + 2*extra_margin;
    size_t const pixels_count = glyphWidth_pixels * coqFont.glyphHeight;
    PixelBGRAArray* pa = coq_callocArray(PixelBGRAArray, PixelBGRA, pixels_count);
    pa->width = glyphWidth_pixels;
    pa->height = coqFont.glyphHeight;
    pa->deltaX = deltaX;
    pa->solidWidth = string_size.width;
    CGPoint const drawPoint = CGPointMake( -glyphRect.origin.x + extra_margin, //0);
                                          extra_margin + coqFont.drawDeltaY); // extra_margin
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    NSUInteger const bytesPerRow = glyphWidth_pixels * 4;
    NSUInteger const bitsPerComponent = 8;
    CGContextRef context = CGBitmapContextCreate(pa->pixels, 
                    glyphWidth_pixels, coqFont.glyphHeight,
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
PixelBGRAArray* Pixels_engine_createArrayFromString(const char* c_str, coq_Font const coqFont) 
{
    NSDictionary* attributes = (__bridge NSDictionary*)coqFont._nsdict_attributes;
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
    size_t const extra_margin = coqFont.nearest ? 1 : 2;
    size_t const width = string_size.width + coqFont.glyphHeight;
    size_t const height = coqFont.glyphHeight;
    PixelBGRAArray* pa = coq_callocArray(PixelBGRAArray, PixelBGRA, width * height);
    pa->width = width;
    pa->height = height;
    pa->deltaX = 0;
    pa->solidWidth = string_size.width;
    
    CGPoint const drawPoint = CGPointMake(0.5*(double)coqFont.glyphHeight, //0);
                                          extra_margin + coqFont.drawDeltaY); // extra_margin
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
