//
//  graph_texture.h
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef COQ_GRAPH_TEXTURE_H
#define COQ_GRAPH_TEXTURE_H

#include "graph_base.h"
#include "../coq_chrono.h"

#define TEXTURE_CHECK_MINI_NEEDED 0

typedef struct Texture Texture;
// MARK: - Dimensions d'une texture.
// Info pratique sur les dimensions d'une texture.
typedef struct TextureDims {
    // Découpage/subdivisions en x/y.
    union {
        UintPair mn;
        struct { uint32_t  m, n; };
    };
    union {
        Vector2 DuDv; // Du = 1/m.
        struct { float Du, Dv; };
    };
    // Dimensions en pixels et ratio d'une tile.
    union {
        Vector2 sizes;
        struct { float width, height; };
    };
    float tileRatio;  // (w n) / (h m)
    // (ratio ordinaire est juste width/height, superflu...)
} TextureDims;

// Info pour renderer pour setter la texture à dessiner.
typedef struct TextureToDraw {
    union {
        const void* metal_texture_cptr;
        uint32_t    glTextureId;
    };
    bool isNearest; // (Style pixélisé)
} TextureToDraw;

// MARK: - Texture Global
// Texture par défaut (du blanc...), un peu comme la mesh par défaut (mesh_sprite, un carré)
extern Texture* Texture_white;
// Textures du frame buffer (résultats de la première passe du renderer)
extern Texture* Texture_frameBuffers[10];

/// Mise en pause (libère les pixels des textures)
void            Texture_suspend(void);
/// Sortie de pause... ne fait pas grand chose...
/// (ne redessine pas les textures, elles sont redessinées avec `Texture_checkToFullyDrawAndUnused`)
void            Texture_resume(void);
/// Nettoyage en fin de programme.
void            Texture_deinit(void);
/// A faire de temps en temps pour dessiner pleinement les textures.
void            Texture_checkToFullyDrawAndUnused(ChronoChecker* cc, int64_t timesUp);
/// Charger les pngs de coqlib (dans `res/pngs_coqlib`)
void            Texture_loadCoqlibPngs(void);
/// Charger les pngs du projet (dans `res/pngs`).
void            Texture_loadProjectPngs(PngInfo const*const pngInfosOpt, const unsigned pngCount);
/// (privé -> voir `CoqGraph_init...`)
void            Texture_init_(void);

// MARK: - Texture de png --------
Texture*        Texture_sharedImage(uint32_t const pngId);
Texture*        Texture_sharedImageByName(const char* pngName);

// MARK: - Texture de pixels
/// Texture avec array de pixels en mode brgra, i.e. en hexadecimal : 0xAARRGGBB.
/// (Les bit/bytes plus significatifs viennent après les bits moins significatifs dans un array.)
Texture*        Texture_createWithPixels(const void* pixelsBGRA8Opt, uint32_t width, uint32_t height,
                                         bool shared, bool nearest);

typedef struct TextureRef {
    /// Référence à la texture.
    Texture *const    tex;
    /// Les infos pratique à propos de la texture.
    TextureDims const dims;
} TextureRef;
void  textureref2_init(TextureRef* texref, Texture *tex);
void  textureref2_releaseAndNull(TextureRef* texref);

// MARK: - Methode
/// Obtenir les dimensions de la texture.
TextureDims const texture_dims(Texture const* tex);
// Obtenir la région en coord uv d'une "tile" de la texture.
static inline Rectangle texturedims_uvRectOfTile(TextureDims texDims, uint32_t i, uint32_t j);
/// Convertie les coord. UV (dans [0, 1]) en coord. pixels (dans [0, width/height]).
static inline RectangleUint texturedims_pixelRegionFromUVrect(TextureDims texDims, Rectangle UVrect);
/// Convertie les coord. pixels (dans [0, width/height]) en coord. UV (dans [0, 1])
static inline Rectangle     texturedims_UVrectFromPixelRegion(TextureDims texDims, RectangleUint region);
/// Info pour dessiner par le renderer (et empêche de libérer)
TextureToDraw   texture_engine_touchAndGetToDraw(Texture * tex);
// Dépend de l'engine (Metal / OpenGL)...
/// Copier un array de pixels dans la texture.
void           texture_engine_writeAllPixels(Texture* tex, const void* pixelsBGRA8);
/// Copier un array de pixels dans la texture aux coordonées de `region`.
void           texture_engine_writePixelsToRegion(Texture* tex, const void* pixelsBGRA8, RectangleUint region);
/// Copier une partie (region) de la texture vers un array de pixels (l'array de pixels doit être de taille suffisante)
void           texture_engine_writeRegionToPixels(const Texture* tex, void* pixelBGRA8, RectangleUint region);
/// Copier une région d'une texture vers une autre texture.
void           texture_engine_copyRegionTo(const Texture* tex, Texture* texDest,
                                           RectangleUint srcRect, UintPair destOrig);

// MARK: - Flags d'une texture -------------------------------------
enum {
//    tex_flag_string_localized = string_flag_localized,
    tex_flag_shared           = 0x0002,   // Partagé (pas deallocated quand released)
//    tex_flag_string_mutable   = string_flag_mutable,
    tex_flag_nearest          = 0x0008,  // Style pixélisé (pas de smoothing linéaire des pixels)
//    tex_flag_string           = 0x0010,
//    tex_flag_png              = 0x0020, // Image de png (devrait être shared aussi...)
    tex_flag_png_coqlib       = 0x0040, // (png inclus par défaut)
    
//    tex_flags_string_         = 0x000F,
    tex_flag_fullyDrawn_      = 0x0100,
    tex_flag_tmpDrawn_        = 0x0200,
    tex_flags_drawn_          = 0x0300,
    tex_flag_static_          = 0x0400, // Ne peut pas être dealloc.
    
};



// Définitions des inlines
#include "graph_texture.inl"

#endif /* graph_texture_h */
