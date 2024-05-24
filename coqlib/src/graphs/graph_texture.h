//
//  graph_texture.h
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef COQ_GRAPH_TEXTURE_H
#define COQ_GRAPH_TEXTURE_H

#include "graph_base.h"
#include "../maths/math_chrono.h"
#include "../utils/util_string.h"

#define TEXTURE_PNG_NAME_SIZE 32

typedef struct Texture {
//    PerTextureUniforms ptu;
    /// Tiling en x, y. Meme chose que ptu -> m, n.
    uint32_t         m, n;
    uint32_t         _width, _height;
    /// Ratio sans les marges en x, y, e.g. (0.95, 0.95).
    /// Pour avoir les dimension utiles de la texture.
    float            alpha, beta;
    /// Ration w / h d'une tile. -> ratio = (w / h) * (n / m).
    float            ratio;
    uint32_t         flags;
    int64_t          touchTime;
    
    char*            string;  // Copie de la String ou nom du png, pour s'il faut redessiner la texture.
    char*            string_fontOpt;
    Texture**        string_refererOpt; // Sa référence dans une liste, i.e. pour les strings quelconques
    Vector4          string_color; // Couleur pour dessiner la string (noir par defaut)
    uint32_t         string_ref_count; // Juste pour les strings. Les png restent en mémoire. (sont libéré si non utilisé)
    // Metal/OpenGl
    union {
        struct {
            const void*    mtlTex_cptr;
            const void*    mtlTexTmp_cptr;
        };
        struct {
            uint32_t glTexId;
            uint32_t glTexTmpId;
        };
    };
} Texture;

// Infos à passer pour initialiser les images png.
typedef struct {
    char     name[TEXTURE_PNG_NAME_SIZE];
    uint32_t m, n;
    bool     nearest;
    bool     isCoqlib;
    // float alpha, beta;  // "Spreading", Superflu ?
} PngInfo;

#pragma mark - Texture Global

void            Texture_init(PngInfo const pngInfos[], const unsigned pngCount, bool loadCoqlibPngs);
/// Mise en pause (libère les pixels des textures)
void            Texture_suspend(void);
/// Sortie de pause... ne fait pas grand chose...
/// (ne redessine pas les textures, elles sont redessinées avec `Texture_checkToFullyDrawAndUnused`)
void            Texture_resume(void);
/// Nettoyage en fin de programme.
void            Texture_deinit(void);
/// A faire de temps en temps pour dessiner pleinement les textures.
void            Texture_checkToFullyDrawAndUnused(ChronoChecker* cc, int64_t timesUp);

#pragma mark - Font des strings

void            Texture_setCurrentFont(const char* fontName);
void            Texture_setCurrentFontSize(double newSize);
double          Texture_currentFontSize(void);

#pragma mark - Constructeurs de textures --------

/*-- Obtenir la texture de png ou string... --*/
Texture*        Texture_sharedImage(uint32_t const pngId);
Texture*        Texture_sharedImageByName(const char* pngName);
Texture*        Texture_retainString(StringDrawable str);
/// Texture avec array de pixels en mode brgra, i.e. en hexadecimal : 0xAARRGGBB.
/// (Les bit/bytes plus significatifs viennent après les bits moins significatifs dans un array.)
Texture*        Texture_createWithPixels(const void* pixelsBGRA8, uint32_t width, uint32_t height,
                                         bool shared, bool nearest);
/// Libérer (si besoin) la texture qui n'est plus utilisé.
void            textureref_releaseAndNull_(Texture** const texRef);

#pragma mark - Update --------------------

/// Change la texture constante/shared pour une autre.
void            textureref_exchangeSharedStringFor(Texture** const texRef, StringDrawable str);
/// Mise à jour d'une texture de type "string mutable", i.e. non constant et non localisée.
/// Si `forceRedraw`, redessine tout de suite en pleine résolution la string (peut lagger si grosse string).
void            texture_updateMutableString(Texture* tex, const char* newString, bool forceRedraw);

#pragma mark - Engine dependant methods (Implementer avec Metal ou OpenGL) --------------

void           texture_engine_tryToLoadAsPng_(Texture* tex, bool isMini);
void           texture_engine_loadWithPixels_(Texture* tex, const void* pixelsBGRA8, uint32_t width, uint32_t height);
void           texture_engine_updatePixels(Texture* tex, const void* pixelsBGRA8);
void           texture_engine_tryToLoadAsString_(Texture* tex, bool isMini);
void           texture_engine_justSetSizeAsString_(Texture* tex);
void           texture_engine_releaseAll_(Texture* tex);
void           texture_engine_releasePartly_(Texture* tex);
void           texture_engine_setStringAsToRedraw_(Texture* tex);


#pragma mark - Flags d une texture -------------------------------------

enum {
    tex_flag_string_localized = string_flag_localized,
    tex_flag_shared           = string_flag_shared,   // Partagé (pas deallocated quand released)
    tex_flag_string_mutable   = string_flag_mutable,
    tex_flag_nearest          = string_flag_nearest,  // Style pixélisé (pas de smoothing linéaire des pixels)
    tex_flag_string           = 0x0010,
    tex_flag_png              = 0x0020, // Image de png (devrait être shared aussi...)
    tex_flag_png_coqlib       = 0x0040, // (png inclus par défaut)
    
    tex_flags_string_         = 0x000F,
    tex_flag_fullyDrawn_      = 0x0100,
    tex_flag_tmpDrawn_        = 0x0200,
    tex_flags_drawn_          = 0x0300,
    tex_flag_static_          = 0x0400, // Ne peut pas être dealloc.
    
};

// Texture par défaut (du blanc...), un peu comme la mesh par défaut (mesh_sprite, un carré)
extern Texture* texture_white;

// Il existe des textures pas encore dessinées...
extern bool     Texture_needToFullyDraw_;

#endif /* graph_texture_h */
