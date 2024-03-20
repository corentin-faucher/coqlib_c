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
    id<MTLTexture>   mtlMini;
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
static NSMutableDictionary* Font_currentAttributes_ = nil;
static NSMutableDictionary* Font_currentMiniAttributes_ = nil;
static double               Font_currentSize_ = 24;
static double               Font_miniSize_ =    20;
static Vector2              Font_current_spreading_ = { 1.3f, 1.0f };

/*-- Creation de la texture Metal -------------------------------*/
static MTKTextureLoader* MTLtextureLoader_ = NULL;
NSMutableDictionary*     FontAttributes_createWithFont_(Font* font) {
    // 2. Paragraph style
    NSMutableParagraphStyle* paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    paragraphStyle.alignment = NSTextAlignmentCenter;
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
    // 3. Attributs de la string (color, font, paragraph style)
    NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
    [attributes setObject:font forKey:NSFontAttributeName];
    [attributes setObject:paragraphStyle forKey:NSParagraphStyleAttributeName];
    paragraphStyle = nil;
    return attributes;
}
static const CGFloat     Texture_y_string_rel_shift_ = -0.15;

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
    NSMutableDictionary* attributes = nil;
    if(tex->string_fontOpt) {
        font = [Font fontWithName:[NSString stringWithUTF8String:tex->string_fontOpt]
                                size:mini ? Font_miniSize_ : Font_currentSize_];
        if(font == nil)
            printwarning("Cannot load font %s.", tex->string_fontOpt);
        else {
            spreading = Font_getFontInfoOf(tex->string_fontOpt)->spreading;
            attributes = FontAttributes_createWithFont_(font);
        }
    }
    if(font == nil) {
        font = mini ? Font_currentMini_ : Font_current_;
        attributes = mini ? Font_currentMiniAttributes_ : Font_currentAttributes_;
        spreading = Font_current_spreading_;
    }
    // Couleur
#if TARGET_OS_OSX == 1
    NSColor* color = [NSColor colorWithRed:tex->string_color.r 
#else
    UIColor* color = [UIColor colorWithRed:tex->string_color.r 
#endif
                                         green:tex->string_color.g 
                                          blue:tex->string_color.b 
                                         alpha:1];
    [attributes setObject:color forKey:NSForegroundColorAttributeName];
    // Dimensions de la string
    CGSize renderedSize = [nsstring sizeWithAttributes:attributes];
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
                                       options:@{MTKTextureLoaderOptionSRGB: @true}
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
    tex->mtlMini = nil;
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
void  _texture_unsetFully(Texture* tex) {
    tex->mtlTexture = nil;
    tex->mtlMini = nil;
}
static  void (* const _texture_unsetFully_)(char*) = (void (*)(char*))_texture_unsetFully;
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
    Font_current_ = newFont;
    Font_currentMini_ = [Font_current_ fontWithSize:Font_miniSize_];
    [Font_currentAttributes_ setObject:Font_current_ forKey:NSFontAttributeName];
    [Font_currentMiniAttributes_ setObject:Font_currentMini_ forKey:NSFontAttributeName];
    Font_current_spreading_ = Font_getFontInfoOf(fontName)->spreading;
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
    [Font_currentAttributes_ setObject:Font_current_ forKey:NSFontAttributeName];
    // Redessiner les strings...
    map_applyToAll(textureOfSharedString_, _texture_unsetPartly_);
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) { _texture_unsetPartly(*tr); }
        tr ++;
    }
}
double   Texture_currentFontSize(void) {
    return Font_currentSize_;
}

#pragma mark -- Globals, constructors... ----------------------------------------------------------*/
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
    // Essayer de loader la mini
    tex->mtlMini = MTLTexture_createImageForOpt_(tex, true);
    
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
    Font_currentAttributes_ = FontAttributes_createWithFont_(Font_current_);
    Font_currentMiniAttributes_ = FontAttributes_createWithFont_(Font_currentMini_);
    
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
    map_applyToAll(textureOfPngName_,        _texture_unsetPartly_);
    map_applyToAll(textureOfSharedString_, _texture_unsetPartly_);
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) { _texture_unsetPartly(*tr); }
        tr ++;
    }
    _Texture_isLoaded = false;
}
void     Texture_resume(void) {
    _Texture_isLoaded = true;
//#warning Superflu finalement ? Texture utilisees seront chargees si besoin dans la premiere frame.
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
            tex->mtlTexture = MTLTexture_createImageForOpt_(tex, false);
            if(chronochecker_elapsedMS(cc) > timesUp)
                return;
        }
    } while(map_iterator_next(textureOfPngName_));
    if(map_iterator_init(textureOfSharedString_)) do {
        Texture* tex = (Texture*)map_iterator_valueRefOpt(textureOfSharedString_);
        if(!_texture_isUsed(tex)) continue;
        if(tex->mtlTexture == nil) {
            tex->mtlTexture = MTLTexture_createStringForOpt_(tex, false);
            if(chronochecker_elapsedMS(cc) > timesUp)
                return;
        }
    } while(map_iterator_next(textureOfSharedString_));
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) if((*tr)->mtlTexture == nil) {
            (*tr)->mtlTexture = MTLTexture_createStringForOpt_(*tr, false);
            if(chronochecker_elapsedMS(cc) > timesUp)
                return;
        }
        tr ++;
    }
    _Texture_needToFullyDraw = false;
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


Texture* Texture_sharedImage(uint const pngId) {
    if(!textureOfPngId_) { printerror("pngs not loaded."); return &_Texture_dummy; }
    if(pngId >= pngCount_) {
        printerror("Invalid png id %d. Max id is %d.", pngId, pngCount_);
        return &_Texture_dummy;
    }
    Texture *tex = textureOfPngId_[pngId];
    tex->touchTime = CR_elapsedMS_;
    // S'il y a pas la mini, créer tout de suite la vrai texture.
    // (Normalement, s'il n'y a pas de mini c'est que la texture est petite...)
    if(tex->mtlMini == nil)
        tex->mtlTexture = MTLTexture_createImageForOpt_(tex, false);
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
    if(tex->mtlMini == nil)
        tex->mtlTexture = MTLTexture_createImageForOpt_(tex, false);
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
    // 0. Init des champs
    tex->ptu = ptu_default;
    tex->m =     1;   tex->n =     1;
    tex->alpha = 1.f; tex->beta =  1.f; tex->ratio = 1.f;
    tex->flags = tex_flag_string|(str.string_flags & tex_flags_string_);
    tex->string_ref_count = 1;
    tex->touchTime = CR_elapsedMS_;
    tex->string = String_createCopy(str.c_str);
    tex->string_fontOpt = str.fontNameOpt ? String_createCopy(str.fontNameOpt) : NULL;
    tex->string_color = str.color;
    // On ne dessine que la mini si la resolution est elevee. (Sinon on set la mtlTexture)
    if(Font_currentSize_ > 40)
        tex->mtlMini =    MTLTexture_createStringForOpt_(tex, true);
    else
        tex->mtlTexture = MTLTexture_createStringForOpt_(tex, false);
    
    // Garder une référence pour pause/resume.
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
    id<MTLTexture> mtlTex = MTLTexture_createStringForOpt_(tex, mini);
    if(mini) {
        tex->mtlMini = mtlTex;
        tex->mtlTexture = nil;
    } else {
        tex->mtlTexture = mtlTex;
        tex->mtlMini = nil;  // (N'est plus valide)
    }
    
    tex->touchTime = CR_elapsedMS_;
}

#pragma mark - Getters

id<MTLTexture>    texture_MTLTexture(Texture* tex) {
    tex->touchTime = CR_elapsedMS_;
    if(tex->mtlTexture != nil)
        return tex->mtlTexture;
    // Il y a des texture en demande pas encore "fully drawn"...
    _Texture_needToFullyDraw = true;
    if(tex->mtlMini != nil)
        return tex->mtlMini;
        
    // Ni mini ni standard ? Essayer de dessiner
    if(tex->flags & tex_flag_string) {
        tex->mtlMini = MTLTexture_createStringForOpt_(tex, true);
        return tex->mtlMini;
    }
    // Cas png pas de mini pas init.
    tex->mtlTexture = MTLTexture_createImageForOpt_(tex, false);
    return tex->mtlTexture;
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
