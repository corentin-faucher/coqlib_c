//
//  graph_texture_apple.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//
#include "graph_texture.h"

#include "../coq_map.h"
#include "../utils/util_base.h"
#include "../utils/util_string.h"

#include "graph_colors.h"


#define TEX_UNUSED_DELTATMS_ 1000

static Texture      Texture_dummy_ = {
    .n = 1, .m = 1,
    .width = 8, .height = 8,
    .flags = tex_flag_static_, 
    .touchTime = 0,
    .string = NULL,
};

/*-- Variable globales --*/
static int32_t           Texture_total_count_ = 0;
bool                     Texture_needToFullyDraw_ = false;
static bool              _Texture_isInit = false;
static bool              _Texture_isLoaded = false;

#pragma mark - Fonctions privées sur une instance texture. -------------

bool  _texture_isUsed(Texture* const tex) {
    return CR_elapsedMS_ - tex->touchTime <= TEX_UNUSED_DELTATMS_;
}
bool  _texture_isUnused(Texture* const tex) {
    return CR_elapsedMS_ - tex->touchTime > TEX_UNUSED_DELTATMS_ + 30;
}
void  texture_deinit_(void* tex_void) {
    Texture* tex = tex_void;
    texture_engine_releaseAll_(tex);
    if(tex->string) {
        coq_free(tex->string);
        tex->string = NULL;
    }
    // Déréferencer...
//    if(tex->string_refererOpt)
//        *(tex->string_refererOpt) = (Texture*)NULL;
    Texture_total_count_ --;
}

// Casting des modif de texture d'engine pour les maps. 
static void (* const texture_engine_releasePartly__)(char*) = (void (*)(char*))texture_engine_releasePartly_;
//static void (* const texture_engine_setStringAsToRedraw__)(char*) = (void (*)(char*))texture_engine_setStringAsToRedraw_;
//static void (* const texture_engine_releaseAll__)(char*) = (void (*)(char*))texture_engine_releaseAll_;
void  _texture_unsetUnusedPng_(char* tex_char) {
    Texture* tex = (Texture*)tex_char;
    if(!(tex->flags & tex_flag_tmpDrawn_)) return;
    if(!_texture_isUnused(tex)) return;
    texture_engine_releasePartly_(tex);
}
void  texture_initAsPng_(Texture* const tex, const PngInfo* const info) {
    // Init des champs
    tex->m =     info->m; tex->n =     info->n;
//    tex->ratio = (float)info->n / (float)info->m; 
    tex->flags = tex_flag_shared|(info->isCoqlib ? tex_flag_png_coqlib : 0)
                    |(info->nearest ? tex_flag_nearest : 0);
    tex->touchTime = CR_elapsedMS_ - TEX_UNUSED_DELTATMS_;
    
    tex->string = String_createCopy(info->name);
    
    texture_engine_tryToLoadAsPng_(tex, true);
    
    Texture_total_count_ ++;
}

void  texture_initEmpty_(Texture* const tex) {
    *tex = (Texture) {
        .m = 1, .n = 1,
        .width = 2, .height = 2,
        .flags = 0, .touchTime = CR_elapsedMS_,
        // reste à 0.
    };
    Texture_total_count_ ++;
}

#pragma mark - Listes de textures -------------------------------------------
/// Maps des png (shared).
static StringMap*        textureOfPngName_ = NULL; // Pngs dans une map.
static Texture**         textureOfPngIdOpt_ = NULL;// Pngs dans un array.
static const PngInfo*    pngInfosOpt_ = NULL;      // Liste des infos des pngs du projet. 
static unsigned          pngCount_ = 0;            // Nombre de pngs.
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
};
const uint32_t           coqlib_pngCount_ = sizeof(coqlib_pngInfos_) / sizeof(PngInfo);

Texture* Texture_white = NULL;
static uint32_t texture_white_pixels_[4] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};
Texture* Texture_frameBuffers[10] = {};

#pragma mark -- Globals ------------------------------------------------

void    Texture_init_(bool loadCoqlibPngs) {
    if(_Texture_isInit) {
        printerror("Texture already init.");
        return;
    }
    // Preload des png de coqlib.
    textureOfPngName_ =  Map_create(64, sizeof(Texture));
    if(loadCoqlibPngs) {
        const PngInfo*       info = coqlib_pngInfos_;
        const PngInfo* const end = &coqlib_pngInfos_[coqlib_pngCount_];
        while(info < end) {
            Texture* png_tex = (Texture*)map_put(textureOfPngName_, info->name, NULL);
            texture_initAsPng_(png_tex, info); // (ici c'est juste vide, i.e. sans MTLTexture)
            info ++;
        }
    }
    // Texture par défaut (blanc)
    Texture_white = Texture_createWithPixels(texture_white_pixels_, 2, 2, true, true);    
    
    _Texture_isInit = true;
    _Texture_isLoaded = true;
}
void     Texture_loadProjectPngs(PngInfo const*const pngInfosOpt, const unsigned pngCount) {
    if(textureOfPngIdOpt_) { printwarning("User textures already loaded."); return; }
    if(!_Texture_isInit) { printwarning("Texture not init."); return; }
    if(!pngCount || !pngInfosOpt) { printerror("Missing pngCount or pngInfos to load pngs."); return; }
    pngInfosOpt_ =       pngInfosOpt;
    textureOfPngIdOpt_ = coq_calloc(pngCount, sizeof(Texture*));
    pngCount_ = pngCount;
    const PngInfo*       info = pngInfosOpt_;
    const PngInfo* const end = &pngInfosOpt_[pngCount_];
    uint32_t index = 0;
    while(info < end) {
        // Vérifier si existe déjà... (pourrait être un coqlib png)
        Texture* png_tex = (Texture*)map_valueRefOptOfKey(textureOfPngName_, info->name);
        if(!png_tex) {
            png_tex = (Texture*)map_put(textureOfPngName_, info->name, NULL);
            texture_initAsPng_(png_tex, info);
        }
        // Ajout a l'array de ref.
        textureOfPngIdOpt_[index] = png_tex;
        index ++;
        info ++;
    }
}

void     Texture_suspend(void) {
    if(!_Texture_isInit || !_Texture_isLoaded) return;
    map_applyToAll(textureOfPngName_, texture_engine_releasePartly__); // (On garde les minis pour les pngs)
    _Texture_isLoaded = false;
}
void     Texture_resume(void) {
    _Texture_isLoaded = true;
//#warning Superflu finalement ? Texture utilisees seront chargees si besoin dans la premiere frame.
}
void     Texture_deinit(void) {
    _Texture_isInit = false;
    _Texture_isLoaded = false;
    map_destroyAndNull(&textureOfPngName_,      texture_deinit_);
    if(textureOfPngIdOpt_)
        coq_free(textureOfPngIdOpt_);
    textureOfPngIdOpt_ = NULL;
    texture_deinit_(Texture_white);
    Texture_white = NULL;
    pngCount_ = 0;
}

void     Texture_checkToFullyDrawAndUnused(ChronoChecker* cc, int64_t timesUp) {
    static unsigned checkunset_counter = 0;
    // Une fois de temps en temps, quand on a le temps,
    if(!Texture_needToFullyDraw_) {
        // liberer les png non utilises.
        checkunset_counter = (checkunset_counter + 1) % 30;
        if(checkunset_counter == 0)
            map_applyToAll(textureOfPngName_, _texture_unsetUnusedPng_);
        if(checkunset_counter != 1) return;

        return;
    }    
    // Chercher les png, `const string`, `non const string` à dessiner au complet...
    if(map_iterator_init(textureOfPngName_)) do {
        Texture* tex = (Texture*)map_iterator_valueRefOpt(textureOfPngName_);
        if(!_texture_isUsed(tex)) continue;
        if(!(tex->flags & tex_flag_fullyDrawn_)) {
            texture_engine_tryToLoadAsPng_(tex, false);
            if(chronochecker_elapsedMS(cc) > timesUp)
                return;
        }
    } while(map_iterator_next(textureOfPngName_));
//    if(map_iterator_init(textureOfSharedString_)) do {
//        Texture* tex = (Texture*)map_iterator_valueRefOpt(textureOfSharedString_);
//        if(!_texture_isUsed(tex)) continue;
//         if(!(tex->flags & tex_flag_fullyDrawn_)) {
//            texture_engine_tryToLoadAsString_(tex, false);
//            if(chronochecker_elapsedMS(cc) > timesUp)
//                return;
//        }
//    } while(map_iterator_next(textureOfSharedString_));
//    Texture** tr = _nonCstStrTexArray;
//    while(tr < _nonCstStrArrEnd) {
//        if(*tr) if(!((*tr)->flags & tex_flag_fullyDrawn_)) {
//            texture_engine_tryToLoadAsString_(*tr, false);
//            if(chronochecker_elapsedMS(cc) > timesUp)
//                return;
//        }
//        tr ++;
//    }
    Texture_needToFullyDraw_ = false;
}

#pragma mark -- Constructors... ----------------------------------------

Texture* Texture_sharedImage(uint32_t const pngId) {
    if(!textureOfPngIdOpt_) { printerror("pngs not loaded."); return Texture_white; }
    if(pngId >= pngCount_) {
        printerror("Invalid png id %d. Max id is %d.", pngId, pngCount_);
        return &Texture_dummy_;
    }
    Texture *tex = textureOfPngIdOpt_[pngId];
    tex->touchTime = CR_elapsedMS_;
    // S'il y a pas la mini, créer tout de suite la vrai texture. (besoin des dimensions)
    // (Normalement, s'il n'y a pas de mini c'est que la texture est petite...)
    if((tex->flags & tex_flags_drawn_) == 0) {
        ChronoChecker cc;
        chronochecker_set(&cc);
        texture_engine_tryToLoadAsPng_(tex, false);
        if(chronochecker_elapsedMS(&cc) > 10)
            printwarning("Png %d, no mini for %s. time %lld.", pngId, tex->string, chronochecker_elapsedMS(&cc));
    }
    return tex;
}
Texture* Texture_sharedImageByName(const char* pngName) {
    if(!textureOfPngName_) { printerror("Texture not init."); return &Texture_dummy_; }
    Texture* tex = (Texture*)map_valueRefOptOfKey(textureOfPngName_, pngName);
    if(!tex) {
        printerror("Invalid png name %s.", pngName);
        return &Texture_dummy_;
    }
    tex->touchTime = CR_elapsedMS_;
    if((tex->flags & tex_flags_drawn_) == 0) {
        texture_engine_tryToLoadAsPng_(tex, false);
    }
    return tex;
}


Texture* Texture_createWithPixels(const void* pixelsBGRA8, uint32_t width, uint32_t height, 
                                  bool shared, bool nearest) 
{
    Texture* tex = coq_callocTyped(Texture);
    texture_initEmpty_(tex);
    tex->flags = (shared ? tex_flag_shared : 0)|(nearest ? tex_flag_nearest : 0);
    texture_engine_loadEmptyWithSize_(tex, width, height);
    texture_engine_writeAllPixels(tex, pixelsBGRA8);
    return tex;
}

void    textureref_releaseAndNull_(Texture** const texRef) {
    if(*texRef == NULL) return;
    Texture* const tex = *texRef;
    *texRef = NULL;
    if(tex->flags & tex_flag_shared) return;
    if(tex->flags & tex_flag_static_) { printwarning("Release de static tex ?"); return; }
    
    texture_deinit_(tex);
    coq_free(tex);
}


#pragma mark - Methods ---

RectangleUint  texture_pixelRegionFromUVrect(const Texture* const tex, Rectangle const UVrect) {
    return (RectangleUint) {{
        tex->width * UVrect.o_x, tex->height * UVrect.o_y,
        tex->width * UVrect.w,   tex->height * UVrect.h,
    }};
}

Rectangle      texture_UVrectFromPixelRegion(const Texture* const tex, RectangleUint const region) {
    return (Rectangle) {{
        (float)region.o_x / tex->width, (float)region.o_y / tex->height,
        (float)region.w / tex->width,   (float)region.h / tex->height,
    }};
}

float          texture_tileRatio(const Texture* const tex) {
    return tex->width / tex->height * ((float)tex->n / (float)tex->m);
}
Vector2        texture_tileDuDv(const Texture* const tex) {
    return (Vector2) {{ 1.f/(float)tex->m, 1.f/(float)tex->n }};
}


// Garbage
//void    textureref_exchangeSharedStringFor(Texture** const texRef, StringDrawable str) {
//    if(((*texRef)->flags & tex_flag_shared) == 0 || (str.string_flags & string_flag_shared) == 0) {
//        printerror("Not shared strings.");
//        return;
//    }
//    Texture* const oldTex = *texRef;
//    *texRef = Texture_retainString(str);
//    if(oldTex->string_ref_count > 0) oldTex->string_ref_count --;
//}
//*-- Fonctions public sur une instance Texture -------------------------*/
//void  texture_updateMutableString(Texture* tex, const char* newString, bool forceRedraw) {
//    if(!(tex->flags & string_flag_mutable) || !newString) {
//        printwarning("Not a mutable string or missing newString.");
//        return;
//    }
//    coq_free(tex->string);
//    tex->string = String_createCopy(newString);
//    texture_engine_tryToLoadAsString_(tex, !forceRedraw);
//    tex->touchTime = CR_elapsedMS_;
//}
//void     _texture_addToNonSharedStringReferers(Texture* tex) {
//    Texture** const referer = _nonCstStrCurrent;
//    if(*referer) {
//        printwarning("Texture referer space occupied.");
//    } else {
//        // Référencer dans la liste des "non constant strings".
//        *referer = tex;
//        tex->string_refererOpt = referer;
//    }
//    // Cherche un disponible pour le prochain...
//    do {
//        _nonCstStrCurrent++;
//        if(_nonCstStrCurrent >= _nonCstStrArrEnd)
//            _nonCstStrCurrent = _nonCstStrTexArray;
//        if(*_nonCstStrCurrent == NULL)
//            return;
//    } while(_nonCstStrCurrent != referer);
//    printwarning("Texture referer space not found.");
//}
// Création d'une string quelconque (mutable, localisable, avec font...)
//Texture* Texture_retainString(StringDrawable str) {
//    if(!_Texture_isInit) {
//        printerror("Texture not init.");
//        return &Texture_dummy_;
//    }
//    // 0. Récuperer si la string existe.
//    Texture* tex;
//    if(str.string_flags & string_flag_shared) {
//        if(str.string_flags & string_flag_mutable) {
//            printerror("Shared string cannot be mutable.");
//            str.string_flags &= ~tex_flag_string_mutable;
//        }
//        tex = (Texture*)map_valueRefOptOfKey(textureOfSharedString_, str.c_str);
//        // Existe, juste incrémenter le compteur de références.
//        if(tex) {
//            tex->string_ref_count ++;
//            return tex;
//        }
//        // N'existe pas encore, créer...
//        tex = (Texture*)map_put(textureOfSharedString_, str.c_str, NULL);
//    } else {
//        tex = coq_callocTyped(Texture);
//    }
//    
//    // 1. Init des champs de base.
////    tex->ptu = ptu_default;
//    tex->m =     1;   tex->n =     1;
//    tex->flags = tex_flag_string|(str.string_flags & tex_flags_string_);
//    tex->string_ref_count = 1;
//    tex->touchTime = CR_elapsedMS_;
//    tex->string = String_createCopy(str.c_str);
//    tex->string_fontOpt = str.fontNameOpt ? String_createCopy(str.fontNameOpt) : NULL;
//    tex->string_color = str.color;
//    // 1. Evaluer les dimensions.
//    texture_engine_justSetSizeAsString_(tex);
//    
//    // 3. Garder une référence pour pause/resume.
//    if(!(str.string_flags & string_flag_shared))
//        _texture_addToNonSharedStringReferers(tex);
//    // (sinon c'est dans textureOfSharedString_)
//    
//    Texture_total_count_ ++;
//    return tex;
//}
/// Après un changement de Font...
//void     Texture_redrawStrings(void) {
//    map_applyToAll(textureOfSharedString_, texture_engine_setStringAsToRedraw__);
//    Texture** tr = _nonCstStrTexArray;
//    while(tr < _nonCstStrArrEnd) {
//        if(*tr) { texture_engine_setStringAsToRedraw_(*tr); }
//        tr ++;
//    }
//}
/// Maps des constant strings (shared)
//static StringMap*        textureOfSharedString_ = NULL;
// Liste des strings quelconques...
//#define NonCstStringTexture_Count_ 64
//static Texture*          _nonCstStrTexArray[NonCstStringTexture_Count_];
//static Texture** const   _nonCstStrArrEnd = &_nonCstStrTexArray[NonCstStringTexture_Count_];
//static Texture**         _nonCstStrCurrent = _nonCstStrTexArray;
