//
//  metal_texture_helper.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-05.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "graph__metal.h"
#include "graph_texture_private.h"
#include "math_base.h"
#include "../utils/util_base.h"
#include "../utils/util_file.h"
#include "coq_map.h"
static uint32_t pixels_transparent_[4] = {
    0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
};
static uint32_t pixels_white_[4] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};
static void const* mtltexture_transparent_ = NULL;
static void const* mtltexture_white_ = NULL;
static MTKTextureLoader*    MTLtextureLoader_ = nil;
// MARK: - Init Metal --------------------
void Texture_metal_init_(void) {
    id<MTLDevice> device = CoqGraph_metal_device;
    if(device == nil) {
        printerror("MTL_device_ not set.");
        return;
    }
    if(MTLtextureLoader_ != nil) {
        printerror("MTLTexture already init.");
        return;
    }
    // 1. Texture loader et font.
    MTLtextureLoader_ = [[MTKTextureLoader alloc] initWithDevice:device];
//    Font_current_ =     [Font systemFontOfSize:Font_currentSize_];
//    Font_currentMini_ = [Font systemFontOfSize:Font_miniSize_];
//    #if TARGET_OS_OSX == 1
//    Font_currentSpreading_ = [NSValue valueWithSize:CGSizeMake(1.3f, 1.0f)];
//    NSColor* black = [NSColor blackColor];
//    #else
//    Font_currentSpreading_ = [NSValue valueWithCGSize:CGSizeMake(1.3f, 1.0f)];
//    UIColor* black = [UIColor blackColor];
//    #endif
//    NSMutableParagraphStyle* paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
//    paragraphStyle.alignment = NSTextAlignmentCenter;
//    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
//    Font_paragraphStyle_ = paragraphStyle;
//    Font_currentAttributes_ = [NSMutableDictionary
//        dictionaryWithObjects:@[Font_current_, Font_paragraphStyle_,  black,           Font_currentSpreading_]
//        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
//    Font_currentAttributesMini_ = [NSDictionary
//        dictionaryWithObjects:@[Font_currentMini_, Font_paragraphStyle_, black,        Font_currentSpreading_]
//        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];

    // 2. Texture par défaut
    MTLTextureDescriptor* descr = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:CoqGraph_metal_pixelFormat
                                                        width:2 height:2 mipmapped:NO];
    id<MTLTexture> mtlTexTmp = [CoqGraph_metal_device newTextureWithDescriptor:descr];
    [mtlTexTmp replaceRegion:MTLRegionMake2D(0, 0, 2, 2) mipmapLevel:0
             withBytes:pixels_transparent_ bytesPerRow:2 * 4];
    mtltexture_transparent_ = CFBridgingRetain(mtlTexTmp);

    descr = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:CoqGraph_metal_pixelFormat width:2 height:2 mipmapped:NO];
    mtlTexTmp = [CoqGraph_metal_device newTextureWithDescriptor:descr];
    [mtlTexTmp replaceRegion:MTLRegionMake2D(0, 0, 2, 2) mipmapLevel:0 withBytes:pixels_white_ bytesPerRow:2*4];
    mtltexture_white_ = CFBridgingRetain(mtlTexTmp);
    mtlTexTmp = nil;
}
void Texture_metal_deinit_(void) {
    MTLtextureLoader_ = nil;
    if(mtltexture_transparent_) {
        CFRelease(mtltexture_transparent_);
        mtltexture_transparent_ = NULL;
    }
    if(mtltexture_white_) {
        CFRelease(mtltexture_white_);
        mtltexture_white_ = NULL;
    }
}
void Texture_metal_setFrameBufferToMTLTexture(uint32_t index, id<MTLTexture> mtlTex) {
    if(index > 10) { printerror("Bad index."); return; }
    if(Texture_frameBuffers[index] == NULL) {
        Texture_frameBuffers[index] = coq_callocTyped(Texture);
        *Texture_frameBuffers[index] = (Texture) {
            .dims = {
                .m = 1,  .n = 1,
                .Du = 1, .Dv = 1,
                .width = (float)[mtlTex width], .height = (float)[mtlTex height],
                .tileRatio = (float)[mtlTex width] / (float)[mtlTex height],
            },
            tex_flag_shared|tex_flag_fullyDrawn_|tex_flag_static_, ChronosRender.render_elapsedMS,
        };
    } else {
        Texture_frameBuffers[index]->dims.sizes = (Vector2){(float)[mtlTex width], (float)[mtlTex height]};
        Texture_frameBuffers[index]->dims.tileRatio =  (float)[mtlTex width] / (float)[mtlTex height];
        CFRelease(Texture_frameBuffers[index]->mtlTex_cptr);
    }
    Texture_frameBuffers[index]->mtlTex_cptr = CFBridgingRetain(mtlTex);
}
id<MTLTexture> Texture_metal_defaultWhiteMTLTexture(void) {
    return (__bridge id<MTLTexture>)mtltexture_white_;
}

// MARK: - Metal Engine implemetations
void  texture_engine_releaseAll_(Texture* tex) {
    if(tex->mtlTex_cptr) {    CFRelease(tex->mtlTex_cptr);    tex->mtlTex_cptr = NULL; }
    if(tex->mtlTexTmp_cptr) { CFRelease(tex->mtlTexTmp_cptr); tex->mtlTexTmp_cptr = NULL; }
    tex->flags &= ~ tex_flags_drawn_;
}
void  texture_engine_releasePartly_(Texture* tex) {
    if(tex->mtlTex_cptr) {    CFRelease(tex->mtlTex_cptr);    tex->mtlTex_cptr = NULL; }
    tex->flags &= ~ tex_flag_fullyDrawn_;
}

// MARK: - Créer une MTLTexture à partir d'un png
void           texture_engine_load_(Texture*const tex, size_t const width, size_t const height, bool const asMini, 
                    PixelBGRA const*const pixelsOpt) 
{
    const void**const mtlTexRef = asMini ? &tex->mtlTexTmp_cptr : &tex->mtlTex_cptr;
    if(*mtlTexRef) {
        printwarning("Texture already set.");
        CFRelease(*mtlTexRef); *mtlTexRef = NULL;
    }
    MTLTextureDescriptor* descr = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:CoqGraph_metal_pixelFormat
                                                        width:width height:height mipmapped:NO];
    id<MTLTexture> mtlTex = [CoqGraph_metal_device newTextureWithDescriptor:descr];
    if(pixelsOpt) {
        [mtlTex replaceRegion:MTLRegionMake2D(0, 0, mtlTex.width, mtlTex.height) mipmapLevel:0
             withBytes:pixelsOpt bytesPerRow:mtlTex.width * 4];
    }
    *mtlTexRef = CFBridgingRetain(mtlTex);
    tex->flags |= asMini ? tex_flag_tmpDrawn_ : tex_flag_fullyDrawn_;
    // Mise à jour des dimensions
    tex->dims.width =  (float)width;
    tex->dims.height = (float)height;
    tex->dims.tileRatio =  (float)width / (float)height * (float)tex->dims.n / (float)tex->dims.m;
}

id<MTLTexture> MTLTexture_createPngImageOpt_(char const*const pngPathOpt) {
    if(!pngPathOpt) return nil;
    NSURL *pngURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:pngPathOpt] isDirectory:NO];
    if(pngURL == nil) return nil;
    NSError *error = nil;
    id<MTLTexture> mtlTexture = [MTLtextureLoader_ newTextureWithContentsOfURL:pngURL
                                       options:@{MTKTextureLoaderOptionSRGB: @false}
                                         error:&error];
    if(error != nil || mtlTexture == nil) {
        printerror("Cannot create MTL texture from %s.", pngPathOpt);
        return nil;
    }
    return mtlTexture;
}
void  texture_engine_tryToLoadAsPng_(Texture*const tex, bool const isMini) {
    char *pngPath = FileManager_getPngPathOpt(tex->string, tex->flags & tex_flag_png_coqlib, isMini);
    
    // Chargement avec `MTLtextureLoader_`, est équivalent de :
//    PixelBGRAArray*const pa = Pixels_engine_createOptFromPng(pngPath);
//    if(!pa) { printerror("No png %s at %s.", tex->string, pngPath); return; }
//    texture_engine_load_(tex, pa->width, pa->height, isMini, pa->pixels);
//     free(pa);
    
    id<MTLTexture> mtlTex = MTLTexture_createPngImageOpt_(pngPath);
    if(!mtlTex) {
        if(!isMini) printerror("No png for %s.", tex->string);
        return;
     }
    const void** mtlCptrRef = isMini ? &tex->mtlTexTmp_cptr : &tex->mtlTex_cptr;
    if(*mtlCptrRef) { CFRelease(*mtlCptrRef); *mtlCptrRef = NULL; }
    *mtlCptrRef = CFBridgingRetain(mtlTex);
    tex->flags |= isMini ? tex_flag_tmpDrawn_ : tex_flag_fullyDrawn_;
    // Mise à jour des dimensions
    tex->dims.width =  (float)[mtlTex width];
    tex->dims.height = (float)[mtlTex height];
    tex->dims.tileRatio =  (float)[mtlTex width] / (float)[mtlTex height] * (float)tex->dims.n / (float)tex->dims.m;
}

void  texture_engine_writeAllPixels(Texture* tex, const void* pixelsBGRA8) {
    if(!tex->mtlTex_cptr) { printerror("Texture not drawn."); return; }
    id<MTLTexture> mtlTex = (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    [mtlTex replaceRegion:MTLRegionMake2D(0, 0, mtlTex.width, mtlTex.height) mipmapLevel:0
             withBytes:pixelsBGRA8 bytesPerRow:mtlTex.width * 4];
}
void  texture_engine_writePixelsToRegion(Texture* tex, const void* pixelsBGRA8, RectangleUint region) {
    if(!tex->mtlTex_cptr) { printerror("Texture not drawn."); return; }
    id<MTLTexture> mtlTex = (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    [mtlTex replaceRegion:MTLRegionMake2D(region.o_x, region.o_y, region.w, region.h) mipmapLevel:0
             withBytes:pixelsBGRA8 bytesPerRow:region.w * 4];
}
void  texture_engine_writeRegionToPixels(const Texture* const tex, void* pixelBGRA8, RectangleUint const region) {
    if(!tex->mtlTex_cptr) { printerror("Texture not drawn."); return; }
    id<MTLTexture> mtlTex = (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    [mtlTex getBytes:pixelBGRA8 bytesPerRow:region.w * 4 fromRegion:MTLRegionMake2D(region.o_x, region.o_y, region.w, region.h) mipmapLevel:0];
}
void  texture_engine_copyRegionTo(const Texture* const tex, Texture* const texDest,
                                  RectangleUint const srcRect, UintPair const destOrig)
{
    if(!tex->mtlTex_cptr || !texDest->mtlTex_cptr) { printerror("Texture not drawn."); return; }
    if((srcRect.o_x + srcRect.w > (uint32_t)tex->dims.width) ||        (srcRect.o_y + srcRect.h > (uint32_t)tex->dims.height) ||
       (destOrig.uint0 + srcRect.w > (uint32_t)texDest->dims.width) || (destOrig.uint1 + srcRect.h > (uint32_t)texDest->dims.height)) {
        printerror("Overflow"); return;
    }
    id<MTLTexture> mtlTex = (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    id<MTLTexture> mtlTexDest = (__bridge id<MTLTexture>)texDest->mtlTex_cptr;

    MTLRegion regionSrc = MTLRegionMake2D(srcRect.o_x, srcRect.o_y, srcRect.w, srcRect.h);
    MTLRegion regionDst = MTLRegionMake2D(destOrig.uint0, destOrig.uint1, srcRect.w, srcRect.h);

    PixelBGRA* pixels = coq_calloc(srcRect.w * srcRect.h, sizeof(PixelBGRA));
    [mtlTex getBytes:pixels bytesPerRow:srcRect.w * 4 fromRegion:regionSrc mipmapLevel:0];
    [mtlTexDest replaceRegion:regionDst mipmapLevel:0 withBytes:pixels bytesPerRow:srcRect.w * 4];
    coq_free(pixels);
}

TextureToDraw   texture_engine_touchAndGetToDraw(Texture *const tex) {
    tex->touchTime = ChronosRender.render_elapsedMS;
    // Cas "OK"
    if(tex->mtlTex_cptr) {
        return (TextureToDraw) {
            .metal_texture_cptr = tex->mtlTex_cptr,
            .isNearest = tex->flags & tex_flag_nearest,
        };
    }
    // Il y a des texture en demande pas encore "fully drawn"...
    Texture_needToFullyDraw_ = true;
    if(tex->mtlTexTmp_cptr) {
        return (TextureToDraw) {
            .metal_texture_cptr = tex->mtlTexTmp_cptr,
            .isNearest = tex->flags & tex_flag_nearest,
        };
    }
//    #warning Utile ?
//    printwarning("Texture %s has not been init.", tex->string);
    return (TextureToDraw) {
        .metal_texture_cptr = mtltexture_transparent_,
        .isNearest = true,
    };
}

// MARK: - Metal misc setter getter ----
// Metal Texture par defaut.



// Garbage
//static Font*                Font_current_ = nil;
//static Font*                Font_currentMini_ = nil;
//static double               Font_currentSize_ = 24;
//static double               Font_miniSize_ =    12;
//static NSValue*             Font_currentSpreading_ = nil;
//static NSParagraphStyle*    Font_paragraphStyle_ = nil;
//static NSMutableDictionary* Font_currentAttributes_ = nil;
//static NSDictionary*        Font_currentAttributesMini_ = nil;
//static NSString* const      CoqSpreadingAttributeName = @"CoqStringSpreadingAttributeKey";
// Les infos de dimensions utiles pour la texture et pour dessiner une string dans le CoreGraphics context.
//typedef struct StringDimensions_ {
//    size_t width, height;
//    float alpha, beta;
//    float font_xHeight,  font_descender;
//    float height_string, width_extra;
//} StringDimensions_;
//NSDictionary*     FontAttributes_createWith_(const char* const fontName_cstrOpt, Vector4 const color, bool const mini) {
//    // 0. Cas par defaut
//    if(!fontName_cstrOpt && (color.r == 0 && color.g == 0 && color.b == 0))
//        return mini ? Font_currentAttributesMini_ : Font_currentAttributes_;
//    // 1. Font
//    Font* font = nil;
//    NSValue* spreading = nil;
//    if(fontName_cstrOpt) {
//        font = [Font fontWithName:[NSString stringWithUTF8String:fontName_cstrOpt]
//                             size:mini ? Font_miniSize_ : Font_currentSize_];
//        if(font == nil)
//            printwarning("Cannot load font %s.", fontName_cstrOpt);
//        else {
//            Vector2 spreading_v = Font_getFontInfoOf(fontName_cstrOpt)->spreading;
//#if TARGET_OS_OSX == 1
//            spreading = [NSValue valueWithSize:CGSizeMake(spreading_v.w, spreading_v.h)];
//#else
//            spreading = [NSValue valueWithCGSize:CGSizeMake(spreading_v.w, spreading_v.h)];
//#endif
//        }
//    }
//    // Default font...
//    if(font == nil) {
//        font = mini ? Font_currentMini_ : Font_current_;
//        spreading = Font_currentSpreading_;
//    }
//    // 3. Color
//#if TARGET_OS_OSX == 1
//    NSColor* color_objc = [NSColor colorWithRed:color.r
//#else
//    UIColor* color_objc = [UIColor colorWithRed:color.r
//#endif
//                                          green:color.g blue:color.b alpha:1];
//    // 4. Attributs de la string (Dictionnaire avec font, paragraph style, color,...)
//    return [NSDictionary dictionaryWithObjects:@[font, Font_paragraphStyle_, color_objc, spreading]
//        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
//}
//StringDimensions_ fontattributes_stringDimensions_(NSDictionary* const attributes, NSString* const string) {
//    // 0. Init
//    Font* font =               [attributes valueForKey:NSFontAttributeName];
//    NSValue* spreading_value = [attributes valueForKey:CoqSpreadingAttributeName];
//    CGSize spreading;
//    #if TARGET_OS_OSX == 1
//    if(spreading_value) spreading = [spreading_value sizeValue];
//    #else
//    if(spreading_value) spreading = [spreading_value CGSizeValue];
//    #endif
//    else {
//        printerror("Missing spreading value in attributes.");
//        spreading = CGSizeMake(1.3, 1.0);
//    }
//    spreading_value = nil;
//    CGSize string_size = [string sizeWithAttributes:attributes];
//    CGFloat width_extra = 0.55 * spreading.width * font.xHeight;
//    CGFloat width = ceil(string_size.width) + width_extra;
//    CGFloat height = 2.00 * spreading.height * font.xHeight;
//    return (StringDimensions_){
//        width,  height,
//        (float)(string_size.width / width), (float)(1.f / spreading.height),
//        font.xHeight, font.descender,
//        string_size.height, width_extra
//    };
//}
//id<MTLTexture>    MTLTexture_createStringWith_(NSString* const string, NSDictionary* const attributes,
//                                               StringDimensions_ strDims)
//{
//    static const CGFloat y_stringRelShift = -0.15;
//    // 2. Context CoreGraphics
//    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
//    CGImageAlphaInfo bitmapInfo = kCGImageAlphaPremultipliedLast;
//    CGContextRef context = CGBitmapContextCreate(NULL,
//        (size_t)strDims.width, (size_t)strDims.height,
//        8, 0, colorSpace, bitmapInfo);
//    if(context == nil) {
//        printerror("Cannot load CGContext");
//        CGColorSpaceRelease(colorSpace);
//        return nil;
//    }
//    // (lettres remplies avec le contour)
//    CGContextSetTextDrawingMode(context, kCGTextFillStroke);
//    // 3. Dessiner la string dans le context
//    // (set context CoreGraphics dans context NSGraphics pour dessiner la NSString.)
//#if TARGET_OS_OSX == 1
//@autoreleasepool {
//    [NSGraphicsContext saveGraphicsState];
//    NSGraphicsContext *nsgcontext = [NSGraphicsContext graphicsContextWithCGContext:context flipped:false];
//    [NSGraphicsContext setCurrentContext:nsgcontext];
//    // Si on place à (0,0) la lettre est coller sur le bord du haut... D'où cet ajustement pour être centré.
//    CGPoint drawPoint = CGPointMake(
//        0.5 * strDims.width_extra,
//        (y_stringRelShift - 0.5) * strDims.font_xHeight
//           + 0.5 * strDims.height + strDims.font_descender
//    );
//    [string drawAtPoint:drawPoint withAttributes:attributes];
//    [NSGraphicsContext setCurrentContext:nil];
//    [NSGraphicsContext restoreGraphicsState];
//    nsgcontext = nil;
//}
//#else
//    UIGraphicsPushContext(context);
//    CGContextScaleCTM(context, 1.0, -1.0);
//    CGPoint drawPoint = CGPointMake(
//        0.5 * strDims.width_extra,
//        (0.5 - y_stringRelShift) * strDims.font_xHeight
//          - strDims.height_string - strDims.font_descender
//          - 0.5 * strDims.height
//    );
//    [string drawAtPoint:drawPoint withAttributes:attributes];
//    UIGraphicsPopContext();
//#endif
//    // 4. Créer une image du context et en faire une texture.
//    CGImageRef image = CGBitmapContextCreateImage(context);
//    // 5. Faire du bitmap une texture.
//    NSError *error = nil;
//    id<MTLTexture> mtlTexture = [MTLtextureLoader_ newTextureWithCGImage:image options:nil error:&error];
//    // (6. libérer les ressources)
//    CGImageRelease(image);
//    CGContextRelease(context);
//    CGColorSpaceRelease(colorSpace);
//    if(error != nil || mtlTexture == nil) {
//        NSLog(@"❌ Error: Cannot make MTLTexture of string %@ with CGImage.", string);
//    }
//    return mtlTexture;
//}
//NSString* texture_evalNSStringOpt_(Texture* const tex) {
//    NSString* nsstring = nil;
//    if(!(tex->flags & tex_flag_string)) {
//        printerror("Not a string."); return nil;
//    }
//    if(tex->flags & string_flag_localized) {
//        const char* localized = String_createLocalized(tex->string);
//        nsstring = [NSString stringWithUTF8String: localized];
//        coq_free((char*)localized);
//    } else {
//        nsstring = [NSString stringWithUTF8String: tex->string];
//    }
//    return nsstring;
//}
//void      texture_updateStringSizes_(Texture* const tex, StringDimensions_ strDims) {
//    tex->alpha = strDims.alpha;
//    tex->beta = strDims.beta;
//    tex->sizes.w = (float)strDims.width;
//    tex->sizes.h = (float)strDims.height;
//    tex->ratio = (float)strDims.width / (float)strDims.height;//  *  tex->ptu.n / tex->ptu.m;
//}
// Transfer la mtlTex -> mtlTexTmp (pour redessiner la mtlTex)
//void  texture_engine_setStringAsToRedraw_(Texture* tex) {
//    if(tex->mtlTexTmp_cptr) { CFRelease(tex->mtlTexTmp_cptr); tex->mtlTexTmp_cptr = NULL; }
//    tex->mtlTexTmp_cptr = tex->mtlTex_cptr;
//    if(tex->mtlTexTmp_cptr) {
//        tex->flags |= tex_flag_tmpDrawn_;
//    } else {
//        tex->flags &= ~tex_flag_tmpDrawn_;
//    }
//    tex->mtlTex_cptr = NULL;
//    tex->flags &= ~tex_flag_fullyDrawn_;
//}
//void  texture_engine_tryToLoadAsString_(Texture* tex, bool isMini) {
//    // Dessiner la string.
//    NSString* string = texture_evalNSStringOpt_(tex);
//    if(!string) return;
//    NSDictionary* attributes = FontAttributes_createWith_(tex->string_fontOpt, tex->string_color, isMini);
//    StringDimensions_ strDims = fontattributes_stringDimensions_(attributes, string);
//    id<MTLTexture> mtlTex = MTLTexture_createStringWith_(string, attributes, strDims);
//    if(!mtlTex) return;
//    // Setter le pointeur vers Objective-C
//    const void** mtlCptrRef = isMini ? &tex->mtlTexTmp_cptr : &tex->mtlTex_cptr;
//    if(*mtlCptrRef) { CFRelease(*mtlCptrRef); *mtlCptrRef = NULL; }
//    *mtlCptrRef = CFBridgingRetain(mtlTex);
//    tex->flags |= isMini ? tex_flag_tmpDrawn_ : tex_flag_fullyDrawn_;
//    // Mise à jour des dimensions
//    texture_updateStringSizes_(tex, strDims);
//    // Release tmp.
//    if(!isMini) {
//        if(tex->mtlTexTmp_cptr) { CFRelease(tex->mtlTexTmp_cptr); tex->mtlTexTmp_cptr = NULL; }
//        tex->flags &= ~ tex_flag_tmpDrawn_;
//    }
//}
//void  texture_engine_justSetSizeAsString_(Texture* tex) {
//    // Obtenir ses dimensions.
//    NSString* string = texture_evalNSStringOpt_(tex);
//    NSDictionary* attributes = FontAttributes_createWith_(tex->string_fontOpt, tex->string_color, true);
//    StringDimensions_ strDims = fontattributes_stringDimensions_(attributes, string);
//    // Mise à jour des dimensions
//    texture_updateStringSizes_(tex, strDims);
//    // Mettre un place holder...
//    if(tex->mtlTexTmp_cptr) { CFRelease(tex->mtlTexTmp_cptr); tex->mtlTexTmp_cptr = NULL; }
//    tex->mtlTexTmp_cptr = CFBridgingRetain(mtltexture_transparent_);
//    tex->flags |= tex_flag_tmpDrawn_;
//}
//void     Texture_setCurrentFont(const char* fontName) {
//    if(fontName == NULL) {
//        printerror("No fontName.");
//        return;
//    }
//    Font* newFont = [Font fontWithName:[NSString stringWithUTF8String:fontName]
//                                 size:Font_currentSize_];
//    if(newFont == nil) {
//        printerror("Font %s not found.", fontName);
//        return;
//    }
//    // Mise a jour des string attributes par defaut.
//    Font_current_ = newFont;
//    Font_currentMini_ = [Font_current_ fontWithSize:Font_miniSize_];
//    Vector2 spreading_v = Font_getFontInfoOf(fontName)->spreading;
//    #if TARGET_OS_OSX == 1
//    Font_currentSpreading_ = [NSValue valueWithSize:CGSizeMake(spreading_v.w, spreading_v.h)];
//    NSColor* black = [NSColor blackColor];
//    #else
//    Font_currentSpreading_ = [NSValue valueWithCGSize:CGSizeMake(spreading_v.w, spreading_v.h)];
//    UIColor* black = [UIColor blackColor];
//    #endif
//    Font_currentAttributes_ = [NSMutableDictionary
//        dictionaryWithObjects:@[Font_current_, Font_paragraphStyle_,  black,           Font_currentSpreading_]
//        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
//    Font_currentAttributesMini_ = [NSDictionary
//        dictionaryWithObjects:@[Font_currentMini_, Font_paragraphStyle_, black,        Font_currentSpreading_]
//        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
//}
//void     Texture_setCurrentFontSize(double newSize) {
//    Font_currentSize_ = newSize;
//    if(Font_current_ == nil) {
//        return;
//    }
//    Font_current_ = [Font_current_ fontWithSize:newSize];
////    NSLog(@"Attributes %@", [Font_currentAttributes_ debugDescription]);
//    [Font_currentAttributes_ setValue:Font_current_ forKey:NSFontAttributeName];
//}
//double   Texture_currentFontSize(void) {
//    return Font_currentSize_;
//}
