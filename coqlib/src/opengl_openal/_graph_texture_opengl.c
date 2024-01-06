//
//  graph_texture_opengl.m
//  Version OpenGL pour les texture.
//  On utilise lodepng de Lode Vandevenne pour charger les pngs.
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "lodepng.h"
#include "coq_map.h"
#include "_graph/_graph__opengl.h"
#include "_utils/_utils_file.h"

typedef enum _Texture_flag {
    tex_flag_string           = 0x0001,
    tex_flag_stringLocalized  = 0x0002,
    tex_flag_stringMutable    = 0x0004,
    tex_flag_shared           = 0x0008,
    tex_flag_png              = 0x0010,
    tex_flag_png_coqlib       = 0x0020,
    tex_flag_miniDrawn        = 0x0040,
    tex_flag_fullyDrawn       = 0x0080,
    tex_flag_nearest          = 0x0100,
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
    Texture**        listRefererOpt; // Sa référence dans une liste.
                                // (i.e. pour les strings quelconques)
    /// Id OpenGL
    GLuint           texture_mini_id;
    GLuint           texture_id;
} Texture;

static Texture      _Texture_dummy = {
    PTU_DEFAULT,
    1, 1,
    1.f, 1.f, 1.f,
    tex_flag_shared, 0,
    "dummy", NULL, NULL, 0, 0
};

static int32_t           _Texture_total_count = 0;
static bool              _Texture_isInit = false;
static bool              _Texture_isLoaded = false;
volatile static bool     _Texture_needToFullyDraw = false;
/*-- Font -------------------------------------------------------------*/


// TODO


// typedef NSFont Font;
// static Font*                _Font_current = NULL;
// static Font*                _Font_currentMini = NULL;

static double               _Font_currentSize = 24;
// static Vector2              _Font_current_spreading = {{ 1.3f, 1.0f }};

/*-- List/Map de textures -----------------------------------------------*/
/// Maps des png (shared).
static StringMap*        _textureOfPngName = NULL;
/// Maps des constant strings (shared)
static StringMap*        _textureOfConstantString = NULL;
// Liste des strings quelconques...
static const uint32_t    _nonCstStrTexCount = 64;
static Texture*          _nonCstStrTexArray[_nonCstStrTexCount];
static Texture** const   _nonCstStrArrEnd = &_nonCstStrTexArray[_nonCstStrTexCount];
static Texture**         _nonCstStrCurrent = _nonCstStrTexArray;
/*-- Sub-functions... -------------------------------------------*/
// static const float       _Texture_y_string_rel_shift = -0.15;
void     _texture_setGlTextureAsString(Texture* tex, bool asMini) {
    


    // TODO




    printwarning("TODO ...");

    tex->flags |= asMini ? tex_flag_miniDrawn : tex_flag_fullyDrawn;
}

void    _texture_setGlTextureAsPng(Texture* tex, bool asMini) {
    GLuint* texture_id_ref = asMini ? &tex->texture_mini_id : &tex->texture_id;
    if(*texture_id_ref != 0) {
        printwarning("Texture already set.");
        // (mettre le flag manquant ?...)
        tex->flags |= asMini ? tex_flag_miniDrawn : tex_flag_fullyDrawn;
        return;
    }
    const char* png_dir;
    if(tex->flags & tex_flag_png_coqlib) {
        png_dir = asMini ? "pngs_coqlib/minis" : "pngs_coqlib";
    } else {
        png_dir = asMini ? "pngs/minis" : "pngs";
    }
    const char* png_path = FileManager_getResourcePathOpt(tex->string, "png", png_dir);
    unsigned char* pixels;
    unsigned int width, height;
    // Chargement des pixels avec LodePNG de Lode Vandevenne
    lodepng_decode32_file(&pixels, &width, &height, png_path);
    if(!pixels) { 
        if(!asMini) {
            printerror("Cannot load png %s.", tex->string);
            // (on a tout de meme essayer, mettre le flag...)
            tex->flags |= tex_flag_fullyDrawn;
        }
        return; 
    }
    printdebug("Load png %s of size %d x %d. %p.", png_path, width, height, pixels);
    // Texture OpenGL
    glGenTextures(1, texture_id_ref);
    glBindTexture(GL_TEXTURE_2D, *texture_id_ref);
    GLint filter = (tex->flags & tex_flag_nearest) ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    free(pixels);
    // Mettre à jour les info de la texture.
    tex->ptu.width = (float)width;
    tex->ptu.height = (float)height;
    tex->ratio = tex->ptu.width / tex->ptu.height * tex->ptu.n / tex->ptu.m;
    tex->flags |= asMini ? tex_flag_miniDrawn : tex_flag_fullyDrawn;
}

/// Init partiel de la texture à sa création.
void  _texture_initAsPng(Texture* const tex, const PngInfo* const info, bool isCoqlib) {
    // Init des champs
    tex->ptu = (PerTextureUniforms) { 8, 8, (float)info->m, (float)info->n };
    tex->m =     info->m; tex->n =     info->n;
    tex->alpha = 1.f;     tex->beta =  1.f;
    tex->flags = tex_flag_png|tex_flag_shared|(isCoqlib ? tex_flag_png_coqlib : 0)
                    |(info->nearest ? tex_flag_nearest : 0);
    tex->touchTime = _CR_elapsedMS - _TEX_UNUSED_DELTATMS;
    tex->ratio = (float)info->n / (float)info->m;  // (temporaire)
    tex->string = String_createCopy(info->name);
    // Setter tout de suite la mini si présente.
    _texture_setGlTextureAsPng(tex, true);

    _Texture_total_count ++;
}
void  _texture_initAsString(Texture* const tex, texflag_t flags,
          const char* c_str, const char* fontNameOpt) {
    // 0. Init des champs
    tex->ptu = ptu_default;
    tex->m =     1;   tex->n =     1;
    tex->alpha = 1.f; tex->beta =  1.f;
    tex->flags = flags|tex_flag_string;
    tex->ratio = 1.f;
    tex->string = String_createCopy(c_str);
    tex->fontNameOpt = fontNameOpt ? String_createCopy(fontNameOpt) : NULL;

    _Texture_total_count ++;
}

/// Pour s'assurer qu'il y a au moins une mini quand on demande la texture.
void  _texture_drawPartly(Texture* const tex) {
    if(tex->flags & tex_flag_miniOrFullyDrawn) // Deja dessine.
        return;
    if(tex->flags & tex_flag_string) {
        bool asMini = _Font_currentSize > 40;
        _texture_setGlTextureAsString(tex, asMini);
        return;
    }
    // Png: si pas de mini (créé dans _texture_initAsPng),
    // c'est qu'il faut créer la texture standard.
    _texture_setGlTextureAsPng(tex, false);
}
/// Pour dessiner la vrai texture quand on a le temps...
void  _texture_drawFully(Texture* const tex) {
    if(tex->flags & tex_flag_fullyDrawn)
        return;
    if(tex->flags & tex_flag_string) {
        _texture_setGlTextureAsString(tex, false);
        return;
    }
    _texture_setGlTextureAsPng(tex, false);
}
/// Libere la texture standard (laisse la mini). C'est juste pour liberer la memoire.
void  _texture_unset(Texture* const tex) {
    tex->flags &= ~tex_flag_fullyDrawn;
    if(!tex->texture_id) return;
    glDeleteTextures(1, &tex->texture_id);
    tex->texture_id = 0;
}
bool  _texture_isUsed(Texture* const tex) {
    return _CR_elapsedMS - tex->touchTime <= _TEX_UNUSED_DELTATMS;
}
bool  _texture_isUnused(Texture* const tex) {
    return _CR_elapsedMS - tex->touchTime > _TEX_UNUSED_DELTATMS + 30;
}
void  _texture_deinit(void* tex_void) {
    Texture* tex = tex_void;
    coq_free(tex->string);
    tex->string = NULL;
    // Déréferencer...
    if(tex->listRefererOpt)
        *(tex->listRefererOpt) = (Texture*)NULL;
    if(tex->fontNameOpt) {
        coq_free(tex->fontNameOpt);
        tex->fontNameOpt = NULL;
    }
    if(tex->texture_id) {
        glDeleteTextures(1, &tex->texture_id);
        tex->texture_id = 0;
    }
    if(tex->texture_mini_id) {
        glDeleteTextures(1, &tex->texture_mini_id);
        tex->texture_mini_id = 0;
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

/*-- Font ----------------------------------------------------------------*/
void     Texture_setCurrentFont(const char* fontName) {
    if(fontName == NULL) {
        printerror("No fontName.");
        return;
    }
    
    printwarning("TODO: setCurrentFont...");

    // Font* newFont = [Font fontWithName:[NSString stringWithUTF8String:fontName]
    //                              size:_Font_currentSize];
    // if(newFont == nil) {
    //     printerror("Font %s not found.", fontName);
    //     return;
    // }
    // _Font_current = newFont;
    // _Font_currentMini = [_Font_current fontWithSize:12];
    // [_Font_currentAttributes setObject:_Font_current forKey:NSFontAttributeName];
    // [_Font_currentMiniAttributes setObject:_Font_currentMini forKey:NSFontAttributeName];
    // _Font_current_spreading = Font_getFontInfoOf(fontName)->spreading;
    // Redessiner les strings...
    map_applyToAll(_textureOfConstantString, _map_block_texture_unset);
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) _texture_unset(*tr);
        tr ++;
    }
}
void     Texture_setCurrentFontSize(double newSize) {
    _Font_currentSize = newSize;

    printwarning("TODO: setCurrentFontSize...");
    // if(_Font_current == nil) {
    //     return;
    // }
    // _Font_current = [_Font_current fontWithSize:newSize];
    // [_Font_currentAttributes setObject:_Font_current forKey:NSFontAttributeName];
    // Redessiner les strings...
    map_applyToAll(_textureOfConstantString, _map_block_texture_unset);
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) _texture_unset(*tr);
        tr ++;
    }
}
double   Texture_currentFontSize(void) {
    return _Font_currentSize;
}

/// Liste des pngs par defaut
const PngInfo _coqlib_pngInfos[] = {
    {"coqlib_digits_black", 12, 2, false},
    {"coqlib_scroll_bar_back", 1, 1, false},
    {"coqlib_scroll_bar_front", 1, 1, false},
    {"coqlib_sliding_menu_back", 1, 1, false},
    {"coqlib_sparkle_stars", 3, 2, false},
    {"coqlib_switch_back", 1, 1, false},
    {"coqlib_switch_front", 1, 1, false},
    {"coqlib_test_frame", 1, 1, true},
    {"coqlib_the_cat", 1, 1, true},
    {"coqlib_transparent", 1, 1, true},
    {"coqlib_white", 1, 1, true},
};
const uint32_t _coqlib_png_count = sizeof(_coqlib_pngInfos) / sizeof(PngInfo);

static Texture**         _textureOfPngId = NULL; // Liens vers la map.
static uint32_t          _pngCount = 0;

// OpenGL attribute locations id
static GLint _Texture_tex_wh_id = 0;
static GLint _Texture_tex_mn_id = 0;

/*-- Init, constructors... ----------------------------------------------------------*/
void     Texture_init(GLuint program) {
    if(_Texture_isInit) {
        printerror("Texture already init.");
        return;
    }

    _Texture_tex_wh_id = glGetUniformLocation(program, "tex_wh");
    _Texture_tex_mn_id = glGetUniformLocation(program, "tex_mn");

    printdebug("location tex %d, %d.", _Texture_tex_wh_id, _Texture_tex_mn_id);

    // 1. Texture loader et font.
    // _Font_current = [Font systemFontOfSize:_Font_currentSize];
    // _Font_currentMini = [Font systemFontOfSize:12];
    // _Font_currentAttributes = _FontAttributes_createWithFont(_Font_current);
    // _Font_currentMiniAttributes = _FontAttributes_createWithFont(_Font_currentMini);
    
    _textureOfConstantString = Map_create(64, sizeof(Texture));
    _textureOfPngName =        Map_create(64, sizeof(Texture));
    memset(_nonCstStrTexArray, 0, sizeof(Texture*) * _nonCstStrTexCount);
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
void     Texture_loadPngs(PngInfo const pngInfos[], const uint32_t pngCount) {
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
void     _Texture_suspend(void) {
    if(!_Texture_isInit || !_Texture_isLoaded) return;
    map_applyToAll(_textureOfPngName,        _map_block_texture_unset);
    map_applyToAll(_textureOfConstantString, _map_block_texture_unset);
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) _map_block_texture_unset((char*)*tr);
        tr ++;
    }
    _Texture_isLoaded = false;
}
void     _Texture_resume(void) {
    _Texture_isLoaded = true;
//#warning Superflu finalement ? Texture utilisees seront chargees si besoin dans la premiere frame.
}

static  uint32_t _Texture_checkunset_counter = 0;
void     _Texture_checkToFullyDrawAndUnused(ChronoChecker* cc, int64_t timesUp) {
    if(!_Texture_needToFullyDraw) {
        if(_Texture_checkunset_counter == 0)
            map_applyToAll(_textureOfPngName, _map_block_texture_unsetUnused);
        _Texture_checkunset_counter = (_Texture_checkunset_counter + 1) % 30;
        return;
    }
    if(map_iterator_init(_textureOfPngName)) do {
        Texture* tex = (Texture*)map_iterator_valueRefOpt(_textureOfPngName);
        if(!_texture_isUsed(tex)) continue;
        if(!(tex->flags & tex_flag_fullyDrawn)) {
            _texture_drawFully(tex);
            if(_chronochceker_elapsedMS(cc) > timesUp) {
                return;
            }
        }
    } while(map_iterator_next(_textureOfPngName));
    if(map_iterator_init(_textureOfConstantString)) do {
        Texture* tex = (Texture*)map_iterator_valueRefOpt(_textureOfConstantString);
        if(!_texture_isUsed(tex)) continue;
        if(!(tex->flags & tex_flag_fullyDrawn)) {
            _texture_drawFully(tex);
            if(_chronochceker_elapsedMS(cc) > timesUp) {
                return;
            }
        }
    } while(map_iterator_next(_textureOfConstantString));
    Texture** tr = _nonCstStrTexArray;
    while(tr < _nonCstStrArrEnd) {
        if(*tr) if(!((*tr)->flags & tex_flag_fullyDrawn)) {
            _texture_drawFully(*tr);
            if(_chronochceker_elapsedMS(cc) > timesUp) {
                return;
            }
        }
        tr ++;
    }
    _Texture_needToFullyDraw = false;
}
void     _Texture_deinit(void) {
    _Texture_isInit = false;
    _Texture_isLoaded = false;
    // _MTLtextureLoader = nil;
    map_destroyAndNull(&_textureOfConstantString, _texture_deinit);
    map_destroyAndNull(&_textureOfPngName,        _texture_deinit);
    if(_textureOfPngId)
        coq_free(_textureOfPngId);
    _pngCount = 0;
}

Texture* Texture_sharedImage(uint32_t const pngId) {
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
    if(!_textureOfPngName) {
        printerror("Texture not init.");
        return &_Texture_dummy;
    }
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
    _texture_initAsString(tex, tex_flag_shared, c_str, NULL);
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
    _texture_initAsString(tex, flags,
                          str.c_str, str.fontNameOpt);
    _texture_addToNonConstantStrReferers(tex);
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
    tex->string = String_createCopy(newString);
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

void   texture_glBind(Texture* tex) {
    // Il y a des texture pas encore "fully drawn" ?
    if(!(tex->flags & tex_flag_fullyDrawn))
      _Texture_needToFullyDraw = true;
    glBindTexture(GL_TEXTURE_2D, tex->texture_id ? tex->texture_id : tex->texture_mini_id);
    glUniform2f(_Texture_tex_wh_id, tex->ptu.width, tex->ptu.height);
    glUniform2f(_Texture_tex_mn_id, tex->ptu.m,     tex->ptu.n);
}

/*-- Getters --*/

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
