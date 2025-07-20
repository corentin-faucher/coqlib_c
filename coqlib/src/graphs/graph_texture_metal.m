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
#include "../systems/system_file.h"
#include "utils/util_map.h"
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
                .width = [mtlTex width], .height = [mtlTex height],
                .tileRatio = (float)[mtlTex width] / (float)[mtlTex height],
            },
            tex_flag_shared|tex_flag__fullyDrawn|tex_flag__static, ChronosRender.render_elapsedMS,
        };
    } else {
        Texture_frameBuffers[index]->dims.width  = [mtlTex width];
        Texture_frameBuffers[index]->dims.height = [mtlTex height];
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
    if(tex->mtlTex2_cptr) {   CFRelease(tex->mtlTex2_cptr);   tex->mtlTex2_cptr = NULL; }
    if(tex->mtlTexTmp_cptr) { CFRelease(tex->mtlTexTmp_cptr); tex->mtlTexTmp_cptr = NULL; }
    tex->flags &= ~ tex_flags__drawn;
}
void  texture_engine_releasePartly_(Texture* tex) {
    if(tex->mtlTex_cptr) {    CFRelease(tex->mtlTex_cptr);    tex->mtlTex_cptr = NULL; }
    if(tex->mtlTex2_cptr) {   CFRelease(tex->mtlTex2_cptr);   tex->mtlTex2_cptr = NULL; }
    tex->flags &= ~ tex_flag__fullyDrawn;
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
    *mtlTexRef = CFBridgingRetain(mtlTex);
    id<MTLTexture> mtlTex2 = nil;
    if(!asMini && (tex->flags & tex_flag_doubleBuffer)) {
        if(tex->mtlTex2_cptr) { printwarning("Texture already set.");
            CFRelease(tex->mtlTex2_cptr); tex->mtlTex2_cptr = NULL;
        }
        mtlTex2 = [CoqGraph_metal_device newTextureWithDescriptor:descr];
        tex->mtlTex2_cptr = CFBridgingRetain(mtlTex2);
    }
    if(pixelsOpt) {
        [mtlTex replaceRegion:MTLRegionMake2D(0, 0, mtlTex.width, mtlTex.height)
            mipmapLevel:0 withBytes:pixelsOpt bytesPerRow:mtlTex.width * 4];
        if(mtlTex2)
            [mtlTex2 replaceRegion:MTLRegionMake2D(0, 0, mtlTex2.width, mtlTex2.height) 
                mipmapLevel:0 withBytes:pixelsOpt bytesPerRow:mtlTex2.width * 4];
    }
    
    tex->flags |= asMini ? tex_flag__tmpDrawn : tex_flag__fullyDrawn;
    // Mise à jour des dimensions
    tex->dims.width =  width;
    tex->dims.height = height;
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
    char *pngPath = FileManager_getPngPathOpt(tex->string, tex->flags & tex_flag__png_coqlib, isMini);
    
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
    tex->flags |= isMini ? tex_flag__tmpDrawn : tex_flag__fullyDrawn;
    // Mise à jour des dimensions
    tex->dims.width =  [mtlTex width];
    tex->dims.height = [mtlTex height];
    tex->dims.tileRatio =  (float)[mtlTex width] / (float)[mtlTex height] * (float)tex->dims.n / (float)tex->dims.m;
}

// MARK: - Obtenir les pixels d'une texture
// La texture en "lecture" ou "live" pour lire les pixels officiellement "à jour".
static inline id<MTLTexture> texture_readableMTLTexture_(Texture const*const tex) {
    if(!(tex->flags & tex_flag_doubleBuffer))
        return (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    if(!tex->mtlTex2_cptr) { printerror("Missing mtlTex2."); 
        return (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    }
    if(tex->flags & tex_flag__dbSecondLive)
        return (__bridge id<MTLTexture>)tex->mtlTex2_cptr;
    return (__bridge id<MTLTexture>)tex->mtlTex_cptr;
}
// La texture en "écriture" (pas live) pour éditer les pixels.
static inline id<MTLTexture> texture_editableMTLTexture_(Texture*const tex) {
    if(!(tex->flags & tex_flag_doubleBuffer))
        return (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    if(!tex->mtlTex2_cptr) { printerror("Missing mtlTex2."); 
        return (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    }
    if(tex->flags & tex_flag__dbSecondLive)
        return (__bridge id<MTLTexture>)tex->mtlTex_cptr;
    return (__bridge id<MTLTexture>)tex->mtlTex2_cptr;
}
PixelBGRAArray* PixelBGRAArray_engine_createFromTextureOpt(Texture*const tex) {
    if(!tex->mtlTex_cptr) { printerror("Texture not drawn."); return NULL; }
    id<MTLTexture> mtlTex = texture_readableMTLTexture_(tex);
    PixelBGRAArray*const pa = PixelBGRAArray_createEmpty(mtlTex.width, mtlTex.height);
    MTLRegion const region = MTLRegionMake2D(0, 0, mtlTex.width, mtlTex.height);
    [mtlTex getBytes:pa->pixels bytesPerRow:region.size.width * 4 
          fromRegion:region mipmapLevel:0];
    return pa;
}
PixelBGRAArray* PixelBGRAArray_engine_createFromTextureSubRegionOpt(Texture*const tex, RectangleUint region) {
    if(!tex->mtlTex_cptr) { printerror("Texture not drawn."); return NULL; }
    size_t const tex_width = tex->dims.width;
    size_t const tex_height = tex->dims.height;
    if(region.o_x >= tex_width || region.o_y >= tex_height) {
        printerror("Region outside."); 
        return NULL;
    }
    if(!region.w || !region.h) {
        printwarning("Empty region."); 
        return NULL;
    }
    if(region.o_x + region.w > tex_width || region.o_y + region.h > tex_height) {
        printwarning("Region overflow.");
        region.w = uminu(region.w, (uint32_t)tex_width  - region.o_x);
        region.h = uminu(region.h, (uint32_t)tex_height - region.o_y);
    }
    id<MTLTexture> mtlTex = texture_readableMTLTexture_(tex);
    MTLRegion const regionMtl = MTLRegionMake2D(region.o_x, region.o_y, region.w, region.h);
    PixelBGRAArray*const pa = PixelBGRAArray_createEmpty(region.w, region.h); 
    [mtlTex getBytes:pa->pixels bytesPerRow:region.w * 4 
          fromRegion:regionMtl mipmapLevel:0];
    return pa;
}

// MARK: - Edition
void texture_engine_writePixelsAt(Texture* tex, PixelBGRAArray const* pixels, UintPair const origin) 
{
    if(!tex->mtlTex_cptr || !pixels) { printerror("Texture not drawn or no pixels."); return; }
    if((tex->flags & tex_flag_doubleBuffer) && (tex->flags & tex_flag__dbEdited)) {
        printwarning("Buffer already edited."); return; 
    }
    size_t const tex_width = tex->dims.width;
    size_t const tex_height = tex->dims.height;
    if(origin.uint0 >= tex_width || origin.uint1 >= tex_height) {
        printerror("Region outside."); 
        return;
    }
    size_t dst_width = pixels->width;
    size_t dst_height = pixels->height;
    if(origin.uint0 + dst_width > tex_width || origin.uint1 + dst_height > tex_height) 
    {
        printwarning("Region overflow.");
        dst_width =  uminu((uint32_t)dst_width, (uint32_t)tex_width - origin.uint0);
        dst_height = uminu((uint32_t)dst_height, (uint32_t)tex_height - origin.uint1);
    }
    id<MTLTexture> mtlTex = texture_editableMTLTexture_(tex);
    MTLRegion regionMtl = MTLRegionMake2D(origin.uint0, origin.uint1, dst_width, dst_height);
    
    // Dessin !
    [mtlTex replaceRegion:regionMtl mipmapLevel:0
                withBytes:pixels->pixels bytesPerRow:pixels->width * 4];

    if(tex->flags & tex_flag_doubleBuffer)
        tex->flags |= tex_flag__dbEdited;
}
/// Copier une région d'une texture vers une autre texture.
/// Équivalent de `texture_engine_createPixelsSubRegion`, `texture_engine_writePixelsToRegion`.
void texture_engine_copyRegionTo(const Texture* texSrc, RectangleUint srcRect,
                                  Texture* texDst, UintPair destOrig) 
{
    if(!texSrc->mtlTex_cptr || !texDst->mtlTex_cptr) { printerror("Texture not drawn."); return; }
    if((srcRect.o_x + srcRect.w > (uint32_t)texSrc->dims.width) ||
       (srcRect.o_y + srcRect.h > (uint32_t)texSrc->dims.height) ||
       (destOrig.uint0 + srcRect.w > (uint32_t)texDst->dims.width) ||
       (destOrig.uint1 + srcRect.h > (uint32_t)texDst->dims.height)) {
        printerror("Overflow"); return;
    }
    if((texDst->flags & tex_flag_doubleBuffer) && (texDst->flags & tex_flag__dbEdited)) {
        printwarning("Already edited."); return; 
    }
    id<MTLTexture> mtlTexSrc = texture_readableMTLTexture_(texSrc);
    id<MTLTexture> mtlTexDst = texture_editableMTLTexture_(texDst);
    MTLRegion regionSrc = MTLRegionMake2D(srcRect.o_x, srcRect.o_y, srcRect.w, srcRect.h);
    MTLRegion regionDst = MTLRegionMake2D(destOrig.uint0, destOrig.uint1, srcRect.w, srcRect.h);

    // Copie et dessin !
    with_beg(PixelBGRA, pixels, coq_callocSimpleArray(srcRect.w * srcRect.h, PixelBGRA))
    [mtlTexSrc getBytes:pixels bytesPerRow:srcRect.w * 4 fromRegion:regionSrc mipmapLevel:0];
    [mtlTexDst replaceRegion:regionDst mipmapLevel:0 withBytes:pixels bytesPerRow:srcRect.w * 4];
    with_end(pixels)
    
    if(texDst->flags & tex_flag_doubleBuffer)
        texDst->flags |= tex_flag__dbEdited;
}

// MARK: - Dessin (renderer)
TextureToDraw   texture_render_getTextureToDraw(Texture*const tex) {
    if(tex->mtlTex_cptr) {
        if(tex->mtlTex2_cptr && (tex->flags & tex_flag__dbSecondLive))
            return (TextureToDraw) {
                .mtlTexture = tex->mtlTex2_cptr,
                .isNearest = tex->flags & tex_flag_nearest,
            };
        return (TextureToDraw) {
            .mtlTexture = tex->mtlTex_cptr,
            .isNearest = tex->flags & tex_flag_nearest,
        };
    }
    if(tex->mtlTexTmp_cptr) {
        return (TextureToDraw) {
            .mtlTexture = tex->mtlTexTmp_cptr,
            .isNearest = tex->flags & tex_flag_nearest,
        };
    }
//    #warning Utile ?
    return (TextureToDraw) {
        .mtlTexture = mtltexture_transparent_,
        .isNearest = true,
    };
}

// MARK: - Metal misc setter getter ----
// Metal Texture par defaut.



// Garbage
//void            texture_engine_writePixelsToRegion(Texture* tex, 
//                            PixelBGRA const* pixels, RectangleUint region) {
//    if(!tex->mtlTex_cptr || !pixels) { printerror("Texture not drawn or no pixels."); return; }
//    if((tex->flags & tex_flag_doubleBuffer) && (tex->flags & tex_flag__dbEdited)) {
//        printwarning("Buffer already edited."); return; 
//    }
//    size_t const tex_width = tex->dims.width;
//    size_t const tex_height = tex->dims.height;
//    if(region.o_x >= tex_width || region.o_y >= tex_height) {
//        printerror("Region outside."); 
//        return;
//    }
//    size_t dst_width = region.w;
//    size_t dst_height = region.h;
//    if(region.o_x + dst_width > tex_width || region.o_y + dst_height > tex_height) 
//    {
//        printwarning("Region overflow.");
//        dst_width =  uminu((uint32_t)dst_width, (uint32_t)tex_width - region.o_x);
//        dst_height = uminu((uint32_t)dst_height, (uint32_t)tex_height - region.o_y);
//    }
//    id<MTLTexture> mtlTex = texture_editableMTLTexture_(tex);
//    MTLRegion regionMtl = MTLRegionMake2D(region.o_x, region.o_y, dst_width, dst_height);
//    
//    // Dessin !
//    [mtlTex replaceRegion:regionMtl mipmapLevel:0
//                withBytes:pixels bytesPerRow:region.w * 4];
//
//    if(tex->flags & tex_flag_doubleBuffer)
//        tex->flags |= tex_flag__dbEdited;
//}
/// Obtenir les pixels pour édition (NULL si non mutable)
//TextureToEdit texture_retainToEditOpt(Texture* tex) {
//    if(!tex || !(tex->flags & tex_flag_mutable)) {
//        return (TextureToEdit) {};
//    }
//    if(tex->flags & (tex_flag__dbEditing|tex_flag__dbToUpdate)) {
//        printwarning("Cannot edit. Already editing or still not updated.");
//        return (TextureToEdit) {};
//    }
//    size_t const pixelCount = tex->dims.width * tex->dims.height;
//    PixelBGRAArray*const pa = coq_callocArray(PixelBGRAArray, PixelBGRA, pixelCount); 
//    id<MTLTexture> mtlTex = texture_readableMTLTexture_(tex);
//    [mtlTex getBytes:pa bytesPerRow:tex->dims.width * 4
//          fromRegion:MTLRegionMake2D(0, 0, tex->dims.width, tex->dims.height) mipmapLevel:0];
//    tex->flags |= tex_flag__dbEditing;
//    return (TextureToEdit) {
//        .pa = pa,
//        ._tex = tex,
//    };
//}
///// Fini d'éditer, flager pour que la thread de rendering met à jour la texture.
//void          texturetoedit_release(TextureToEdit texEdit) {
//    if(!texEdit._tex || !(texEdit._tex->flags & tex_flag__dbEditing)) {
//        printerror("Not editing or no texture.");
//        if(texEdit.pa) { coq_free(texEdit.pa); }
//        return;
//    }
//    id<MTLTexture> mtlTex = texture_editableMTLTexture_(texEdit._tex);
//    [mtlTex replaceRegion:MTLRegionMake2D(0, 0, mtlTex.width, mtlTex.height) mipmapLevel:0
//                withBytes:texEdit.pa bytesPerRow:mtlTex.width * 4];
//    coq_free(texEdit.pa);
//    texEdit._tex->flags |= tex_flag__dbToUpdate;
//    texEdit._tex->flags &= ~tex_flag__dbEditing;
//}
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
