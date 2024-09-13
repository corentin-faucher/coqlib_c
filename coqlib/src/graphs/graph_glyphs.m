//
//  graph_glyphs.c
//  Texture contenant les glyphs d'une police de caractères.
//
//  Created by Corentin Faucher on 2024-09-11.
//

#include "graph_glyphs.h"

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "../utils/util_base.h"
#include "../coq_map.h"

#include "graph__apple.h"

#pragma mark - Map de glyphs (pour dessiner les strings)

void fontglyphmapcustomchars_copyFrom_(FontGlyphMapCustomChars* cc, FontGlyphMapCustomChars* ref) {
    cc->count = ref->count;
    cc->texture = ref->texture;
    size_t charsSize = cc->count * sizeof(Character);
    cc->chars = coq_malloc(charsSize);
    memcpy(cc->chars, ref->chars, charsSize);
    size_t rectsSize = cc->count * sizeof(Rectangle);
    cc->uvRects = coq_malloc(rectsSize);
    memcpy(cc->uvRects, ref->uvRects, rectsSize);
}
void fontglyphmapcustomchars_deinit_(FontGlyphMapCustomChars* cc) {
    if(cc->chars)
        coq_free(cc->chars);
    cc->chars = NULL;
    if(cc->uvRects)
        coq_free(cc->uvRects);
    cc->uvRects = NULL;
    cc->count = 0;
    cc->texture = NULL;
}

/// Une texture avec les glyphs/caractères d'une font.
typedef struct FontGlyphMap {
    Texture       tex;  // "upcasting"
    
    /// Info de la Font macOS/iOS.
    NSDictionary* attributes;
    
    // Info général des glyph
    /// Hauteur d'un glyph en pixels : ascender + |descender| + 2*marge (constant pour simplifier la glyph map...)
    size_t const glyphHeight;
    /// Hauteur de référence pour un glyph 
    /// -> hauteur "occupé" (sans extra des accents) : capHeihgt + descender.
    /// (Les autres dimensions sont normalisé par rapport à `solidHeight`, 
    ///  on a `solidHeight < glyphHeight`.)
    float const solidHeight;
    /// Décalage du center en y dû à la difference entre solidHeight et glyphHeight:
    ///  deltaY = 0.5*(ascender - capHeight).
    float const deltaY;
    
    size_t     currentTexX;
    size_t     currentTexY;
    StringMap* glyphInfos;
    GlyphInfo  _defaultGlyph;
    
    FontGlyphMapCustomChars customChars;
} FontGlyphMap;

#if TARGET_OS_OSX == 1
typedef NSFont Font;
#else
typedef UIFont Font;
#endif

#pragma mark - Dessin des glyphs avec CoreGraphics / Metal
GlyphInfo  fontglyphmap_drawCharacter_(FontGlyphMap* const gm, Character const c) {
    if(gm->currentTexY + gm->glyphHeight > gm->tex.height) {
        printerror("Cannot add glyph of %s. Glyphmap full.", c.c_str);
        return (GlyphInfo) {};
    }
    Font* font = [gm->attributes valueForKey:NSFontAttributeName];
    bool const isEmoji = c.c_str[2];
    NSString* string = [NSString stringWithUTF8String:c.c_str];
    if(isEmoji) {
        font = [Font systemFontOfSize:[font pointSize]];
    }
    CTFontRef font_ct = (__bridge CTFontRef)font;
    CGSize const string_size = [string sizeWithAttributes:gm->attributes];
    NSUInteger string_length = [string length];
    unichar characters[string_length + 1];
    [string getCharacters:characters range:NSMakeRange(0, string_length)];
    characters[string_length] = 0;
    CGGlyph glyphs[string_length];
    CTFontGetGlyphsForCharacters(font_ct, characters, glyphs, string_length);
    NSRect glyphRect = [font boundingRectForCGGlyph:glyphs[0]];
    double deltaX = glyphRect.origin.x - 0.5*string_size.width + 0.5*glyphRect.size.width;
    
    // Pixels et Context CoreGraphics pour dessiner la string.
    size_t const extra_margin = (gm->tex.flags & tex_flag_nearest) ? 1 : 2;
    size_t const glyphWidth_pixels = glyphRect.size.width + 2*extra_margin;
    size_t const pixels_count = glyphWidth_pixels * gm->glyphHeight;
    PixelBGRA* pixelsBGRA = coq_calloc(pixels_count, sizeof(PixelBGRA));
    CGPoint const drawPoint = CGPointMake( -glyphRect.origin.x + extra_margin, //0);
                                    extra_margin); // extra_margin
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    NSUInteger const bytesPerRow = glyphWidth_pixels * 4;
    NSUInteger const bitsPerComponent = 8;
    CGContextRef context = CGBitmapContextCreate(pixelsBGRA, glyphWidth_pixels, gm->glyphHeight,
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
        [string drawAtPoint:drawPoint withAttributes:gm->attributes];
        [NSGraphicsContext setCurrentContext:nil];
        [NSGraphicsContext restoreGraphicsState];
    }
    // 4. Relâcher le context CoreGraphic
    CGContextRelease(context);
    
    // 5. Copier dans la texture Metal
//    pixelsBGRA[0] =                          pixelbgra_red;
//    pixelsBGRA[glyphWidth_pixels-1] =        pixelbgra_white;
//    pixelsBGRA[glyphWidth_pixels*(gm->glyphHeight-1)] = pixelbgra_yellow;
//    pixelsBGRA[pixels_count-1] = pixelbgra_black;
    // Vérifier s'il reste assez de place sur la ligne.
    if(gm->currentTexX + glyphWidth_pixels > gm->tex.width) {
        gm->currentTexX = 0;
        gm->currentTexY += gm->glyphHeight;
        if(gm->currentTexY + gm->glyphHeight > gm->tex.height) {
            printerror("Cannot add glyph of %s. Glyphmap full.", c.c_str);
            coq_free(pixelsBGRA);
            return (GlyphInfo){};
        }
        if(gm->currentTexY + gm->glyphHeight > 0.65*gm->tex.height) 
            printwarning("Glyph map texture seems too small... Texture size is %d for font of height %zu.",
                (int)gm->tex.height, gm->glyphHeight);
    }
    Rectangle uvRect = {
        (float)gm->currentTexX   / gm->tex.width, (float)gm->currentTexY / gm->tex.height,
        (float)glyphWidth_pixels / gm->tex.width, (float)gm->glyphHeight / gm->tex.height };
    texture_engine_writePixelsToRegion(&gm->tex, pixelsBGRA, 
        (RectangleUint){ (uint32_t)gm->currentTexX, (uint32_t)gm->currentTexY,
                         (uint32_t)glyphWidth_pixels, (uint32_t)gm->glyphHeight });
    // Placer x tex coord sur next.
    gm->currentTexX += glyphWidth_pixels;
    
    // 6. Libérer les pixels
    coq_free(pixelsBGRA);
    // 7. Infos du glyph dans la texture (glyphHeight_pixels est la dimension de référence)
    return (GlyphInfo) {
        .uvRect = uvRect,
        .relGlyphX = deltaX / gm->solidHeight,
        .relGlyphWidth = glyphWidth_pixels / gm->solidHeight, 
        .relSolidWidth = string_size.width / gm->solidHeight,
    };
}
GlyphInfo  fontglyphmap_tryToDrawAsCustomCharacter_(FontGlyphMap* const gm, Character const c) {
    if(!gm->customChars.texture) { return (GlyphInfo){}; }
    // Trouver si c'est un custom char.
    uint32_t charIndex = 0;
    for(; charIndex < gm->customChars.count; charIndex++) {
        if(gm->customChars.chars[charIndex].c_data8 == c.c_data8)
            break;
    }
    if(charIndex >= gm->customChars.count) return (GlyphInfo) {};
    
    // Vérifier les dimensions de la région à copier.
    if(gm->currentTexY + gm->glyphHeight > gm->tex.height) {
        printerror("Cannot add texture glyph. Glyphmap full.");
        return (GlyphInfo) {};
    }
    RectangleUint srcRegion = texture_pixelRegionFromUVrect(
        gm->customChars.texture, gm->customChars.uvRects[charIndex]
    );
    if(srcRegion.h > gm->glyphHeight) {
        printwarning("H > glyphHeight."); srcRegion.h = (uint32_t)gm->glyphHeight;
    }
    if(srcRegion.w > 3*gm->glyphHeight) {
        printwarning("W > 3*glyphHeight."); srcRegion.w = (uint32_t)(3*gm->glyphHeight);
    }
    size_t const extra_margin = (gm->tex.flags & tex_flag_nearest) ? 1 : 2;
    size_t const dstWidth = srcRegion.w + 2*extra_margin;
    
     // Vérifier s'il reste assez de place sur la ligne.
    if(gm->currentTexX + dstWidth > gm->tex.width) {
        gm->currentTexX = 0;
        gm->currentTexY += gm->glyphHeight;
        if(gm->currentTexY + gm->glyphHeight > gm->tex.height) {
            printerror("Cannot add texture glyph. Glyphmap full.");
            return (GlyphInfo){};
        }
        if(gm->currentTexY + gm->glyphHeight > 0.65*gm->tex.height) 
            printwarning("Glyph map texture seems too small... Texture size is %d for font of height %zu.",
                (int)gm->tex.height, gm->glyphHeight);
    }
    
    // Copier l'image pour ce char.
    UintPair dstOrigin = {(uint32_t)gm->currentTexX + (uint32_t)extra_margin, 
                          (uint32_t)gm->currentTexY + (uint32_t)(gm->glyphHeight - srcRegion.h)/2};
    texture_engine_copyRegionTo(gm->customChars.texture, &gm->tex, srcRegion, dstOrigin);
    
    // Region du dessin (avec marge)
    RectangleUint glyphRegion = {
        (uint32_t)gm->currentTexX, (uint32_t)gm->currentTexY,
        (uint32_t)dstWidth,        (uint32_t)gm->glyphHeight,
    };
    Rectangle glyphUVrect = texture_UVrectFromPixelRegion(&gm->tex, glyphRegion);
    // Placer x tex coord sur next.
    gm->currentTexX += dstWidth;
    
    // 7. Infos du glyph dans la texture (glyphHeight_pixels est la dimension de référence)
    return (GlyphInfo) {
        .uvRect = glyphUVrect,
        .relGlyphX = 0,
        .relGlyphWidth = dstWidth / gm->solidHeight,    // (avec marge)
        .relSolidWidth = srcRegion.w / gm->solidHeight, // (sans marge)
    };
}

#pragma mark - Constructor / destructor
// (sous-fonction de `FontGlyphMap_create`)
size_t         textureWidthFromRefHeight_(float refHeight) {
    if(refHeight < 25) return 256;
    if(refHeight < 50) return 512;
    if(refHeight < 100) return 1024;
    return 2048;
}
FontGlyphMap*  FontGlyphMap_create(const char* const fontNameOpt, double fontSize, 
                    size_t textureWidthOpt, bool const nearest, 
                    FontGlyphMapCustomChars* customCharsOpt)
{
    // 0. Init font/attributes
    Font* font;
    if(fontSize < 12) { printwarning("Font too small."); fontSize = 12; }
    if(fontSize > 200) { printwarning("Font too big."); fontSize = 200; }
    if(fontNameOpt)
        font = [Font fontWithName:[NSString stringWithUTF8String:fontNameOpt]
                                 size:fontSize];
    else
        font = [Font systemFontOfSize:fontSize];
    if(font == nil) { 
        printwarning("Font %s not found, taking system font.", fontNameOpt);
        font = [Font systemFontOfSize:fontSize];
    }
    NSMutableParagraphStyle* paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    paragraphStyle.alignment = NSTextAlignmentCenter;
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
    NSDictionary* attributes = [NSMutableDictionary 
        dictionaryWithObjects:@[font,  paragraphStyle,                [NSColor blackColor]         ]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName]];
        
    // 1. Création de glyphmap
    FontGlyphMap* gm = coq_callocTyped(FontGlyphMap);
    gm->attributes = attributes;
    gm->glyphInfos = Map_create(100, sizeof(GlyphInfo));
    
    // 2. Glyph sizes
    size_t const extra_margin = nearest ? 1 : 2;
    size_initConst(&gm->glyphHeight, 
        ceil([font ascender] - [font descender]) + 2*extra_margin
    );
//    printdebug("Init %s, ascender %f, descender %f, capheight %f.", fontNameOpt, [font ascender], [font descender], [font capHeight]);
    
    float_initConst(&gm->solidHeight, [font capHeight] - [font descender]);
    float_initConst(&gm->deltaY, 0.5*([font ascender] - [font capHeight]));
    // Custom chars
    if(customCharsOpt) {
        fontglyphmapcustomchars_copyFrom_(&gm->customChars, customCharsOpt);
    }
    
    // 3. Init texture
    if(textureWidthOpt < 5*gm->solidHeight) {
        textureWidthOpt = textureWidthFromRefHeight_(gm->solidHeight);
    }
    texture_initEmpty_(&gm->tex);
    MTLTextureDescriptor* descr = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                        width:textureWidthOpt height:textureWidthOpt mipmapped:NO];
    id<MTLTexture> mtlTex = [CoqGraph_getMTLDevice() newTextureWithDescriptor:descr];
    gm->tex.mtlTex_cptr = CFBridgingRetain(mtlTex);
    gm->tex.flags |= tex_flag_shared|(nearest? tex_flag_nearest : 0);
    gm->tex.width = textureWidthOpt;
    gm->tex.height = textureWidthOpt;
    
    // 4. Prédessiner au moins le ? (init avec plus de glyph ? genre abcd...)
    gm->_defaultGlyph = fontglyphmap_drawCharacter_(gm, spchar_questionMark);
    map_put(gm->glyphInfos, spchar_questionMark.c_str, &gm->_defaultGlyph);
    
    return gm;
}
void           fontglyphmap_deinit(FontGlyphMap* gm) {
    texture_deinit_(&gm->tex);
    map_destroyAndNull(&gm->glyphInfos, NULL);
    gm->attributes = nil;
    fontglyphmapcustomchars_deinit_(&gm->customChars);
}
void           fontglyphmapref_releaseAndNull(FontGlyphMap** const fgmOptRef) {
    FontGlyphMap* const fgm = *fgmOptRef;
    if(!fgm) return;
    *fgmOptRef = NULL;
    
    fontglyphmap_deinit(fgm);
    coq_free(fgm);
}


#pragma mark - Getters...
GlyphInfo      fontglyphmap_getGlyphInfoOfChar(FontGlyphMap* gm, Character c) {
    GlyphInfo* info = (GlyphInfo*)map_valueRefOptOfKey(gm->glyphInfos, c.c_str);
    if(info) return *info;
    // N'existe pas encore, dessiner et enregistrer
    GlyphInfo newGlyph = fontglyphmap_tryToDrawAsCustomCharacter_(gm, c);
    if(newGlyph.relSolidWidth == 0.f)
        newGlyph = fontglyphmap_drawCharacter_(gm, c);
    // Erreur ?
    if(newGlyph.relSolidWidth == 0.f) return gm->_defaultGlyph;
    map_put(gm->glyphInfos, c.c_str, &newGlyph);
    return newGlyph;
}
Texture*       fontglyphmap_getTexture(FontGlyphMap* gm) {
    return &gm->tex;
}
float          fontglyphmap_getRelY(FontGlyphMap* gm) {
    return gm->deltaY / gm->solidHeight;
}
float          fontglyphmap_getRelHeight(FontGlyphMap* gm) {
    return gm->glyphHeight / gm->solidHeight;
}
