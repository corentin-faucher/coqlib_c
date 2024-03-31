//
//  graph_texture_apple.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <CoreGraphics/CoreGraphics.h>

#include "coq_map.h"
#include "graph__apple.h"
#include "graphs/graph_font_manager.h"
#include "graphs/graph_colors.h"

enum {
    tex_flag_string_localized = string_flag_localized,
    tex_flag_string_shared    = string_flag_shared,
    tex_flag_string_mutable   = string_flag_mutable,
    tex_flag_nearest          = string_flag_nearest,
    tex_flag_string           = 0x0010,
    tex_flag_png_shared       = 0x0020, // (png sont toujours shared)
    tex_flag_png_coqlib       = 0x0040, // (png inclus par défaut)
    tex_flag_static_          = 0x0080, // Ne peut pas être dealloc.
    
    tex_flags_string_         = 0x000F,
};

#define TEX_UNUSED_DELTATMS_ 1000

typedef struct Texture_ {
    PerTextureUniforms ptu;
    /// Tiling en x, y. Meme chose que ptu -> m, n.
    uint32_t         m, n;
    /// Ratio sans les marges en x, y, e.g. (0.95, 0.95).
    /// Pour avoir les dimension utiles de la texture.
    float            alpha, beta;
    /// Ration w / h d'une tile. -> ratio = (w / h) * (n / m).
    float            ratio;
    uint32_t         flags;
    int64_t          touchTime;
    
    char*            string;  // Copie de la String ou nom du png, pour s'il faut redessiner la texture.
    char*            string_fontOpt;
    Texture**        string_refererOpt; // Sa référence dans une liste.
                                // (i.e. pour les strings quelconques)
    Vector4          string_color; // Couleur pour dessiner la string (noir par defaut)
    uint32_t         string_ref_count; // Juste pour les strings. Les png restent en mémoire. (sont libéré si non utilisé)
    // Metal
    id<MTLTexture>   mtlTexture;
    id<MTLTexture>   mtlTextureTmp;
} Texture;

static Texture      _Texture_dummy = {
    PTU_DEFAULT,
    1, 1,
    1.f, 1.f, 1.f,
    tex_flag_static_, 0,
    "dummy", NULL, NULL, { 0, 0, 0, 0 }, 0,
    nil, nil
};

/*-- Variable globales --*/
static int32_t           Texture_total_count_ = 0;
static bool              _Texture_isInit = false;
static bool              _Texture_isLoaded = false;
volatile static bool     _Texture_needToFullyDraw = false;

/*-- Font pour les texture de strings --------------------------------------*/
#if TARGET_OS_OSX == 1
typedef NSFont Font;
#else
typedef UIFont Font;
#endif
static Font*                Font_current_ = nil;
static Font*                Font_currentMini_ = nil;
static double               Font_currentSize_ = 24;
static NSMutableDictionary* Font_currentAttributes_ = nil;
static NSDictionary*        Font_currentAttributesMini_ = nil;
static double               Font_miniSize_ =    12;
static NSValue*             Font_current_spreading_ = nil;
static NSParagraphStyle*    Font_paragraphStyle_ = nil;
//static NSColor*             Font_defaultColor_ = nil;

/*-- Creation de la texture Metal -------------------------------*/
static MTKTextureLoader* MTLtextureLoader_ = NULL;
static NSString* const CoqSpreadingAttributeName = @"CoqStringSpreadingAttributeKey";
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
        spreading = Font_current_spreading_;
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
static const CGFloat     Texture_y_string_rel_shift_ = -0.15;
// Les infos de dimensions utiles pour la texture et pour dessiner une string dans le CoreGraphics context. 
typedef struct StringDimensions_ {
    size_t width, height;
    float alpha, beta;
    float font_xHeight,  font_descender;
    float height_string, width_extra;
} StringDimensions_;
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
id<MTLTexture>    fontattributes_MTLTextureOfString_(NSDictionary* const attributes, NSString* const string, StringDimensions_* strDimsRef) {
    // 1. Dimensions de la string
    *strDimsRef = fontattributes_stringDimensions_(attributes, string);
    // 2. Context CoreGraphics
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageAlphaInfo bitmapInfo = kCGImageAlphaPremultipliedLast;
    CGContextRef context = CGBitmapContextCreate(NULL,
        (size_t)strDimsRef->width, (size_t)strDimsRef->height,
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
        0.5 * strDimsRef->width_extra,
        (Texture_y_string_rel_shift_ - 0.5) * strDimsRef->font_xHeight
           + 0.5 * strDimsRef->height + strDimsRef->font_descender
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
        0.5 * strDimsRef->width_extra,
        (0.5 - Texture_y_string_rel_shift_) * strDimsRef->font_xHeight
          - strDimsRef->height_string - strDimsRef->font_descender
          - 0.5 * strDimsRef->height
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

__attribute__((deprecated("Utiliser `FontAttributes_MTLTextureOfString_`.")))
id<MTLTexture> MTLTexture_createStringForOpt_(Texture* const tex, bool mini) {
    // NSString
    NSString* nsstring = nil;
    if(tex->flags & string_flag_localized) {
        char* localized = String_createLocalized(tex->string);
        nsstring = [NSString stringWithUTF8String: localized];
        coq_free(localized);
    } else {
        nsstring = [NSString stringWithUTF8String: tex->string];
    }
    // Font et string attributes
    Vector2 spreading;
    Font* font = nil;
    NSDictionary* attributes = FontAttributes_createWith_(tex->string_fontOpt, tex->string_color, mini);

    // Dimensions de la string
    CGSize renderedSize = [nsstring sizeWithAttributes:attributes];
    printdebug("Sizes %f, %f for %s.", renderedSize.width, renderedSize.height, tex->string);
    CGFloat extraWidth = 0.55 * spreading.w * font.xHeight;
    CGFloat contextHeight = 2.00 * spreading.h * font.xHeight;
    CGFloat contextWidth =  ceil(renderedSize.width) + extraWidth;
    // Mise à jour des variable de la texture
    tex->alpha = (float)(renderedSize.width / contextWidth);
    tex->beta =  (float)(1.f / spreading.h);
    tex->ptu.width = contextWidth;
    tex->ptu.height = contextHeight;
    tex->ratio = contextWidth / contextHeight;
    // Context
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageAlphaInfo bitmapInfo = kCGImageAlphaPremultipliedLast;
    CGContextRef context = CGBitmapContextCreate(NULL,
        (size_t)contextWidth, (size_t)contextHeight,
        8, 0, colorSpace, bitmapInfo);
    if(context == nil) {
        printerror("Cannot load CGContext");
        CGColorSpaceRelease(colorSpace);
        return nil;
    }
    // (lettres remplies avec le contour)
    CGContextSetTextDrawingMode(context, kCGTextFillStroke);
    // 7. Dessiner la string dans le context
    // (set context CoreGraphics dans context NSGraphics pour dessiner la NSString.)
#if TARGET_OS_OSX == 1
@autoreleasepool {
    [NSGraphicsContext saveGraphicsState];
    NSGraphicsContext *nsgcontext = [NSGraphicsContext graphicsContextWithCGContext:context flipped:false];
    [NSGraphicsContext setCurrentContext:nsgcontext];
    // Si on place à (0,0) la lettre est coller sur le bord du haut... D'où cet ajustement pour être centré.
    CGPoint drawPoint = CGPointMake(
        0.5 * extraWidth,
        (Texture_y_string_rel_shift_ - 0.5) * font.xHeight
           + 0.5 * contextHeight + font.descender
    );
    [nsstring drawAtPoint:drawPoint withAttributes:attributes];
    [NSGraphicsContext setCurrentContext:nil];
    [NSGraphicsContext restoreGraphicsState];
    nsgcontext = nil;
}
#else
    CGFloat strHeight = renderedSize.height;
    UIGraphicsPushContext(context);
    CGContextScaleCTM(context, 1.0, -1.0);
    CGPoint drawPoint = CGPointMake(
        0.5 * extraWidth,
        (0.5 - Texture_y_string_rel_shift_) * font.xHeight
          - strHeight - font.descender
          - 0.5 * contextHeight
    );
    [nsstring drawAtPoint:drawPoint
           withAttributes:attributes];
    UIGraphicsPopContext();
#endif
    nsstring = nil;
    font = nil;
    attributes = nil;
    // 8. Créer une image du context et en faire une texture.
    CGImageRef image = CGBitmapContextCreateImage(context);
    
    NSError *error = nil;
    id<MTLTexture> mtlTexture = [MTLtextureLoader_ newTextureWithCGImage:image options:nil error:&error];
    CGImageRelease(image);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    
    if(error != nil || mtlTexture == nil) {
        printerror("Cannot make MTLTexture of string %s with CGImage.", tex->string);
    }
    return mtlTexture;
}
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
__attribute__((deprecated("Utiliser `MTLTexture_createPngImageOpt_`.")))
id<MTLTexture> MTLTexture_createImageForOpt_(Texture* const tex, bool mini) {
    NSString *NSpngName = [NSString stringWithUTF8String:tex->string];
    NSString *png_dir = (tex->flags & tex_flag_png_coqlib) ? @"pngs_coqlib" : @"pngs";
    if(mini) png_dir = [png_dir stringByAppendingString:@"/minis"];
    NSURL *pngUrl = [[NSBundle mainBundle] URLForResource:NSpngName
                                            withExtension:@"png"
                                             subdirectory:png_dir];
    png_dir = nil;
    NSpngName = nil;
    if(pngUrl == NULL) {
        // (Ok, si pas de mini, mais probleme si pas la texture standard)
        if(!mini) {
            printerror("cannot init url for non-mini %s.", tex->string);
        }
        return nil;
    }
//    tex->flags |= setMini ? tex_flag_miniDrawn : tex_flag_fullyDrawn;
    NSError *error = nil;
    id<MTLTexture> mtlTexture = [MTLtextureLoader_ newTextureWithContentsOfURL:pngUrl
                                       options:@{MTKTextureLoaderOptionSRGB: @false}
                                         error:&error];
    if(error != nil || mtlTexture == nil) {
        printerror("cannot create MTL texture for %s png.", tex->string);
        return nil;
    }
    // Mettre à jour les variables de la texture.
    tex->ptu.width =  (float)[mtlTexture width];
    tex->ptu.height = (float)[mtlTexture height];
    tex->ratio = tex->ptu.width / tex->ptu.height * tex->ptu.n / tex->ptu.m;
    return mtlTexture;
}

/*-- Fonctions privees sur une instance Texture -------------------------*/
NSString* texture_evalNSStringOpt(Texture* const tex) {
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
void  texture_updatePngSizesWithMTLTexture_(Texture* const tex) {
    id<MTLTexture> mtlTex;
    if(tex->mtlTexture) mtlTex = tex->mtlTexture;
    else mtlTex = tex->mtlTextureTmp;
    if(mtlTex == nil) {
        printerror("Metal texture not init for %s.", tex->string);
        return;
    }
    tex->ptu.width =  (float)[mtlTex width];
    tex->ptu.height = (float)[mtlTex height];
    tex->ratio = tex->ptu.width / tex->ptu.height * tex->ptu.n / tex->ptu.m;
}
void  texture_updateStringSizes_(Texture* const tex, StringDimensions_ strDims) {
    tex->alpha = strDims.alpha;
    tex->beta = strDims.beta;
    tex->ptu.width = (float)strDims.width;
    tex->ptu.height = (float)strDims.height;
    tex->ratio = tex->ptu.width / tex->ptu.height * tex->ptu.n / tex->ptu.m;
}
bool  _texture_isUsed(Texture* const tex) {
    return CR_elapsedMS_ - tex->touchTime <= TEX_UNUSED_DELTATMS_;
}
bool  _texture_isUnused(Texture* const tex) {
    return CR_elapsedMS_ - tex->touchTime > TEX_UNUSED_DELTATMS_ + 30;
}
void  texture_deinit_(void* tex_void) {
    Texture* tex = tex_void;
//    if(tex->flags & tex_flag_string) {
//        if(tex->flags & tex_flag_string_shared)
//            printdebug("Remove shared string %s.", tex->string);
//        else if(tex->flags & tex_flag_string_mutable)
//            printdebug("Remove mutable string %s.", tex->string);
//        else printdebug("Remove some string %s.", tex->string);
//    }
    tex->mtlTexture = nil;
    tex->mtlTextureTmp = nil;
    coq_free(tex->string);
    tex->string = NULL;
    // Déréferencer...
    if(tex->string_refererOpt)
        *(tex->string_refererOpt) = (Texture*)NULL;
    if(tex->string_fontOpt) {
        coq_free(tex->string_fontOpt);
        tex->string_fontOpt = NULL;
    }
    Texture_total_count_ --;
}
void  _texture_unsetPartly(Texture* tex) {
    tex->mtlTexture = nil;
}
static  void (* const _texture_unsetPartly_)(char*) = (void (*)(char*))_texture_unsetPartly;
void  texture_setAsToRedraw_(Texture* tex) {
    if(!(tex->flags & tex_flag_string)) return;
    tex->mtlTextureTmp = tex->mtlTexture;
    tex->mtlTexture = nil;
}
static void (* const texture_setAsToRedraw__)(char*) = (void (*)(char*))texture_setAsToRedraw_;
void  _texture_unsetFully(Texture* tex) {
    tex->mtlTexture = nil;
    tex->mtlTextureTmp = nil;
}
static void (* const _texture_unsetFully_)(char*) = (void (*)(char*))_texture_unsetFully;
void  _texture_unsetUnusedPartly_(char* tex_char) {
    Texture* tex = (Texture*)tex_char;
    if(tex->mtlTexture == nil) return;
    if(!_texture_isUnused(tex)) return;
    tex->mtlTexture = nil;
}


/*-- List de textures -----------------------------------------------------*/
/// Maps des png (shared).
static StringMap*        textureOfPngName_ = NULL; // Pngs dans une map.
static Texture**         textureOfPngId_ = NULL;   // Pngs dans un array.
static const PngInfo*    pngInfos_ = NULL;         // Liste des infos des pngs. 
static uint              pngCount_ = 0;            // Nombre de pngs.
// Metal Texture par defaut.
static id<MTLTexture>    mtltexture_transparent_ = nil;
/// Liste des pngs par defaut
const PngInfo            coqlib_pngInfos_[] = {
    {"coqlib_bar_in", 1, 1, false, true},
    {"coqlib_digits_black", 12, 2, false, true},
    {"coqlib_scroll_bar_back", 1, 1, false, true},
    {"coqlib_scroll_bar_front", 1, 1, false, true},
    {"coqlib_sliding_menu_back", 1, 1, false, true},
    {"coqlib_sparkle_stars", 3, 2, false, true},
    {"coqlib_switch_back", 1, 1, false, true},
    {"coqlib_switch_front", 1, 1, false, true},
    {"coqlib_test_frame", 1, 1, true, true},
    {"coqlib_the_cat", 1, 1, true, true},
    {"coqlib_transparent", 1, 1, true, true},
    {"coqlib_white", 1, 1, true, true},
};
const uint32_t           coqlib_pngCount_ = sizeof(coqlib_pngInfos_) / sizeof(PngInfo);
/// Maps des constant strings (shared)
static StringMap*        textureOfSharedString_ = NULL;
// Liste des strings quelconques...
static const uint32_t    _nonCstStrTexCount = 64;
static Texture*          _nonCstStrTexArray[_nonCstStrTexCount];
static Texture** const   _nonCstStrArrEnd = &_nonCstStrTexArray[_nonCstStrTexCount];
static Texture**         _nonCstStrCurrent = _nonCstStrTexArray;
// Strings par défaut (utile ? au lieu des mini)
//static Texture*          texture_default_1_ = NULL;
//static Texture*          texture_default_2_ = NULL;
//static Texture*          texture_default_3_ = NULL;

/*-- Font ----------------------------------------------------------------*/
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
    Font_current_spreading_ = [NSValue valueWithSize:CGSizeMake(spreading_v.w, spreading_v.h)];
    NSColor* black = [NSColor blackColor];
    #else
    Font_current_spreading_ = [NSValue valueWithCGSize:CGSizeMake(spreading_v.w, spreading_v.h)];
    UIColor* black = [UIColor blackColor];
    #endif
    Font_currentAttributes_ = [NSMutableDictionary 
        dictionaryWithObjects:@[Font_current_, Font_paragraphStyle_,  black,           Font_current_spreading_]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
    Font_currentAttributesMini_ = [NSDictionary 
        dictionaryWithObjects:@[Font_currentMini_, Font_paragraphStyle_, black,        Font_current_spreading_]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
    
    // Redessiner les strings...
    map_applyToAll(textureOfSharedString_, _texture_unsetFully_);
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) { _texture_unsetFully(*tr); }
        tr ++;
    }
}
void     Texture_setCurrentFontSize(double newSize) {
    Font_currentSize_ = newSize;
    if(Font_current_ == nil) {
        return;
    }
    Font_current_ = [Font_current_ fontWithSize:newSize];
//    NSLog(@"Attributes %@", [Font_currentAttributes_ debugDescription]);
    [Font_currentAttributes_ setValue:Font_current_ forKey:NSFontAttributeName];
    // Defaire les mtlTexture pour redessiner les strings...
    map_applyToAll(textureOfSharedString_, texture_setAsToRedraw__);
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) { texture_setAsToRedraw_(*tr); }
        tr ++;
    }
}
double   Texture_currentFontSize(void) {
    return Font_currentSize_;
}

#pragma mark -- Globals ----------------------------------------------------------*/
void     texture_initAsPng_(Texture* const tex, const PngInfo* const info) {
    // Init des champs
    tex->ptu = (PerTextureUniforms) { 8, 8, (float)info->m, (float)info->n };
    tex->m =     info->m; tex->n =     info->n;
    tex->alpha = 1.f;     tex->beta =  1.f;
    tex->ratio = (float)info->n / (float)info->m;  // (temporaire)
    tex->flags = tex_flag_png_shared|(info->isCoqlib ? tex_flag_png_coqlib : 0)
                    |(info->nearest ? tex_flag_nearest : 0);
    tex->touchTime = CR_elapsedMS_ - TEX_UNUSED_DELTATMS_;
    
    tex->string = String_createCopy(info->name);
    
    tex->mtlTexture = nil;
    // Essayer de loader la mini (et init les dimensions)
    NSString* pngName = [NSString stringWithUTF8String:tex->string];
    tex->mtlTextureTmp = MTLTexture_createPngImageOpt_(pngName, tex->flags & tex_flag_png_coqlib, true);
    if(tex->mtlTextureTmp) texture_updatePngSizesWithMTLTexture_(tex);
    
    Texture_total_count_ ++;
}
void     Texture_init(id<MTLDevice> const device, PngInfo const pngInfos[], const uint pngCount) {
    if(_Texture_isInit) {
        printerror("Texture already init.");
        return;
    }
    // 1. Texture loader et font.
    MTLtextureLoader_ = [[MTKTextureLoader alloc] initWithDevice:device];
    Font_current_ =     [Font systemFontOfSize:Font_currentSize_];
    Font_currentMini_ = [Font systemFontOfSize:Font_miniSize_];
    #if TARGET_OS_OSX == 1
    Font_current_spreading_ = [NSValue valueWithSize:CGSizeMake(1.3f, 1.0f)];
    NSColor* black = [NSColor blackColor];
    #else
    Font_current_spreading_ = [NSValue valueWithCGSize:CGSizeMake(1.3f, 1.0f)];
    UIColor* black = [UIColor blackColor];
    #endif
    NSMutableParagraphStyle* paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    paragraphStyle.alignment = NSTextAlignmentCenter;
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
    Font_paragraphStyle_ = paragraphStyle;
    Font_currentAttributes_ = [NSMutableDictionary 
        dictionaryWithObjects:@[Font_current_, Font_paragraphStyle_,  black,           Font_current_spreading_]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
    Font_currentAttributesMini_ = [NSDictionary 
        dictionaryWithObjects:@[Font_currentMini_, Font_paragraphStyle_, black,        Font_current_spreading_]
        forKeys:@[NSFontAttributeName, NSParagraphStyleAttributeName, NSForegroundColorAttributeName, CoqSpreadingAttributeName]];
        
    // 2. Pngs
    pngInfos_ = pngInfos;
    textureOfPngId_ =    coq_calloc(pngCount, sizeof(Texture*));
    textureOfPngName_ =  Map_create(64, sizeof(Texture));
    pngCount_ = pngCount;
    // Preload 
    const PngInfo* info = coqlib_pngInfos_;
    const PngInfo* end = &coqlib_pngInfos_[coqlib_pngCount_];
    while(info < end) {
        Texture* png_tex = (Texture*)map_put(textureOfPngName_, info->name, NULL);
        texture_initAsPng_(png_tex, info); // (ici c'est juste vide, i.e. sans MTLTexture)
        info ++;
    }
    mtltexture_transparent_ = MTLTexture_createPngImageOpt_(@"coqlib_transparent", true, false);
    
    // Preload des user pngs.
    info = pngInfos_;
    end = &pngInfos_[pngCount_];
    uint32_t index = 0;
    while(info < end) {
        // Vérifier si existe déjà... (pourrait redéfinir un coqlib png)
        Texture* png_tex = (Texture*)map_valueRefOptOfKey(textureOfPngName_, info->name);
        if(!png_tex) {
            png_tex = (Texture*)map_put(textureOfPngName_, info->name, NULL);
            texture_initAsPng_(png_tex, info);
        }
        // Ajout a l'array de ref.
        textureOfPngId_[index] = png_tex;
        index ++;
        info ++;
    }
    
    // 3. Strings
    textureOfSharedString_ = Map_create(64, sizeof(Texture));
    memset(_nonCstStrTexArray, 0, sizeof(Texture*) * _nonCstStrTexCount);
    
    _Texture_isInit = true;
    _Texture_isLoaded = true;
}
void     Texture_suspend(void) {
    if(!_Texture_isInit || !_Texture_isLoaded) return;
    map_applyToAll(textureOfPngName_,      _texture_unsetPartly_);
    map_applyToAll(textureOfSharedString_, _texture_unsetFully_);
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) { _texture_unsetFully(*tr); }
        tr ++;
    }
    _Texture_isLoaded = false;
}
void     Texture_resume(void) {
    _Texture_isLoaded = true;
//#warning Superflu finalement ? Texture utilisees seront chargees si besoin dans la premiere frame.
}
void     Texture_deinit(void) {
    _Texture_isInit = false;
    _Texture_isLoaded = false;
    MTLtextureLoader_ = nil;
    map_destroyAndNull(&textureOfSharedString_, texture_deinit_);
    map_destroyAndNull(&textureOfPngName_,      texture_deinit_);
    if(textureOfPngId_)
        coq_free(textureOfPngId_);
    pngCount_ = 0;
}

void     Texture_checkToFullyDrawAndUnused(ChronoChecker* cc, int64_t timesUp) {
    static unsigned checkunset_counter = 0;
    // Une fois de temps en temps, quand on a le temps,
    if(!_Texture_needToFullyDraw) {
        // liberer les png non utilises.
        checkunset_counter = (checkunset_counter + 1) % 30;
        if(checkunset_counter == 0)
            map_applyToAll(textureOfPngName_, _texture_unsetUnusedPartly_);
        if(checkunset_counter != 1) return;
        // liberer les shared strings non utilisées...
        bool notFinish = map_iterator_init(textureOfSharedString_);
        while(notFinish) {
            Texture* tex = (Texture*)map_iterator_valueRefOpt(textureOfSharedString_);
            if((tex->string_ref_count == 0) && _texture_isUnused(tex)) {
                notFinish = map_iterator_removeAndNext(textureOfSharedString_, texture_deinit_);
            } else {
                notFinish = map_iterator_next(textureOfSharedString_);
            }
        }
        return;
    }    
    // Chercher les png, `const string`, `non const string` à dessiner au complet...
    if(map_iterator_init(textureOfPngName_)) do {
        Texture* tex = (Texture*)map_iterator_valueRefOpt(textureOfPngName_);
        if(!_texture_isUsed(tex)) continue;
        if(tex->mtlTexture == nil) {
            tex->mtlTexture = MTLTexture_createPngImageOpt_([NSString stringWithUTF8String:tex->string], tex->flags & tex_flag_png_coqlib, false);
            texture_updatePngSizesWithMTLTexture_(tex);
            if(chronochecker_elapsedMS(cc) > timesUp)
                return;
        }
    } while(map_iterator_next(textureOfPngName_));
    if(map_iterator_init(textureOfSharedString_)) do {
        Texture* tex = (Texture*)map_iterator_valueRefOpt(textureOfSharedString_);
        if(!_texture_isUsed(tex)) continue;
        if(tex->mtlTexture == nil) {
            NSDictionary* attributes = FontAttributes_createWith_(tex->string_fontOpt, tex->string_color, false);
            NSString* string = texture_evalNSStringOpt(tex);
            StringDimensions_ strDims = { 0 };
            if(string) tex->mtlTexture = fontattributes_MTLTextureOfString_(attributes, string, &strDims);
            texture_updateStringSizes_(tex, strDims);
            tex->mtlTextureTmp = nil;
            if(chronochecker_elapsedMS(cc) > timesUp)
                return;
        }
    } while(map_iterator_next(textureOfSharedString_));
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) if((*tr)->mtlTexture == nil) {
            NSString* string = texture_evalNSStringOpt(*tr);
            NSDictionary* attributes = FontAttributes_createWith_((*tr)->string_fontOpt, (*tr)->string_color, false);
            StringDimensions_ strDims = { 0 };
            if(string) (*tr)->mtlTexture = fontattributes_MTLTextureOfString_(attributes, string, &strDims);
            texture_updateStringSizes_(*tr, strDims);
            (*tr)->mtlTextureTmp = nil;
            if(chronochecker_elapsedMS(cc) > timesUp)
                return;
        }
        tr ++;
    }
    _Texture_needToFullyDraw = false;
}

#pragma mark -- Constructors... ----------------------------*/

Texture* Texture_sharedImage(uint const pngId) {
    if(!textureOfPngId_) { printerror("pngs not loaded."); return &_Texture_dummy; }
    if(pngId >= pngCount_) {
        printerror("Invalid png id %d. Max id is %d.", pngId, pngCount_);
        return &_Texture_dummy;
    }
    Texture *tex = textureOfPngId_[pngId];
    tex->touchTime = CR_elapsedMS_;
    // S'il y a pas la mini, créer tout de suite la vrai texture. (besoin des dimensions)
    // (Normalement, s'il n'y a pas de mini c'est que la texture est petite...)
    if((tex->mtlTextureTmp == nil) && (tex->mtlTexture == nil)) {
        ChronoChecker cc;
        chronochecker_set(&cc);
        NSString* pngName = [NSString stringWithUTF8String:tex->string];
        tex->mtlTexture = MTLTexture_createPngImageOpt_(pngName, tex->flags & tex_flag_png_coqlib, false);
        texture_updatePngSizesWithMTLTexture_(tex);
        if(chronochecker_elapsedMS(&cc) > 10)
            printwarning("Png %d, no mini for %s. time %lld.", pngId, tex->string, chronochecker_elapsedMS(&cc));
    }
    return tex;
}
Texture* Texture_sharedImageByName(const char* pngName) {
    if(!textureOfPngName_) { printerror("Texture not init."); return &_Texture_dummy; }
    Texture* tex = (Texture*)map_valueRefOptOfKey(textureOfPngName_, pngName);
    if(!tex) {
        printerror("Invalid png name %s.", pngName);
        return &_Texture_dummy;
    }
    tex->touchTime = CR_elapsedMS_;
    if((tex->mtlTextureTmp == nil) && (tex->mtlTexture == nil)) {
        NSString* pngName = [NSString stringWithUTF8String:tex->string];
        tex->mtlTexture = MTLTexture_createPngImageOpt_(pngName, tex->flags & tex_flag_png_coqlib, false);
        texture_updatePngSizesWithMTLTexture_(tex);
    }
    return tex;
}

void     _texture_addToNonSharedStringReferers(Texture* tex) {
    Texture** const referer = _nonCstStrCurrent;
    if(*referer) {
        printwarning("Texture referer space occupied.");
    } else {
        // Référencer dans la liste des "non constant strings".
        *referer = tex;
        tex->string_refererOpt = referer;
    }
    // Cherche un disponible pour le prochain...
    do {
        _nonCstStrCurrent++;
        if(_nonCstStrCurrent >= _nonCstStrArrEnd)
            _nonCstStrCurrent = _nonCstStrTexArray;
        if(*_nonCstStrCurrent == NULL)
            return;
    } while(_nonCstStrCurrent != referer);
    printwarning("Texture referer space not found.");
}
// Création d'une string quelconque (mutable, localisable, avec font...)
Texture* Texture_retainString(StringDrawable str) {
    if(!_Texture_isInit) {
        printerror("Texture not init.");
        return &_Texture_dummy;
    }
    // 0. Récuperer si la string existe.
    Texture* tex;
    if(str.string_flags & tex_flag_string_shared) {
        if(str.string_flags & string_flag_mutable) {
            printerror("Shared string cannot be mutable.");
            str.string_flags &= ~tex_flag_string_mutable;
        }
        tex = (Texture*)map_valueRefOptOfKey(textureOfSharedString_, str.c_str);
        // Existe, juste incrémenter le compteur de références.
        if(tex) {
            tex->string_ref_count ++;
            return tex;
        }
        // N'existe pas encore, créer...
        tex = (Texture*)map_put(textureOfSharedString_, str.c_str, NULL);
    } else {
        tex = coq_calloc(1, sizeof(Texture));
    }
    
    // 1. Init des champs de base.
    tex->ptu = ptu_default;
    tex->m =     1;   tex->n =     1;
    tex->flags = tex_flag_string|(str.string_flags & tex_flags_string_);
    tex->string_ref_count = 1;
    tex->touchTime = CR_elapsedMS_;
    tex->string = String_createCopy(str.c_str);
    tex->string_fontOpt = str.fontNameOpt ? String_createCopy(str.fontNameOpt) : NULL;
    tex->string_color = str.color;
    // 1. Evaluer les dimensions.
    NSDictionary* attributes = FontAttributes_createWith_(str.fontNameOpt, str.color, false);
    NSString* nsstring = texture_evalNSStringOpt(tex);
    StringDimensions_ strDims = { 0 };
    
//    tex->mtlTexture = fontattributes_MTLTextureOfString_(attributes, nsstring, &strDims);
    strDims = fontattributes_stringDimensions_(attributes, nsstring);
    tex->mtlTextureTmp = mtltexture_transparent_;
    
    texture_updateStringSizes_(tex, strDims);
    
    // 3. Garder une référence pour pause/resume.
    if(!(str.string_flags & string_flag_shared))
        _texture_addToNonSharedStringReferers(tex);
    // (sinon c'est dans textureOfSharedString_)
    
    Texture_total_count_ ++;
    return tex;
}
void    textureref_releaseAndNull_(Texture** const texRef) {
    if(*texRef == NULL) return;
    Texture* const tex = *texRef;
    *texRef = NULL;
    // Fait rien pour les pngs... (reste en mémoire)
    if(tex->flags & tex_flag_png_shared) return;
    if(tex->string_ref_count > 0) tex->string_ref_count --;
    if(tex->string_ref_count > 0) return;
    // Ne fait que décrémenter le compteur pour les shared
    if(tex->flags & tex_flag_string_shared) return;
    
    // Seules les non-shared sont deallocted (tout de suite)
    texture_deinit_(tex);
    coq_free(tex);
}
void    textureref_exchangeSharedStringFor(Texture** const texRef, StringDrawable str) {
    if(((*texRef)->flags & tex_flag_string_shared) == 0 || (str.string_flags & string_flag_shared) == 0) {
        printerror("Not shared strings.");
        return;
    }
    Texture* const oldTex = *texRef;
    *texRef = Texture_retainString(str);
    if(oldTex->string_ref_count > 0) oldTex->string_ref_count --;
}

/*-- Fonctions public sur une instance Texture -------------------------*/
void  texture_updateMutableString(Texture* tex, const char* newString, bool forceRedraw) {
    if(!(tex->flags & string_flag_mutable) || !newString) {
        printwarning("Not a mutable string or missing newString.");
        return;
    }
    coq_free(tex->string);
    tex->string = String_createCopy(newString);
    // Besoin d'une mini temporaire ?
    bool mini = !forceRedraw && (Font_currentSize_ > 40);
    // (La texture est live, il faut creer la nouvelle avant de remplacer.)
    NSDictionary* attributes = FontAttributes_createWith_(tex->string_fontOpt, tex->string_color, mini);
    NSString* string = texture_evalNSStringOpt(tex);
    StringDimensions_ strDims = { 0 };
    id<MTLTexture> mtlTex = fontattributes_MTLTextureOfString_(attributes, string, &strDims);
    texture_updateStringSizes_(tex, strDims);
    if(mini) {
        tex->mtlTextureTmp = mtlTex;
        tex->mtlTexture = nil;
    } else {
        tex->mtlTexture = mtlTex;
        tex->mtlTextureTmp = nil;  // (N'est plus valide)
    }
    
    tex->touchTime = CR_elapsedMS_;
}

#pragma mark - Getters

id<MTLTexture>    texture_MTLTexture(Texture* tex) {
    tex->touchTime = CR_elapsedMS_;
    id<MTLTexture> mtlTex = tex->mtlTexture;
    // Cas "OK"
    if(mtlTex != nil)
        return mtlTex;
    // Il y a des texture en demande pas encore "fully drawn"...
    _Texture_needToFullyDraw = true;
    mtlTex = tex->mtlTextureTmp;
    if(mtlTex != nil)
        return mtlTex;
        
//    printwarning("Texture %s has not been init.", tex->string);
//    return mtltexture_transparent_;
    // Ni mini ni standard ? Essayer de dessiner
    if(tex->flags & tex_flag_string) {
        NSDictionary* attributes = FontAttributes_createWith_(tex->string_fontOpt, tex->string_color, true);
        NSString* string = texture_evalNSStringOpt(tex);
        StringDimensions_ strDims = { 0 };
        mtlTex = fontattributes_MTLTextureOfString_(attributes, string, &strDims);
        texture_updateStringSizes_(tex, strDims);
        tex->mtlTextureTmp = mtlTex;
        return tex->mtlTextureTmp;
    }
//    // Cas png pas de mini pas init.
    return mtltexture_transparent_;
//    NSString* pngName = [NSString stringWithUTF8String:tex->string];
//    tex->mtlTexture = MTLTexture_createPngImageOpt_(pngName, tex->flags & tex_flag_png_coqlib, false);
//    return tex->mtlTexture;
}
bool              texture_isNearest(Texture* tex) {
    return tex->flags & tex_flag_nearest;
}
bool              texture_isSharedPng(Texture* tex) {
    return tex->flags & tex_flag_png_shared;
}
const PerTextureUniforms* texture_ptu(Texture* tex) {
    return &tex->ptu;
}
uint32_t          texture_m(Texture *tex) {
    return tex->m;
}
uint32_t          texture_n(Texture *tex) {
    return tex->n;
}
uint32_t          texture_mn(Texture *tex) {
    return tex->m * tex->n;
}
float             texture_ratio(Texture* tex) {
    return tex->ratio;
}
float             texture_alpha(Texture* tex) {
    return tex->alpha;
}
float             texture_beta(Texture* tex) {
    return tex->beta;
}
const char*       texture_string(Texture* tex) {
    return tex->string;
}
