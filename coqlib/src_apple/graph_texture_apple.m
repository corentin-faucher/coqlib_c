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

#include "math_base.h"
#include "graph_font_manager.h"
#include "graph__apple.h"

#if TARGET_OS_OSX == 1
typedef NSFont Font;
#else
typedef UIFont Font;
#endif

#pragma mark - Globals...

static MTKTextureLoader*    MTLtextureLoader_ = nil;
static Font*                Font_current_ = nil;
static Font*                Font_currentMini_ = nil;
static double               Font_currentSize_ = 24;
static double               Font_miniSize_ =    12;
static NSValue*             Font_currentSpreading_ = nil;
static NSParagraphStyle*    Font_paragraphStyle_ = nil;
static NSMutableDictionary* Font_currentAttributes_ = nil;
static NSDictionary*        Font_currentAttributesMini_ = nil;
static NSString* const      CoqSpreadingAttributeName = @"CoqStringSpreadingAttributeKey";
// Metal Texture par defaut.
static id<MTLTexture>       mtltexture_transparent_ = nil;

#pragma mark - Drawing de strings avec FontAttributes --------------

// Les infos de dimensions utiles pour la texture et pour dessiner une string dans le CoreGraphics context. 
typedef struct StringDimensions_ {
    size_t width, height;
    float alpha, beta;
    float font_xHeight,  font_descender;
    float height_string, width_extra;
} StringDimensions_;
NSDictionary*     FontAttributes_createWith_(const char* const fontName_cstrOpt, Vector4 const color, bool const mini) {
    // 0. Cas par defaut
    if(!fontName_cstrOpt && (color.r == 0 && color.g == 0 && color.b == 0))
        return mini ? Font_currentAttributesMini_ : Font_currentAttributes_;
    // 1. Font
    Font* font = nil;
    NSValue* spreading = nil;
    if(fontName_cstrOpt) {
        font = [Font fontWithName:[NSString stringWithUTF8String:fontName_cstrOpt]
                             size:mini ? Font_miniSize_ : Font_currentSize_];
        if(font == nil)
            printwarning("Cannot load font %s.", fontName_cstrOpt);
        else {
            Vector2 spreading_v = Font_getFontInfoOf(fontName_cstrOpt)->spreading;
#if TARGET_OS_OSX == 1
            spreading = [NSValue valueWithSize:CGSizeMake(spreading_v.w, spreading_v.h)];
#else
            spreading = [NSValue valueWithCGSize:CGSizeMake(spreading_v.w, spreading_v.h)];
#endif
        }
    }
    // Default font...
    if(font == nil) {
        font = mini ? Font_currentMini_ : Font_current_;
        spreading = Font_currentSpreading_;
    }
    // 3. Color
#if TARGET_OS_OSX == 1
    NSColor* color_objc = [NSColor colorWithRed:color.r 
#else
    UIColor* color_objc = [UIColor colorWithRed:color.r 
#endif
                                          green:color.g blue:color.b alpha:1];
    // 4. Attributs de la string (Dictionnaire avec font, paragraph style, color,...)
    return [NSDictionary dictionaryWithObjects:@[font, Font_paragraphStyle_, color_objc, spreading]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
}
StringDimensions_ fontattributes_stringDimensions_(NSDictionary* const attributes, NSString* const string) {
    // 0. Init
    Font* font =               [attributes valueForKey:NSFontAttributeName];
    NSValue* spreading_value = [attributes valueForKey:CoqSpreadingAttributeName];
    CGSize spreading;
    #if TARGET_OS_OSX == 1
    if(spreading_value) spreading = [spreading_value sizeValue];
    #else
    if(spreading_value) spreading = [spreading_value CGSizeValue];
    #endif
    else {
        printerror("Missing spreading value in attributes.");
        spreading = CGSizeMake(1.3, 1.0);
    }
    spreading_value = nil;
    CGSize string_size = [string sizeWithAttributes:attributes];
    CGFloat width_extra = 0.55 * spreading.width * font.xHeight;
    CGFloat width = ceil(string_size.width) + width_extra;
    CGFloat height = 2.00 * spreading.height * font.xHeight;
    return (StringDimensions_){
        width,  height,
        (float)(string_size.width / width), (float)(1.f / spreading.height),
        font.xHeight, font.descender,
        string_size.height, width_extra
    };
}
id<MTLTexture>    MTLTexture_createStringWith_(NSString* const string, NSDictionary* const attributes,  
                                               StringDimensions_ strDims) 
{
    static const CGFloat y_stringRelShift = -0.15;
    // 2. Context CoreGraphics
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageAlphaInfo bitmapInfo = kCGImageAlphaPremultipliedLast;
    CGContextRef context = CGBitmapContextCreate(NULL,
        (size_t)strDims.width, (size_t)strDims.height,
        8, 0, colorSpace, bitmapInfo);
    if(context == nil) {
        printerror("Cannot load CGContext");
        CGColorSpaceRelease(colorSpace);
        return nil;
    }
    // (lettres remplies avec le contour)
    CGContextSetTextDrawingMode(context, kCGTextFillStroke);
    // 3. Dessiner la string dans le context
    // (set context CoreGraphics dans context NSGraphics pour dessiner la NSString.)
#if TARGET_OS_OSX == 1
@autoreleasepool {
    [NSGraphicsContext saveGraphicsState];
    NSGraphicsContext *nsgcontext = [NSGraphicsContext graphicsContextWithCGContext:context flipped:false];
    [NSGraphicsContext setCurrentContext:nsgcontext];
    // Si on place à (0,0) la lettre est coller sur le bord du haut... D'où cet ajustement pour être centré.
    CGPoint drawPoint = CGPointMake(
        0.5 * strDims.width_extra,
        (y_stringRelShift - 0.5) * strDims.font_xHeight
           + 0.5 * strDims.height + strDims.font_descender
    );
    [string drawAtPoint:drawPoint withAttributes:attributes];
    [NSGraphicsContext setCurrentContext:nil];
    [NSGraphicsContext restoreGraphicsState];
    nsgcontext = nil;
}
#else
    UIGraphicsPushContext(context);
    CGContextScaleCTM(context, 1.0, -1.0);
    CGPoint drawPoint = CGPointMake(
        0.5 * strDims.width_extra,
        (0.5 - y_stringRelShift) * strDims.font_xHeight
          - strDims.height_string - strDims.font_descender
          - 0.5 * strDims.height
    );
    [string drawAtPoint:drawPoint withAttributes:attributes];
    UIGraphicsPopContext();
#endif
    // 4. Créer une image du context et en faire une texture.
    CGImageRef image = CGBitmapContextCreateImage(context);
    // 5. Faire du bitmap une texture.
    NSError *error = nil;
    id<MTLTexture> mtlTexture = [MTLtextureLoader_ newTextureWithCGImage:image options:nil error:&error];
    // (6. libérer les ressources)
    CGImageRelease(image);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    if(error != nil || mtlTexture == nil) {
        NSLog(@"❌ Error: Cannot make MTLTexture of string %@ with CGImage.", string);
    }
    return mtlTexture;
}
NSString* texture_evalNSStringOpt_(Texture* const tex) {
    NSString* nsstring = nil;
    if(!(tex->flags & tex_flag_string)) {
        printerror("Not a string."); return nil;
    }
    if(tex->flags & string_flag_localized) {
        char* localized = String_createLocalized(tex->string);
        nsstring = [NSString stringWithUTF8String: localized];
        coq_free(localized);
    } else {
        nsstring = [NSString stringWithUTF8String: tex->string];
    }
    return nsstring;
}
void      texture_updateStringSizes_(Texture* const tex, StringDimensions_ strDims) {
    tex->alpha = strDims.alpha;
    tex->beta = strDims.beta;
//    tex->ptu.width = (float)strDims.width;
//    tex->ptu.height = (float)strDims.height;
    tex->ratio = (float)strDims.width / (float)strDims.height;//  *  tex->ptu.n / tex->ptu.m;
}

#pragma mark - Chargement de png en MTLTexture ---------------------

id<MTLTexture> MTLTexture_createPngImageOpt_(NSString* const pngName, bool const isCoqlib, bool const isMini) {
    NSString *png_dir = isCoqlib ? @"pngs_coqlib" : @"pngs";
    if(isMini) png_dir = [png_dir stringByAppendingString:@"/minis"];
    NSURL *pngUrl = [[NSBundle mainBundle] URLForResource:pngName
                                            withExtension:@"png"
                                             subdirectory:png_dir];
    png_dir = nil;
    if(pngUrl == NULL) {
        // (Ok, si pas de mini, mais probleme si pas la texture standard)
        if(!isMini) {
            NSLog(@"❌ Error: Cannot init url for non-mini %@.", pngName);
        }
        return nil;
    }
    NSError *error = nil;
    id<MTLTexture> mtlTexture = [MTLtextureLoader_ newTextureWithContentsOfURL:pngUrl
                                       options:@{MTKTextureLoaderOptionSRGB: @false}
                                         error:&error];
    if(error != nil || mtlTexture == nil) {
        NSLog(@"❌ Error: Cannot create MTL texture for %@ png.", pngName);
        return nil;
    }
    return mtlTexture;
}

id<MTLTexture> MTLTexture_createWithPixels_(const void* pixelsBGRA8, uint32_t width, uint32_t height) {
    MTLTextureDescriptor* descr = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTL_pixelFormat_
                                                        width:width height:height mipmapped:NO];
    id<MTLTexture> mtlTex = [MTL_device_ newTextureWithDescriptor:descr];
    [mtlTex replaceRegion:MTLRegionMake2D(0, 0, width, height) mipmapLevel:0 
             withBytes:pixelsBGRA8 bytesPerRow:width * 4];
    return mtlTex;
}

#pragma mark - Methods texture ----------------------

void  texture_engine_releaseAll_(Texture* tex) {
    if(tex->mtlTex_cptr) {    CFRelease(tex->mtlTex_cptr);    tex->mtlTex_cptr = NULL; }
    if(tex->mtlTexTmp_cptr) { CFRelease(tex->mtlTexTmp_cptr); tex->mtlTexTmp_cptr = NULL; }
    tex->flags &= ~ tex_flags_drawn_;
}
void  texture_engine_releasePartly_(Texture* tex) {
    if(tex->mtlTex_cptr) {    CFRelease(tex->mtlTex_cptr);    tex->mtlTex_cptr = NULL; }
    tex->flags &= ~ tex_flag_fullyDrawn_;
}
// Transfer la mtlTex -> mtlTexTmp (pour redessiner la mtlTex)
void  texture_engine_setStringAsToRedraw_(Texture* tex) {
    if(tex->mtlTexTmp_cptr) { CFRelease(tex->mtlTexTmp_cptr); tex->mtlTexTmp_cptr = NULL; }
    tex->mtlTexTmp_cptr = tex->mtlTex_cptr;
    if(tex->mtlTexTmp_cptr) {
        tex->flags |= tex_flag_tmpDrawn_;
    } else {
        tex->flags &= ~tex_flag_tmpDrawn_;
    }
    tex->mtlTex_cptr = NULL;
    tex->flags &= ~tex_flag_fullyDrawn_;
}
void  texture_engine_tryToLoadAsPng_(Texture* tex, bool const isMini) {
    NSString* pngName = [NSString stringWithUTF8String:tex->string];
    id<MTLTexture> mtlTex = MTLTexture_createPngImageOpt_(pngName, tex->flags & tex_flag_png_coqlib, isMini);
    if(!mtlTex) return;
    const void** mtlCptrRef = isMini ? &tex->mtlTexTmp_cptr : &tex->mtlTex_cptr;
    if(*mtlCptrRef) { CFRelease(*mtlCptrRef); *mtlCptrRef = NULL; }
    *mtlCptrRef = CFBridgingRetain(mtlTex);
    tex->flags |= isMini ? tex_flag_tmpDrawn_ : tex_flag_fullyDrawn_;
    // Mise à jour des dimensions
//    tex->ptu.width =  (float)[mtlTex width];
//    tex->ptu.height = (float)[mtlTex height];
    tex->ratio = (float)[mtlTex width] / (float)[mtlTex height] 
                 * (float)tex->n / (float)tex->m;
}
void  texture_engine_tryToLoadAsString_(Texture* tex, bool isMini) {
    // Dessiner la string.
    NSString* string = texture_evalNSStringOpt_(tex);
    if(!string) return;
    NSDictionary* attributes = FontAttributes_createWith_(tex->string_fontOpt, tex->string_color, isMini);
    StringDimensions_ strDims = fontattributes_stringDimensions_(attributes, string);
    id<MTLTexture> mtlTex = MTLTexture_createStringWith_(string, attributes, strDims);
    if(!mtlTex) return;
    // Setter le pointeur vers Objective-C
    const void** mtlCptrRef = isMini ? &tex->mtlTexTmp_cptr : &tex->mtlTex_cptr;
    if(*mtlCptrRef) { CFRelease(*mtlCptrRef); *mtlCptrRef = NULL; }
    *mtlCptrRef = CFBridgingRetain(mtlTex);
    tex->flags |= isMini ? tex_flag_tmpDrawn_ : tex_flag_fullyDrawn_;
    // Mise à jour des dimensions
    texture_updateStringSizes_(tex, strDims);
    // Release tmp.
    if(!isMini) {
        if(tex->mtlTexTmp_cptr) { CFRelease(tex->mtlTexTmp_cptr); tex->mtlTexTmp_cptr = NULL; }
        tex->flags &= ~ tex_flag_tmpDrawn_;
    }
}
void  texture_engine_loadWithPixels_(Texture* tex, const void* pixelsBGRA8, uint32_t width, uint32_t height) {
    id<MTLTexture> mtlTex = MTLTexture_createWithPixels_(pixelsBGRA8, width, height);
    if(tex->mtlTex_cptr) { CFRelease(tex->mtlTex_cptr); tex->mtlTex_cptr = NULL; }
    tex->mtlTex_cptr = CFBridgingRetain(mtlTex);
    tex->flags |= tex_flag_fullyDrawn_;
    // Mise à jour des dimensions
    tex->_width = width;
    tex->_height = height;
    tex->ratio = (float)width / (float)height 
                 * (float)tex->n / (float)tex->m;
}
void  texture_engine_updatePixels(Texture* tex, const void* pixelsBGRA8) {
    if(!tex->mtlTex_cptr) { printerror("Texture not drawn."); return; }
    id<MTLTexture> mtlTex = (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    [mtlTex replaceRegion:MTLRegionMake2D(0, 0, mtlTex.width, mtlTex.height) mipmapLevel:0 
             withBytes:pixelsBGRA8 bytesPerRow:mtlTex.width * 4];
}
void  texture_engine_justSetSizeAsString_(Texture* tex) {
    // Obtenir ses dimensions.
    NSString* string = texture_evalNSStringOpt_(tex);
    NSDictionary* attributes = FontAttributes_createWith_(tex->string_fontOpt, tex->string_color, true);
    StringDimensions_ strDims = fontattributes_stringDimensions_(attributes, string);
    // Mise à jour des dimensions
    texture_updateStringSizes_(tex, strDims);
    // Mettre un place holder...
    if(tex->mtlTexTmp_cptr) { CFRelease(tex->mtlTexTmp_cptr); tex->mtlTexTmp_cptr = NULL; }
    tex->mtlTexTmp_cptr = CFBridgingRetain(mtltexture_transparent_);
    tex->flags |= tex_flag_tmpDrawn_;
}
id<MTLTexture> texture_MTLTexture(Texture* tex) {
    tex->touchTime = CR_elapsedMS_;
    // Cas "OK"
    if(tex->mtlTex_cptr) {
        return (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    }
    // Il y a des texture en demande pas encore "fully drawn"...
    Texture_needToFullyDraw_ = true;
    if(tex->mtlTexTmp_cptr) {
        return (__bridge id<MTLTexture>)tex->mtlTexTmp_cptr;
    }
    printwarning("Texture %s has not been init.", tex->string);
    return mtltexture_transparent_;
}

#pragma mark - Fonts -----------------------------

void     Texture_setCurrentFont(const char* fontName) {
    if(fontName == NULL) {
        printerror("No fontName.");
        return;
    }
    Font* newFont = [Font fontWithName:[NSString stringWithUTF8String:fontName]
                                 size:Font_currentSize_];
    if(newFont == nil) {
        printerror("Font %s not found.", fontName);
        return;
    }
    // Mise a jour des string attributes par defaut.
    Font_current_ = newFont;
    Font_currentMini_ = [Font_current_ fontWithSize:Font_miniSize_];
    Vector2 spreading_v = Font_getFontInfoOf(fontName)->spreading;
    #if TARGET_OS_OSX == 1
    Font_currentSpreading_ = [NSValue valueWithSize:CGSizeMake(spreading_v.w, spreading_v.h)];
    NSColor* black = [NSColor blackColor];
    #else
    Font_currentSpreading_ = [NSValue valueWithCGSize:CGSizeMake(spreading_v.w, spreading_v.h)];
    UIColor* black = [UIColor blackColor];
    #endif
    Font_currentAttributes_ = [NSMutableDictionary 
        dictionaryWithObjects:@[Font_current_, Font_paragraphStyle_,  black,           Font_currentSpreading_]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
    Font_currentAttributesMini_ = [NSDictionary 
        dictionaryWithObjects:@[Font_currentMini_, Font_paragraphStyle_, black,        Font_currentSpreading_]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
}
void     Texture_setCurrentFontSize(double newSize) {
    Font_currentSize_ = newSize;
    if(Font_current_ == nil) {
        return;
    }
    Font_current_ = [Font_current_ fontWithSize:newSize];
//    NSLog(@"Attributes %@", [Font_currentAttributes_ debugDescription]);
    [Font_currentAttributes_ setValue:Font_current_ forKey:NSFontAttributeName];
}
double   Texture_currentFontSize(void) {
    return Font_currentSize_;
}


#pragma mark - Init Metal --------------------

static Vertex mesh_sprite_vertices_[4] = {
    {-0.5, 0.5, 0, 0.0001, 0.0001, 0,0,1},
    {-0.5,-0.5, 0, 0.0001, 0.9999, 0,0,1},
    { 0.5, 0.5, 0, 0.9999, 0.0001, 0,0,1},
    { 0.5,-0.5, 0, 0.9999, 0.9999, 0,0,1},
};
static uint32_t transparent_texture_pixels_[4] = {
    0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
};
Mesh*  mesh_sprite = NULL;
id<MTLDevice>  MTL_device_ = nil;
const MTLPixelFormat MTL_pixelFormat_ = MTLPixelFormatBGRA8Unorm;
void      CoqGraph_MTLinit(id<MTLDevice> const device) {
    if(MTLtextureLoader_) {
        printerror("Texture already init.");
        return;
    }
    MTL_device_ = device;
    // 1. Texture loader et font.
    MTLtextureLoader_ = [[MTKTextureLoader alloc] initWithDevice:device];
    Font_current_ =     [Font systemFontOfSize:Font_currentSize_];
    Font_currentMini_ = [Font systemFontOfSize:Font_miniSize_];
    #if TARGET_OS_OSX == 1
    Font_currentSpreading_ = [NSValue valueWithSize:CGSizeMake(1.3f, 1.0f)];
    NSColor* black = [NSColor blackColor];
    #else
    Font_currentSpreading_ = [NSValue valueWithCGSize:CGSizeMake(1.3f, 1.0f)];
    UIColor* black = [UIColor blackColor];
    #endif
    NSMutableParagraphStyle* paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    paragraphStyle.alignment = NSTextAlignmentCenter;
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
    Font_paragraphStyle_ = paragraphStyle;
    Font_currentAttributes_ = [NSMutableDictionary 
        dictionaryWithObjects:@[Font_current_, Font_paragraphStyle_,  black,           Font_currentSpreading_]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
    Font_currentAttributesMini_ = [NSDictionary 
        dictionaryWithObjects:@[Font_currentMini_, Font_paragraphStyle_, black,        Font_currentSpreading_]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
    // 2. Texture par défaut
    mtltexture_transparent_ = MTLTexture_createWithPixels_(transparent_texture_pixels_, 2, 2);
    
    // 3. Mesh par défaut (sprite)
    mesh_sprite = Mesh_createEmpty(mesh_sprite_vertices_, 4, NULL, 0,
          mesh_primitive_triangleStrip, mesh_cullMode_none, true);
}

