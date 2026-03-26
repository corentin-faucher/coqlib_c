//
//  graph_texture_apple.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//
#include "graph_texture.h"
#include "graph_texture_private.h"
#include "../utils/util_map.h"
#include "../utils/util_base.h"
#include "../utils/util_chars.h"
#include "../systems/system_file.h"

#define TEX_UNUSED_DELTATMS_ 1000

/*-- Variable globales --*/
static int32_t           Texture_total_count_ = 0;
void     texture_initEmpty_(Texture* const tex, size_t const width, size_t const height, uint32_t flags) {
    tex->dims = (TextureDims) {
        .width = width, .height = height,
        .m = 1,         .n = 1,
        .Du = 1,        .Dv = 1,
    };
    tex->touchTime = RendererTimeCapture.render_elapsedMS;
    if(flags & (~tex_flags__initFlags)) {
        printwarning("Create texture with non valid flags %#06x.", 
                     flags & (~tex_flags__initFlags));
        flags &= tex_flags__initFlags;
    }
    if((flags & tex_flag_doubleBuffer) && !(flags & tex_flag_mutable)) {
        printwarning("Non mutable texture doesn't need double buffer.");
        flags &= ~tex_flag_doubleBuffer;
    }
    tex->flags = flags;
    tex->pixelsOpt = PixelArray_createEmpty(width, height);
    Texture_total_count_ ++;
}
Texture* Texture_createWithPixels(void const*const pixelsBGRA8Opt, uint32_t const width, uint32_t const height, uint32_t const flags) 
{
    Texture* tex = coq_callocTyped(Texture);
    texture_initEmpty_(tex, width, height, flags);
    if(tex->pixelsOpt && pixelsBGRA8Opt) {
        memcpy(tex->pixelsOpt->pixels, pixelsBGRA8Opt, width*height*sizeof(PixelRGBA));
    }
    
    return tex;
}
void  textureref_init(Texture*const*const texref, Texture*const initValue) {
    if(*texref || !initValue) { printerror("Already init or no initValue."); return; }
    *(Texture**)texref = initValue;
}
void  textureref_render_releaseAndNull(Texture*const*const texRef) {
    if(!texRef) { printerror("No tex ref."); return; }
    Texture*const toRelease = *texRef;
    *(Texture**)texRef = (Texture*)NULL;
    if(!toRelease) return; // (ok pas grave, déjà release)
    if(toRelease->flags & tex_flag_shared) return;
    texture_render_deinit_(toRelease);
    coq_free(toRelease);
}

void  texture_render_deinit_(void* tex_void) {
    Texture*const tex = tex_void;
    if(tex->flags & tex_flag__editing) {
        printerror("dealloc while still editing.");
    }
    texture_render_releaseBuffers_(tex);
    if(tex->string) {
        coq_free(tex->string);
        tex->string = NULL;
    }
    if(tex->pixelsOpt) {
        coq_free(tex->pixelsOpt);
        tex->pixelsOpt = NULL;
    }
    if(tex->miniPixelsOpt) {
        coq_free(tex->miniPixelsOpt);
        tex->miniPixelsOpt = NULL;
    }
    Texture_total_count_ --;
}

TextureDims texture_dims(Texture const*const tex) {
    return tex->dims;
}
PixelArray const* texture_getPixelsOpt(Texture const*const tex) {
    if(!tex->pixelsOpt || !(tex->flags & (tex_flag_keepPixels|tex_flag_mutable))) {
        return NULL;
    }
    return tex->pixelsOpt;
}
bool              texture_isShared(Texture const*const tex) {
    return tex->flags & tex_flag_shared;
}

// MARK: - Edition
/// Obtenir la référence aux pixels pour édition (NULL si non mutable)
TextureToEdit texture_retainToEditOpt(Texture*const tex) {
    if(!(tex->flags & tex_flag_mutable) || !tex->pixelsOpt) {
        printerror("Trying to edit non mutable texture or missing pixels.");
        return (TextureToEdit) {};
    }
    if(tex->flags & tex_flag__editing) {
        printerror("Texture already in edit mode. Forget releaseVertices?");
        return (TextureToEdit) {};
    }
    tex->flags |= tex_flag__editing;
    return (TextureToEdit) {
        .pa = tex->pixelsOpt,
        ._tex = tex,
    };
}
/// Fini d'éditer, flager pour que la thread de rendering met à jour les pixels.
void texturetoedit_release(TextureToEdit const texEdit) {
    Texture*const tex = texEdit._tex;
    if(!(tex->flags & (tex_flag_mutable|tex_flag__editing))) {
        printerror("Bad vertices release."); return;
    }
    tex->flags &= ~tex_flag__editing;
    tex->version_pixels ++;
}

// MARK: - Listes de textures
/// Maps des png (shared).
static StringMap*        textureOfPngName_ = NULL; // Pngs dans une map.
static Texture**         textureOfPngIdOpt_ = NULL;// Pngs dans un array.
static unsigned          pngCount_ = 0;            // Nombre de pngs.
Texture* Texture_white = NULL;
static uint32_t texture_white_pixels_[4] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};
Texture* Texture_frameBuffers[10] = {};

// MARK: - Globals
static bool              _Texture_isInit = false;
static bool              _Texture_isLoaded = false;
void     Texture_init_(void) {
    if(_Texture_isInit) {
        printerror("Texture already init.");
        return;
    }
    // Map des textures du projet.
    textureOfPngName_ =  Map_create(64, sizeof(Texture));
    // Texture par défaut (blanc)
    Texture_white = Texture_createWithPixels(texture_white_pixels_, 2, 2, tex_flag_shared|tex_flag_nearest);    
    _Texture_isInit = true;
    _Texture_isLoaded = true;
}
void     Texture_render_deinit(void) {
    _Texture_isInit = false;
    _Texture_isLoaded = false;
    map_destroyAndNull(&textureOfPngName_, texture_render_deinit_);
    if(textureOfPngIdOpt_)
        coq_free(textureOfPngIdOpt_);
    textureOfPngIdOpt_ = NULL;
    texture_render_deinit_(Texture_white);
    coq_free(Texture_white);
    Texture_white = NULL;
    pngCount_ = 0;
}

void     Texture_resume(void) {
    _Texture_isLoaded = true;
//#warning Superflu finalement ? Texture utilisees seront chargees si besoin dans la premiere frame.
}
void     Texture_suspend(void) {
    if(!_Texture_isInit || !_Texture_isLoaded) return;
    map_applyToAll(textureOfPngName_, texture_render_releaseBuffers_); // (On garde les minis pour les pngs)
    _Texture_isLoaded = false;
}

// MARK: - Gestion des pngs
const PngInfo            coqlib_pngInfos_[] = {
    { .name = "coqlib_bar_in",          .m = 1, .n = 1, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_digits_black",    .m = 12, .n = 2, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_frame_gray_back", .m = 1, .n = 1, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_frame_mocha",     .m = 1, .n = 1, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_frame_red",       .m = 1, .n = 1, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_frame_white_back", .m = 1, .n = 1, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_scroll_bar_back", .m = 1, .n = 1, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_scroll_bar_front", .m = 1, .n = 1, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_sliding_menu_back", .m = 1, .n = 1, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_sparkle_stars",   .m = 3, .n = 2, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_switch_back",     .m = 1, .n = 1, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_switch_front",    .m = 1, .n = 1, .flags = tex_flag_png_coqlib },
    { .name = "coqlib_test_frame",      .m = 1, .n = 1, .flags = tex_flag_nearest|tex_flag_png_coqlib },
    { .name = "coqlib_the_cat",         .m = 1, .n = 1, .flags = tex_flag_nearest|tex_flag_png_coqlib },
};
static const PngInfo*    pngInfosOpt_ = NULL;      // Liste des infos des pngs du projet. 
const uint32_t           coqlib_pngCount_ = sizeof(coqlib_pngInfos_) / sizeof(PngInfo);

void   texture_initAsPng_(Texture* const tex, const PngInfo* const info) {
    tex->dims = (TextureDims) {
        .width = 2,               .height = 2,
        .m = info->m,             .n = info->n,
        .Du = 1.f/(float)info->m, .Dv = 1.f/(float)info->n,
    };
    uint32_t flags = info->flags;
    if(flags & (~tex_flags__initFlags)) {
        printwarning("Create texture with non valid flags %#06x.", 
                     flags & (~tex_flags__initFlags));
        flags &= tex_flags__initFlags;
    }
    tex->flags = flags | tex_flag__png | tex_flag_shared;
    tex->touchTime = RendererTimeCapture.render_elapsedMS - TEX_UNUSED_DELTATMS_ - 30;
    tex->string = String_createCopy(info->name);
    
    Texture_total_count_ ++;
}
void   Texture_loadCoqlibPngs(void) {
    if(!_Texture_isInit) { printwarning("Texture not init ?"); Texture_init_(); }
    const PngInfo* const end = &coqlib_pngInfos_[coqlib_pngCount_];
    for(const PngInfo *info = coqlib_pngInfos_; info < end; info++) { 
        Texture* png_tex = (Texture*)map_put(textureOfPngName_, info->name, NULL);
        if(png_tex->dims.m) { printwarning("png already loaded."); continue; }
        texture_initAsPng_(png_tex, info);
    }
}
void   Texture_loadProjectPngs(PngInfo const*const pngInfosOpt, const unsigned pngCount) {
    if(textureOfPngIdOpt_) { printwarning("User textures already loaded."); return; }
    if(!_Texture_isInit) { printwarning("Texture not init ?"); Texture_init_(); }
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
        } else {
            uint32_t flags = info->flags;
            if(flags & (~tex_flags__initFlags)) {
                printwarning("Create texture with non valid flags %#06x.", 
                    flags & (~tex_flags__initFlags));
                flags &= tex_flags__initFlags;
            }
            png_tex->flags |= flags;
        }
        
        // Ajout a l'array de ref.
        textureOfPngIdOpt_[index] = png_tex;
        index ++;
        info ++;
    }
}
char*    FileManager_getPngPathOpt_(const char* pngName, bool const isCoqlib, bool const isMini) 
{
    if(!pngName) { printerror("Texture without name."); return NULL; }
    const char* png_dir;
    if(isCoqlib) {
        png_dir = isMini ? "pngs_coqlib/minis" : "pngs_coqlib";
    } else {
        png_dir = isMini ? "pngs/minis" : "pngs";
    }
    char* path = FileManager_getResourcePath();
    String_pathAdd(path, pngName, "png", png_dir);
    return path;
}
// Superflu ?
void     texturedims_update_(TextureDims*const dims, size_t const width, size_t const height) {
    dims->width =  (float)width;
    dims->height = (float)height;
}
void     texture_loadPngBarePixels_(void*const tex_void) {
    Texture*const tex = (Texture*)tex_void;
    if(!(tex->flags & tex_flag__png)) { printerror("Not a png."); return; }
    if(RendererTimeCapture.render_elapsedMS - tex->touchTime > TEX_UNUSED_DELTATMS_) return;
    if(tex->miniPixelsOpt || tex->pixelsOpt) return;
    if(tex->rendFlags & tex_flags_rend_buffersSet) return;
    
    // Chargement des pixels. A priori juste la mini.
    const char* pngPath = FileManager_getPngPathOpt_(tex->string, tex->flags & tex_flag_png_coqlib, true);
    tex->miniPixelsOpt = PixelArray_createFromPngFileOpt(pngPath, false);
    PixelArray* pixels = tex->miniPixelsOpt;
    // Si pas de mini, on prend l'image normale.
    if(!tex->miniPixelsOpt) {
        pngPath = FileManager_getPngPathOpt_(tex->string, tex->flags & tex_flag_png_coqlib, false);
        tex->pixelsOpt = PixelArray_createFromPngFileOpt(pngPath, true);
        pixels = tex->pixelsOpt;
    } else {
    }
    // Mise à jour des dimensions
    if(pixels) {
        texturedims_update_(&tex->dims, pixels->width, pixels->height);
    }
}
void     texture_loadPngFullPixels_(void*const tex_void) {
    Texture*const tex = (Texture*)tex_void;
    if(!(tex->flags & tex_flag__png)) { printerror("Not a png."); return; }
    if(RendererTimeCapture.render_elapsedMS - tex->touchTime > TEX_UNUSED_DELTATMS_) return;
    if(tex->pixelsOpt || (tex->rendFlags & tex_flag_rend_bufferSet)) return;
    // Chargement des pixels.
    const char* pngPath = FileManager_getPngPathOpt_(tex->string, tex->flags & tex_flag_png_coqlib, false);
    tex->pixelsOpt = PixelArray_createFromPngFileOpt(pngPath, true);
    // Mise à jour des dimensions
    if(tex->pixelsOpt) {
        texturedims_update_(&tex->dims, tex->pixelsOpt->width, tex->pixelsOpt->height);
    } else {
        printwarning("Cannot load full pixels for %s ?", tex->string);
    }
}
Texture* Texture_getPng(uint32_t const pngId) {
    if(!textureOfPngIdOpt_) { printerror("pngs not loaded."); return Texture_white; }
    if(pngId >= pngCount_) {
        printerror("Invalid png id %d. Max id is %d.", pngId, pngCount_);
        return Texture_white;
    }
    Texture *tex = textureOfPngIdOpt_[pngId];
    tex->touchTime = RendererTimeCapture.render_elapsedMS;
    ChronoChecker cc = chronochecker_startNew();
    texture_loadPngBarePixels_((char*)tex);
    if(chronochecker_elapsedMS(cc) > TEXTURE_MAX_LOADTIME_MS) {
        if(tex->pixelsOpt) {
            printwarning("Texture %s (full). Take %lld ms to load. Add a mini ?", 
                tex->string, chronochecker_elapsedMS(cc));
        } else {
            printdebug("Texture %s (mini). Take %lld ms to load. Mini too big ?", 
                tex->string, chronochecker_elapsedMS(cc));
        }
    }
    return tex;
}
Texture* Texture_getPngByName(const char* pngName) {
    if(!textureOfPngName_) { printerror("Texture not init."); return Texture_white; }
    Texture* tex = (Texture*)map_valueRefOptOfKey(textureOfPngName_, pngName);
    if(!tex) {
        printerror("Invalid png name %s.", pngName);
        return Texture_white;
    }
    tex->touchTime = RendererTimeCapture.render_elapsedMS;
    ChronoChecker cc = chronochecker_startNew();
    texture_loadPngBarePixels_((char*)tex);
    if(chronochecker_elapsedMS(cc) > TEXTURE_MAX_LOADTIME_MS) {
        printwarning("Texture %s (%s). Take %lld ms to load. Add a mini ?", 
            tex->string, tex->pixelsOpt ? "full" : "mini", chronochecker_elapsedMS(cc));
    }
    return tex;
}

static bool volatile Texture_needToBareDraw_ = false; // Png sans même la mini.
static bool volatile Texture_needToFullyDraw_ = false;
static bool volatile Texture_isDrawingPng_ = false;
bool   Texture_needToDrawPngs(void) {
    return (Texture_needToBareDraw_ || Texture_needToFullyDraw_) && !Texture_isDrawingPng_;
}
void   Texture_setNeedToBareDrawPng_(void) {
    Texture_needToBareDraw_ = true;
}
void   Texture_setNeedToFullyDrawPng_(void) {
    Texture_needToFullyDraw_ = true;
}
void   Texture_drawMissingPngs(void) {
    if(Texture_isDrawingPng_) {
        printwarning("Already drawing. Check with Texture_isReadyToDrawPngs.");
        return;
    }
    Texture_isDrawingPng_ = true;
    if(Texture_needToBareDraw_) {
        map_applyToAll(textureOfPngName_, texture_loadPngBarePixels_);
        Texture_needToBareDraw_ = false;
    }
    if(Texture_needToFullyDraw_) {
        map_applyToAll(textureOfPngName_, texture_loadPngFullPixels_);
        Texture_needToFullyDraw_ = false;
    }
    Texture_isDrawingPng_ = false;
}

void   texture_render_unsetUnusedPng_(void*const tex_char) {
    Texture* tex = (Texture*)tex_char;
    if(!(tex->rendFlags & tex_flags_rend_buffersSet)) return;
    // Utilisée recemment ?
    if(RendererTimeCapture.render_elapsedMS - tex->touchTime < TEX_UNUSED_DELTATMS_ + 30) return;
//    printdebug("Found unused texture %s to release.", tex->string);
    texture_render_releaseBuffers_(tex);
}
void   Texture_render_releaseUnusedPngs(void) {
    map_applyToAll(textureOfPngName_, texture_render_unsetUnusedPng_);
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
