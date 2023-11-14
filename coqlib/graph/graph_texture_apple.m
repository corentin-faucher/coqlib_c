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

#include "graph__apple.h"

#include "utils.h"
#include "map.h"
#include "language.h"
#include "string_utils.h"
#include "font_manager.h"

typedef enum _Texture_flag {
    tex_flag_string           = 0x0001,
    tex_flag_stringLocalized  = 0x0002,
    tex_flag_stringMutable    = 0x0004,
    tex_flag_shared           = 0x0008,
    tex_flag_png              = 0x0010,
    tex_flag_png_coqlib       = 0x0020,
    tex_flag_miniDrawn        = 0x0040,
    tex_flag_fullyDrawn       = 0x0080,
    tex_flag_miniOrFullyDrawn = tex_flag_fullyDrawn|tex_flag_miniDrawn,
} texflag_t;

#define _TEX_UNUSED_DELTATMS 1000

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
    Texture**        nonSharedReferer;
    // Metal
    id<MTLTexture>   mtlTexture;
    id<MTLTexture>   mtlMini;
} Texture;

static Texture      _Texture_dummy = {
    PTU_DEFAULT,
    1, 1,
    1.f, 1.f, 1.f,
    tex_flag_shared, 0,
    "dummy", NULL, NULL, nil
};

static int32_t           _Texture_total_count = 0;
static MTKTextureLoader* _MTLtextureLoader = NULL;
static Bool              _Texture_isInit = false;
static Bool              _Texture_isLoaded = false;
volatile static Bool     _Texture_needToFullyDraw = false;

/*-- Png drawing -------------------------------------------------------------------*/
id<MTLTexture> _MTLTexture_createPng(const char* pngName, Bool asCoqlib, Bool asMini) {
    NSString *NSpngName = [NSString stringWithUTF8String:pngName];
    NSString *png_dir = asCoqlib ? @"coqlib_pngs" : @"pngs";
    if(asMini) png_dir = [png_dir stringByAppendingString:@"/minis"];
    NSURL *pngUrl = [[NSBundle mainBundle] URLForResource:NSpngName
                                            withExtension:@"png"
                                             subdirectory:png_dir];
    png_dir = nil;
    NSpngName = nil;
    if(pngUrl == NULL) {
        if(!asMini) printerror("cannot init url for non-mini %s.", pngName);
        return nil;
    }
    NSError *error = nil;
    id<MTLTexture> mtlTexture = [_MTLtextureLoader newTextureWithContentsOfURL:pngUrl
                                       options:@{MTKTextureLoaderOptionSRGB: @false}
                                         error:&error];
    if(error != nil || mtlTexture == nil) {
        printerror("cannot load %s png.", pngName); return nil;
    }
    return mtlTexture;
}
void  _texture_initAsPng(Texture* const tex, const PngInfo* const info, Bool isCoqlib) {
    // Init des champs
    tex->ptu = (PerTextureUniforms) { 8, 8, (float)info->m, (float)info->n };
    tex->m =     info->m; tex->n =     info->n;
    tex->alpha = 1.f;     tex->beta =  1.f;
    tex->flags = tex_flag_png|tex_flag_shared|(isCoqlib ? tex_flag_png_coqlib : 0);
    tex->touchTime = _CR_elapsedMS - _TEX_UNUSED_DELTATMS;
    // (on a pas tex_flag_dimsInit, il faut charger le png pour avoir width et height)
    tex->ratio = (float)info->n / (float)info->m;  // (temporaire)
    tex->string = string_createCopy(info->name);
    tex->mtlMini = _MTLTexture_createPng(info->name, isCoqlib, true);
    tex->mtlTexture = nil; // Pas encore besoin.
    _Texture_total_count ++;
}

/*-- String drawing ----------------------------------------------------------------*/
/*-- Font -------------------------------------------------------------*/
#if TARGET_OS_OSX == 1
typedef NSFont Font;
#else
typedef UIFont Font;
#endif
static Font*                _Font_current = nil;
static Font*                _Font_currentMini = nil;
static NSMutableDictionary* _Font_currentAttributes = nil;
static NSMutableDictionary* _Font_currentMiniAttributes = nil;
static double               _Font_currentSize = 24;
static Vector2              _Font_current_spreading = { 1.3f, 1.0f };
void     Texture_setCurrentFont(const char* fontName) {
    if(fontName == NULL) {
        printerror("No fontName.");
        return;
    }
    Font* newFont = [Font fontWithName:[NSString stringWithUTF8String:fontName]
                                 size:_Font_currentSize];
    if(newFont == nil) {
        printerror("Font %s not found.", fontName);
        return;
    }
    _Font_current = newFont;
    _Font_currentMini = [_Font_current fontWithSize:12];
    [_Font_currentAttributes setObject:_Font_current forKey:NSFontAttributeName];
    [_Font_currentMiniAttributes setObject:_Font_currentMini forKey:NSFontAttributeName];
    _Font_current_spreading = Font_getFontInfoOf(fontName)->spreading;
}
void     Texture_setCurrentFontSize(double newSize) {
    _Font_currentSize = newSize;
    if(_Font_current == nil) {
        return;
    }
    _Font_current = [_Font_current fontWithSize:newSize];
    [_Font_currentAttributes setObject:_Font_current forKey:NSFontAttributeName];
}
NSMutableDictionary* _FontAttributes_createWithFont(Font* font) {
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
/*-- Drawing String with Metal----------------------------------------------------------*/
static const CGFloat _Texture_y_string_rel_shift = -0.15;
id<MTLTexture>       _MTLTexture_createAndSetStringTexture(Texture* const tex, Bool asMini) {
    // NSString
    NSString* nsstring = nil;
    if(tex->flags & tex_flag_stringLocalized) {
        char* localized = string_createLocalized(tex->string);
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
                                size:asMini ? 12 : _Font_currentSize];
        if(font == nil)
            printwarning("Cannot load font %s.", tex->fontNameOpt);
        else {
            spreading = Font_getFontInfoOf(tex->fontNameOpt)->spreading;
            attributes = _FontAttributes_createWithFont(font);
        }
    }
    if(font == nil) {
        font = asMini ? _Font_currentMini : _Font_current;
        attributes = asMini ? _Font_currentMiniAttributes : _Font_currentAttributes;
        spreading = _Font_current_spreading;
    }
    // Dimensions de la string
    CGSize renderedSize = [nsstring sizeWithAttributes:attributes];
    CGFloat extraWidth = 0.55 * spreading.w * font.xHeight;
    CGFloat contextHeight = 2.00 * spreading.h * font.xHeight;
    CGFloat contextWidth =  ceil(renderedSize.width) + extraWidth;
    // (Mise à jour alpha/beta)
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
        return nil;
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
        (_Texture_y_string_rel_shift - 0.5) * font.xHeight
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
    id<MTLTexture> mtlTexture = [_MTLtextureLoader newTextureWithCGImage:image options:nil error:&error];
    CGImageRelease(image);
    if(error != nil || mtlTexture == nil) {
        printerror("Cannot make MTLTexture with CGImage.");
        return nil;
    }
    return mtlTexture;
}
void  _texture_initAsString(Texture* const tex, texflag_t flags,
          const char* c_str, const char* fontNameOpt) {
    // 0. Init des champs
    tex->ptu = ptu_default;
    tex->m =     1;   tex->n =     1;
    tex->alpha = 1.f; tex->beta =  1.f;
    tex->flags = flags|tex_flag_string;
    tex->ratio = 1.f;
    tex->string = string_createCopy(c_str);
    tex->fontNameOpt = fontNameOpt ? string_createCopy(fontNameOpt) : NULL;
    tex->mtlMini = nil;
    tex->mtlTexture = nil;
    _Texture_total_count ++;
}

/*-- Sub-functions... -------------------------------------------*/
void  _texture_drawPartly(Texture* const tex) {
    if(tex->flags & tex_flag_miniOrFullyDrawn) // Deja dessine.
        return;
    Bool asMiniTmp;
    if(tex->flags & tex_flag_string) {
        asMiniTmp = _Font_currentSize > 40;
        tex->mtlTexture = _MTLTexture_createAndSetStringTexture(tex, asMiniTmp);
    } else {
        if(tex->mtlMini) {
            tex->mtlTexture = tex->mtlMini;
            asMiniTmp = true;
        } else {
            tex->mtlTexture = _MTLTexture_createPng(tex->string, tex->flags & tex_flag_png_coqlib, false);
            asMiniTmp = false;
        }
    }
    tex->ptu.width =  (float)[tex->mtlTexture width];
    tex->ptu.height = (float)[tex->mtlTexture height];
    tex->ratio = tex->ptu.width / tex->ptu.height * tex->ptu.n / tex->ptu.m;
    
    tex->flags |= asMiniTmp ? tex_flag_miniDrawn : tex_flag_fullyDrawn;
}
void  _texture_drawFully(Texture* const tex) {
    if(tex->flags & tex_flag_fullyDrawn)
        return;
    if(tex->flags & tex_flag_string)
        tex->mtlTexture = _MTLTexture_createAndSetStringTexture(tex, false);
    else
        tex->mtlTexture = _MTLTexture_createPng(tex->string, tex->flags & tex_flag_png_coqlib, false);
    if(!tex->mtlTexture) {
        printerror("Cannot draw texture %s.", tex->string);
        tex->mtlTexture = tex->mtlMini;
        return;
    }
    tex->ptu.width =  (float)[tex->mtlTexture width];
    tex->ptu.height = (float)[tex->mtlTexture height];
    tex->ratio = tex->ptu.width / tex->ptu.height * tex->ptu.n / tex->ptu.m;
    
    tex->flags |= tex_flag_fullyDrawn;
}
void  _texture_unset(Texture* const tex) {
    tex->mtlTexture = nil;
    tex->flags &= ~tex_flag_miniOrFullyDrawn;
}
Bool  _texture_isUsed(Texture* const tex) {
    return _CR_elapsedMS - tex->touchTime <= _TEX_UNUSED_DELTATMS;
}
Bool  _texture_isUnused(Texture* const tex) {
    return _CR_elapsedMS - tex->touchTime > _TEX_UNUSED_DELTATMS + 30;
}
void  _texture_deinit(void* tex_void) {
    Texture* tex = tex_void;
    tex->mtlTexture = nil;
    tex->mtlMini = nil;
    coq_free(tex->string);
    tex->string = NULL;
    if(tex->nonSharedReferer)
        *(tex->nonSharedReferer) = (Texture*)NULL;
    if(tex->fontNameOpt) {
        coq_free(tex->fontNameOpt);
        tex->fontNameOpt = NULL;
    }
}
void  _map_block_texture_unset(char* tex_data) {
    _texture_unset((Texture*)tex_data);
}
void  _map_block_texture_unsetUnused(char* tex_data) {
    Texture* tex = (Texture*)tex_data;
    if(!(tex->flags & tex_flag_miniOrFullyDrawn)) return;
    if(!_texture_isUnused(tex)) return;
    // (n'est pas utilise)
    _texture_unset(tex);
}


/// Maps des strings.
static StringMap*        _textureOfPngName = NULL;
static StringMap*        _textureOfConstantString = NULL;

static const uint32_t    _nonSharedTexCount = 64;
static Texture*          _nonSharedStrTexArray[_nonSharedTexCount];
static Texture** const   _nonSharedEnd = &_nonSharedStrTexArray[_nonSharedTexCount];
static Texture**         _nonSharedCurrent = _nonSharedStrTexArray;

/// Liste des pngs par defaut
const PngInfo _coqlib_pngInfos[] = {
    {"coqlib_scroll_bar_back", 1, 3},
    {"coqlib_scroll_bar_front", 1, 3},
    {"coqlib_sliding_menu_back", 1, 1},
    {"coqlib_sparkle_stars", 3, 2},
    {"coqlib_switch_back", 1, 1},
    {"coqlib_switch_front", 1, 1},
    {"coqlib_test_frame", 1, 1},
    {"coqlib_the_cat", 1, 1},
    {"coqlib_transparent", 1, 1},
    {"coqlib_white", 1, 1},
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
    _MTLtextureLoader = [[MTKTextureLoader alloc] initWithDevice:device];
    _Font_current = [Font systemFontOfSize:_Font_currentSize];
    _Font_currentMini = [Font systemFontOfSize:12];
    _Font_currentAttributes = _FontAttributes_createWithFont(_Font_current);
    _Font_currentMiniAttributes = _FontAttributes_createWithFont(_Font_currentMini);
    
    _textureOfConstantString = Map_create(64, sizeof(Texture));
    _textureOfPngName =        Map_create(64, sizeof(Texture));
    memset(_nonSharedStrTexArray, 0, sizeof(Texture*) * _nonSharedTexCount);
    // Init des png de base.
    const PngInfo* info =           _coqlib_pngInfos;
    const PngInfo* const infoEnd = &_coqlib_pngInfos[_coqlib_png_count];
    while(info < infoEnd) {
        Texture* tex = (Texture*)map_put(_textureOfPngName, info->name, NULL);
        _texture_initAsPng(tex, info, true);
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
        _texture_initAsPng(tex, info, false);
        _textureOfPngId[pngId] = tex;
        info ++;
        pngId ++;
    }
}
void     Texture_suspend(void) {
    if(!_Texture_isInit || !_Texture_isLoaded) return;
    map_applyToAll(_textureOfConstantString, _map_block_texture_unset);
    map_applyToAll(_textureOfPngName,        _map_block_texture_unset);
    Texture** tr = _nonSharedStrTexArray;
    while(tr < _nonSharedEnd) {
        if(*tr) _map_block_texture_unset((char*)*tr);
        tr ++;
    }
    _Texture_isLoaded = false;
}
void     Texture_resume(void) {
    _Texture_isLoaded = true;
#warning Superflu finalement ? Texture utilisees seront chargees si besoin dans la premiere frame.
//    if(!_Texture_isInit || _Texture_isLoaded) return;
    // Pas besoin, ceux qui on besoin seront redessiner...
//    map_applyToAll(_textureOfConstantString, _map_block_texture_redrawAsString);
//    map_applyToAll(_textureOfPngName,        _map_block_texture_redrawAsPng);
//    Texture** tr = _nonSharedStrTexArray;
//    while(tr < _nonSharedEnd) {
//#warning Besoin ?
//        if(*tr) _texture_drawPartlyString(*tr);
//        tr ++;
//    }
//    _Texture_isLoaded = true;
}
void     Texture_checkToFullyDraw(ChronoChecker* cc) {
    if(!_Texture_needToFullyDraw) return;
    if(map_iterator_init(_textureOfPngName)) do {
        Texture* tex = (Texture*)map_iterator_valueRefOpt(_textureOfPngName);
        if(!_texture_isUsed(tex)) continue;
        if(!(tex->flags & tex_flag_fullyDrawn)) {
            _texture_drawFully(tex);
            if(_chronochceker_elapsedMS(cc) > Chrono_UpdateDeltaTMS) {
                return;
            }
        }
    } while(map_iterator_next(_textureOfPngName));
    if(map_iterator_init(_textureOfConstantString)) do {
        Texture* tex = (Texture*)map_iterator_valueRefOpt(_textureOfConstantString);
        if(!_texture_isUsed(tex)) continue;
        if(!(tex->flags & tex_flag_fullyDrawn)) {
            _texture_drawFully(tex);
            if(_chronochceker_elapsedMS(cc) > Chrono_UpdateDeltaTMS) {
                return;
            }
        }
    } while(map_iterator_next(_textureOfConstantString));
    Texture** tr = _nonSharedStrTexArray;
    while(tr < _nonSharedEnd) {
        if(*tr) if(!((*tr)->flags & tex_flag_fullyDrawn)) {
            _texture_drawFully(*tr);
            if(_chronochceker_elapsedMS(cc) > Chrono_UpdateDeltaTMS) {
                return;
            }
        }
        tr ++;
    }
    _Texture_needToFullyDraw = false;
}
void     Texture_checkUnused(void) {
    map_applyToAll(_textureOfPngName, _map_block_texture_unsetUnused);
}
void     _Texture_deinit(void) {
    _Texture_isInit = false;
    _Texture_isLoaded = false;
    _MTLtextureLoader = nil;
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
    _texture_initAsString(tex, tex_flag_shared, c_str, NULL);
    tex->touchTime = _CR_elapsedMS;
    _texture_drawPartly(tex);
    return tex;
}
void     _texture_addToNonSharedStrReferers(Texture* tex) {
    Texture** const referer = _nonSharedCurrent;
    if(*referer) {
        printwarning("Texture referer space occupied.");
    } else {
        *referer = tex;
        tex->nonSharedReferer = referer;
    }
    // Cherche un autre disponible.
    _nonSharedCurrent++;
    while(_nonSharedCurrent < _nonSharedEnd) {
        if(*_nonSharedCurrent == NULL)
            return;
        _nonSharedCurrent ++;
    }
    _nonSharedCurrent = _nonSharedStrTexArray;
    while(_nonSharedCurrent < referer) {
        if(*_nonSharedCurrent == NULL)
            return;
        _nonSharedCurrent ++;
    }
    printwarning("Texture referer space not found.");
}
Texture* Texture_createString(UnownedString str) {
    Texture* tex = coq_calloc(1, sizeof(Texture));
    if(!_Texture_isInit) {
        printerror("Texture not init.");
        *tex = _Texture_dummy;
        return tex;
    }
    // Owned / non-shared string.
    _texture_initAsString(tex, str.isLocalized ? tex_flag_stringLocalized : tex_flag_stringMutable,
                          str.c_str, str.fontNameOpt);
    _texture_addToNonSharedStrReferers(tex);
    tex->touchTime = _CR_elapsedMS;
    _texture_drawPartly(tex);
    return tex;
}
void     texture_updateString(Texture* tex, const char* newString) {
    if(!(tex->flags & tex_flag_stringMutable) || !newString) {
        printwarning("Not a mutable string or missing newString.");
        return;
    }
    coq_free(tex->string);
    tex->string = string_createCopy(newString);
    tex->touchTime = _CR_elapsedMS;
    _texture_unset(tex);
    _texture_drawPartly(tex);
}

void     texture_destroy(Texture* tex) {
    _texture_deinit(tex);
    coq_free(tex);
    _Texture_total_count --;
    printf("🐰 destr tex Total texture %d.\n", _Texture_total_count);
}


/*-- Getters --*/

id<MTLTexture>    texture_MTLTexture(Texture* tex) {
    tex->touchTime = _CR_elapsedMS;
    if(!(tex->flags & tex_flag_miniOrFullyDrawn))
        _texture_drawPartly(tex);
    if(!(tex->flags & tex_flag_fullyDrawn))
        _Texture_needToFullyDraw = true;
    return tex->mtlTexture;
}
const PerTextureUniforms* texture_ptu(Texture* tex) {
    return &tex->ptu;
}
uint              texture_m(Texture *tex) {
    return tex->m;
}
uint              texture_n(Texture *tex) {
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
Bool              texture_isShared(Texture* tex) {
    return tex->flags & tex_flag_shared;
}
