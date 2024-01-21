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
#include "coq_utils.h"
#include "graph__apple.h"
#include "graphs/graph_font_manager.h"

typedef enum {
    tex_flag_string           = 0x0001,
    tex_flag_stringLocalized  = 0x0002,
    tex_flag_stringMutable    = 0x0004,
    tex_flag_shared           = 0x0008,
    tex_flag_png              = 0x0010,
    tex_flag_png_coqlib       = 0x0020,
    tex_flag_nearest          = 0x0040,
} texflag_t;

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
    texflag_t        flags;
    int64_t          touchTime;
    char*            string;  // Copie de la String ou nom du png, pour s'il faut redessiner la texture.
    char*            fontNameOpt;
    Texture**        listRefererOpt; // Sa référence dans une liste.
                                // (i.e. pour les strings quelconques)
    // Metal
    id<MTLTexture>   mtlTexture;
    id<MTLTexture>   mtlMini;
} Texture;

static Texture      _Texture_dummy = {
    PTU_DEFAULT,
    1, 1,
    1.f, 1.f, 1.f,
    tex_flag_shared, 0,
    "dummy", NULL, NULL, nil, nil
};

/*-- Variable globales --*/
static int32_t           _Texture_total_count = 0;
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
static Vector2              Font_current_spreading_ = { 1.3f, 1.0f };

/*-- Creation de la texture Metal -------------------------------*/
static MTKTextureLoader* MTLtextureLoader_ = NULL;
NSMutableDictionary*     FontAttributes_createWithFont_(Font* font) {
#if TARGET_OS_OSX == 1
    NSColor* color = NSColor.whiteColor;
#else
    UIColor* color = UIColor.whiteColor;
#endif
    // 2. Paragraph style
    NSMutableParagraphStyle* paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    paragraphStyle.alignment = NSTextAlignmentCenter;
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
    // 3. Attributs de la string (color, font, paragraph style)
    NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
    [attributes setObject:font forKey:NSFontAttributeName];
    [attributes setObject:color forKey:NSForegroundColorAttributeName];
    [attributes setObject:paragraphStyle forKey:NSParagraphStyleAttributeName];
    paragraphStyle = nil;
    color = nil;
    return attributes;
}
static const CGFloat     Texture_y_string_rel_shift_ = -0.15;
void      _texture_setMTLtextureAsString(Texture* const tex, bool setMini) {
    if(((tex->mtlMini != nil) && setMini) || ((tex->mtlTexture) && !setMini)) {
        printwarning("Texture already set.");
        return;
    }
    // NSString
    NSString* nsstring = nil;
    if(tex->flags & tex_flag_stringLocalized) {
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
    if(tex->fontNameOpt) {
        font = [Font fontWithName:[NSString stringWithUTF8String:tex->fontNameOpt]
                                size:setMini ? 12 : Font_currentSize_];
        if(font == nil)
            printwarning("Cannot load font %s.", tex->fontNameOpt);
        else {
            spreading = Font_getFontInfoOf(tex->fontNameOpt)->spreading;
            attributes = FontAttributes_createWithFont_(font);
        }
    }
    if(font == nil) {
        font = setMini ? Font_currentMini_ : Font_current_;
        attributes = setMini ? Font_currentMiniAttributes_ : Font_currentAttributes_;
        spreading = Font_current_spreading_;
    }
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
    CGColorSpaceRelease(colorSpace);
    if(context == nil) {
        printerror("Cannot load CGContext");
        return;
    }
    // (lettres remplies avec le contour)
    CGContextSetTextDrawingMode(context, kCGTextFillStroke);
    // 7. Dessiner la string dans le context
    // (set context CoreGraphics dans context NSGraphics pour dessiner la NSString.)
#if TARGET_OS_OSX == 1
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
    [NSGraphicsContext restoreGraphicsState];
    nsgcontext = nil;
#else
    CGFloat strHeight = renderedSize.height;
    UIGraphicsPushContext(context);
    CGContextScaleCTM(context, 1.0, -1.0);
    CGPoint drawPoint = CGPointMake(
        0.5 * extraWidth,
        (0.5 - _Texture_y_string_rel_shift) * font.xHeight
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
    CGContextRelease(context);
    NSError *error = nil;
    id<MTLTexture> mtlTexture = [MTLtextureLoader_ newTextureWithCGImage:image options:nil error:&error];
    CGImageRelease(image);
    if(error != nil || mtlTexture == nil) {
        printerror("Cannot make MTLTexture of string %s with CGImage.", tex->string);
        return;
    }
    // Mettre à jour de la texture metal.
    if(setMini) {
        tex->mtlMini = mtlTexture;
    } else {
        tex->mtlTexture = mtlTexture;
    }
}
void      _texture_setMTLtextureAsPng(Texture* const tex, bool setMini) {
    if(((tex->mtlMini != nil) && setMini) || ((tex->mtlTexture) && !setMini)) {
        printwarning("Texture already set.");
        return;
    }
    NSString *NSpngName = [NSString stringWithUTF8String:tex->string];
    NSString *png_dir = (tex->flags & tex_flag_png_coqlib) ? @"pngs_coqlib" : @"pngs";
    if(setMini) png_dir = [png_dir stringByAppendingString:@"/minis"];
    NSURL *pngUrl = [[NSBundle mainBundle] URLForResource:NSpngName
                                            withExtension:@"png"
                                             subdirectory:png_dir];
    png_dir = nil;
    NSpngName = nil;
    if(pngUrl == NULL) {
        // (Ok, si pas de mini, mais probleme si pas la texture standard)
        if(!setMini) {
            printerror("cannot init url for non-mini %s.", tex->string);
        }
        return;
    }
//    tex->flags |= setMini ? tex_flag_miniDrawn : tex_flag_fullyDrawn;
    NSError *error = nil;
    id<MTLTexture> mtlTexture = [MTLtextureLoader_ newTextureWithContentsOfURL:pngUrl
                                       options:@{MTKTextureLoaderOptionSRGB: @true}
                                         error:&error];
    if(error != nil || mtlTexture == nil) {
        printerror("cannot create MTL texture for %s png.", tex->string);
        mtlTexture = nil;
        return;
    }
    // Mettre à jour les variables de la texture.
    if(setMini) {
        tex->mtlMini = mtlTexture;
    } else {
        tex->mtlTexture = mtlTexture;
    }
    tex->ptu.width =  (float)[mtlTexture width];
    tex->ptu.height = (float)[mtlTexture height];
    tex->ratio = tex->ptu.width / tex->ptu.height * tex->ptu.n / tex->ptu.m;
}

/*-- Fonctions privees sur une instance Texture -------------------------*/
void  texture_initAsPng_(Texture* const tex, const PngInfo* const info) {
    // Init des champs
    tex->ptu = (PerTextureUniforms) { 8, 8, (float)info->m, (float)info->n };
    tex->m =     info->m; tex->n =     info->n;
    tex->alpha = 1.f;     tex->beta =  1.f;
    tex->flags = tex_flag_png|tex_flag_shared|(info->isCoqlib ? tex_flag_png_coqlib : 0)
                    |(info->nearest ? tex_flag_nearest : 0);
    tex->touchTime = _CR_elapsedMS - TEX_UNUSED_DELTATMS_;
    tex->ratio = (float)info->n / (float)info->m;  // (temporaire)
    tex->string = String_createCopy(info->name);
    tex->mtlTexture = nil;
    tex->mtlMini = nil;
    // Charger tout de suite la mini (si possible)
    _texture_setMTLtextureAsPng(tex, true);
    
    _Texture_total_count ++;
}
void  texture_initAsString_(Texture* const tex, texflag_t flags,
          const char* c_str, const char* fontNameOpt) {
    // 0. Init des champs
    tex->ptu = ptu_default;
    tex->m =     1;   tex->n =     1;
    tex->alpha = 1.f; tex->beta =  1.f;
    tex->flags = flags|tex_flag_string;
    tex->ratio = 1.f;
    tex->string = String_createCopy(c_str);
    tex->fontNameOpt = fontNameOpt ? String_createCopy(fontNameOpt) : NULL;
    tex->mtlMini = nil;
    tex->mtlTexture = nil;
    
    _Texture_total_count ++;
}
/// Pour s'assurer qu'il y a au moins une mini quand on demande la texture.
void  _texture_drawPartly(Texture* const tex) {
    if(tex->mtlTexture != nil) return;
    if(tex->mtlMini != nil) return;
    if(tex->flags & tex_flag_string) {
        // On ne dessine que la mini si la resolution est elevee.
        _texture_setMTLtextureAsString(tex, Font_currentSize_ > 40);
        return;
    }
    // Png: si pas de mini (créé dans _texture_initAsPng),
    // c'est qu'il faut créer la texture standard.
    // (Normalement, s'il n'y a pas de mini c'est que la texture est petite...)
    _texture_setMTLtextureAsPng(tex, false);
}
/// Pour dessiner la vrai texture (grosse) quand on a le temps...
void  _texture_drawFully(Texture* const tex) {
    if(tex->mtlTexture != nil) return;
    if(tex->flags & tex_flag_string) {
        _texture_setMTLtextureAsString(tex, false);
        return;
    }
    _texture_setMTLtextureAsPng(tex, false);
}
bool  _texture_isUsed(Texture* const tex) {
    return _CR_elapsedMS - tex->touchTime <= TEX_UNUSED_DELTATMS_;
}
bool  _texture_isUnused(Texture* const tex) {
    return _CR_elapsedMS - tex->touchTime > TEX_UNUSED_DELTATMS_ + 30;
}
void  _texture_deinit(void* tex_void) {
    Texture* tex = tex_void;
    tex->mtlTexture = nil;
    tex->mtlMini = nil;
    coq_free(tex->string);
    tex->string = NULL;
    // Déréferencer...
    if(tex->listRefererOpt)
        *(tex->listRefererOpt) = (Texture*)NULL;
    if(tex->fontNameOpt) {
        coq_free(tex->fontNameOpt);
        tex->fontNameOpt = NULL;
    }
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

/*-- Fonctions public sur une instance Texture -------------------------*/
void              texture_updateString(Texture* tex, const char* newString) {
    if(!(tex->flags & tex_flag_stringMutable) || !newString) {
        printwarning("Not a mutable string or missing newString.");
        return;
    }
    coq_free(tex->string);
    tex->string = String_createCopy(newString);
    tex->mtlTexture = nil;
    tex->mtlMini = nil;
    _texture_drawPartly(tex);
    tex->touchTime = _CR_elapsedMS;
}
void              texture_destroy(Texture* tex) {
    _texture_deinit(tex);
    coq_free(tex);
    _Texture_total_count --;
}
id<MTLTexture>    texture_MTLTexture(Texture* tex) {
    tex->touchTime = _CR_elapsedMS;
    if(tex->mtlTexture != nil)
        return tex->mtlTexture;
    // Il y a des texture en demande pas encore "fully drawn"...
    _Texture_needToFullyDraw = true;
    if(tex->mtlMini != nil)
        return tex->mtlMini;
    // Ni mini ni standard ? Essayer de dessiner
    _texture_drawPartly(tex);
    if(tex->mtlTexture != nil)
        return tex->mtlTexture;
    return tex->mtlMini;
}
bool              texture_isNearest(Texture* tex) {
    return tex->flags & tex_flag_nearest;
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
bool              texture_isShared(Texture* tex) {
    return tex->flags & tex_flag_shared;
}
const char*       texture_string(Texture* tex) {
    return tex->string;
}



/*-- List de textures -----------------------------------------------------*/
/// Maps des png (shared).
static StringMap*        _textureOfPngName = NULL;
/// Maps des constant strings (shared)
static StringMap*        _textureOfConstantString = NULL;
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
    Font_currentMini_ = [Font_current_ fontWithSize:12];
    [Font_currentAttributes_ setObject:Font_current_ forKey:NSFontAttributeName];
    [Font_currentMiniAttributes_ setObject:Font_currentMini_ forKey:NSFontAttributeName];
    Font_current_spreading_ = Font_getFontInfoOf(fontName)->spreading;
    // Redessiner les strings...
    map_applyToAll(_textureOfConstantString, _texture_unsetFully_);
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
    map_applyToAll(_textureOfConstantString, _texture_unsetPartly_);
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) { _texture_unsetPartly(*tr); }
        tr ++;
    }
}
double   Texture_currentFontSize(void) {
    return Font_currentSize_;
}

/// Liste des pngs par defaut
const PngInfo _coqlib_pngInfos[] = {
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
const uint32_t _coqlib_png_count = sizeof(_coqlib_pngInfos) / sizeof(PngInfo);

static Texture**         _textureOfPngId = NULL; // Liens vers la map.
static uint              _pngCount = 0;

/*-- Init, constructors... ----------------------------------------------------------*/

void     Texture_init(id<MTLDevice> const device) {
    if(_Texture_isInit) {
        printerror("Texture already init.");
        return;
    }
    // 1. Texture loader et font.
    MTLtextureLoader_ = [[MTKTextureLoader alloc] initWithDevice:device];
    Font_current_ = [Font systemFontOfSize:Font_currentSize_];
    Font_currentMini_ = [Font systemFontOfSize:12];
    Font_currentAttributes_ = FontAttributes_createWithFont_(Font_current_);
    Font_currentMiniAttributes_ = FontAttributes_createWithFont_(Font_currentMini_);
    
    _textureOfConstantString = Map_create(64, sizeof(Texture));
    _textureOfPngName =        Map_create(64, sizeof(Texture));
    memset(_nonCstStrTexArray, 0, sizeof(Texture*) * _nonCstStrTexCount);
    // Init des png de base.
    const PngInfo* info =           _coqlib_pngInfos;
    const PngInfo* const infoEnd = &_coqlib_pngInfos[_coqlib_png_count];
    while(info < infoEnd) {
        Texture* tex = (Texture*)map_put(_textureOfPngName, info->name, NULL);
        texture_initAsPng_(tex, info);
        info ++;
    }
    _Texture_isInit = true;
    _Texture_isLoaded = true;
}
void     Texture_loadPngs(PngInfo const pngInfos[], const uint pngCount) {
    if(!_Texture_isInit) {
        printerror("Texture not init."); return;
    }
    if(_textureOfPngId) {
        printerror("pngs already loaded"); return;
    }
    _textureOfPngId = coq_calloc(pngCount, sizeof(Texture*));
    _pngCount = pngCount;
    const PngInfo *info =     pngInfos;
    const PngInfo *infoEnd = &pngInfos[pngCount];
    uint32_t pngId = 0;
    while(info < infoEnd) {
        Texture* tex = (Texture*)map_put(_textureOfPngName, info->name, NULL);
        texture_initAsPng_(tex, info);
        _textureOfPngId[pngId] = tex;
        info ++;
        pngId ++;
    }
}
void     Texture_suspend(void) {
    if(!_Texture_isInit || !_Texture_isLoaded) return;
    map_applyToAll(_textureOfPngName,        _texture_unsetPartly_);
    map_applyToAll(_textureOfConstantString, _texture_unsetPartly_);
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

static  uint32_t _Texture_checkunset_counter = 0;
void     Texture_checkToFullyDrawAndUnused(ChronoChecker* cc, int64_t timesUp) {
    // Une fois de temps en temps, quand on a le temps, liberer les png non utilises.
    if(!_Texture_needToFullyDraw) {
        _Texture_checkunset_counter = (_Texture_checkunset_counter + 1) % 30;
        if(_Texture_checkunset_counter == 0)
            map_applyToAll(_textureOfPngName, _texture_unsetUnusedPartly_);
        return;
    }
    // Chercher les png, `const string`, `non const string` à dessiner au complet...
    if(map_iterator_init(_textureOfPngName)) do {
        Texture* tex = (Texture*)map_iterator_valueRefOpt(_textureOfPngName);
        if(!_texture_isUsed(tex)) continue;
        if(tex->mtlTexture == nil) {
            _texture_drawFully(tex);
            if(chronochecker_elapsedMS(cc) > timesUp)
                return;
        }
    } while(map_iterator_next(_textureOfPngName));
    if(map_iterator_init(_textureOfConstantString)) do {
        Texture* tex = (Texture*)map_iterator_valueRefOpt(_textureOfConstantString);
        if(!_texture_isUsed(tex)) continue;
        if(tex->mtlTexture == nil) {
            _texture_drawFully(tex);
            if(chronochecker_elapsedMS(cc) > timesUp)
                return;
        }
    } while(map_iterator_next(_textureOfConstantString));
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) if((*tr)->mtlTexture == nil) {
            _texture_drawFully(*tr);
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
    map_destroyAndNull(&_textureOfConstantString, _texture_deinit);
    map_destroyAndNull(&_textureOfPngName,        _texture_deinit);
    if(_textureOfPngId)
        coq_free(_textureOfPngId);
    _pngCount = 0;
}

Texture* Texture_sharedImage(uint const pngId) {
    if(!_textureOfPngId) { printerror("pngs not loaded."); return &_Texture_dummy; }
    if(pngId >= _pngCount) {
        printerror("Invalid png id %d. Max id is %d.", pngId, _pngCount);
        return &_Texture_dummy;
    }
    Texture *tex = _textureOfPngId[pngId];
    tex->touchTime = _CR_elapsedMS;
    _texture_drawPartly(tex);
    return tex;
}
Texture* Texture_sharedImageByName(const char* pngName) {
    Texture* tex = (Texture*)map_valueRefOptOfKey(_textureOfPngName, pngName);
    if(!tex) {
        printerror("No png %s found.", pngName);
        return &_Texture_dummy;
    }
    tex->touchTime = _CR_elapsedMS;
    _texture_drawPartly(tex);
    return tex;
}
/// Une string constante partagée avec la font par defaut.
Texture* Texture_sharedConstantString(const char* c_str) {
    if(!_Texture_isInit) {
        printerror("Texture not init.");
        return &_Texture_dummy;
    }
    Texture* tex = (Texture*)map_valueRefOptOfKey(_textureOfConstantString, c_str);
    if(tex != NULL)
        return tex;
    // Creer une nouvelle texture.
    tex = (Texture*)map_put(_textureOfConstantString, c_str, NULL);
    texture_initAsString_(tex, tex_flag_shared, c_str, NULL);
    tex->touchTime = _CR_elapsedMS;
    _texture_drawPartly(tex);
    return tex;
}
void     _texture_addToNonConstantStrReferers(Texture* tex) {
    Texture** const referer = _nonCstStrCurrent;
    if(*referer) {
        printwarning("Texture referer space occupied.");
    } else {
        // Référencer dans la liste des "non constant strings".
        *referer = tex;
        tex->listRefererOpt = referer;
    }
    // Cherche un autre disponible.
    _nonCstStrCurrent++;
    while(_nonCstStrCurrent < _nonCstStrArrEnd) {
        if(*_nonCstStrCurrent == NULL)
            return;
        _nonCstStrCurrent ++;
    }
    _nonCstStrCurrent = _nonCstStrTexArray;
    while(_nonCstStrCurrent < referer) {
        if(*_nonCstStrCurrent == NULL)
            return;
        _nonCstStrCurrent ++;
    }
    printwarning("Texture referer space not found.");
}
// Création d'une string quelconque (mutable, localisable, avec font...)
Texture* Texture_createString(UnownedString str, bool isShared) {
    Texture* tex = coq_calloc(1, sizeof(Texture));
    if(!_Texture_isInit) {
        printerror("Texture not init.");
        *tex = _Texture_dummy;
        return tex;
    }
    // Owned / non-shared string.
    texflag_t flags = (isShared ? tex_flag_shared : 0)
      |(str.isLocalized ? tex_flag_stringLocalized : tex_flag_stringMutable);
    texture_initAsString_(tex, flags,
                          str.c_str, str.fontNameOpt);
    _texture_addToNonConstantStrReferers(tex);
    tex->touchTime = _CR_elapsedMS;
    _texture_drawPartly(tex);
    return tex;
}

