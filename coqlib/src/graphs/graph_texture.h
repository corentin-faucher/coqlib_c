//
//  graph_texture.h
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef COQ_GRAPH_TEXTURE_H
#define COQ_GRAPH_TEXTURE_H

#include "graph_base.h"
#include "../utils/util_chrono.h"

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
    size_t    width, height;
    float tileRatio;  // (w n) / (h m)
    // (ratio ordinaire est juste width/height, superflu...)
} TextureDims;

// Info pour renderer pour setter la texture à dessiner.
typedef struct TextureToDraw {
    union {
        const void* mtlTexture;
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

typedef struct TextureRef {
    /// Référence à la texture.
    Texture *const    tex;
    /// Les infos pratique à propos de la texture.
    TextureDims const dims;
} TextureRef;
void  textureref_init(TextureRef* texref, Texture *tex);
void  textureref_releaseAndNull(TextureRef* texref);

// MARK: - Texture de png --------
Texture*        Texture_sharedImage(uint32_t const pngId);
Texture*        Texture_sharedImageByName(const char* pngName);

// MARK: - Texture de pixels
/// Texture avec array de pixels en mode brgra, i.e. en hexadecimal : 0xAARRGGBB.
/// (Les bit/bytes plus significatifs viennent après les bits moins significatifs dans un array.)
Texture*        Texture_createWithPixels(const void* pixelsBGRA8Opt, 
                        uint32_t width, uint32_t height, uint32_t flags);

// MARK: - Getters
/// Obtenir les dimensions de la texture.
TextureDims const texture_dims(Texture const* tex);
// Obtenir la région en coord uv d'une "tile" de la texture.
static inline Rectangle texturedims_uvRectOfTile(TextureDims texDims, uint32_t i, uint32_t j);
/// Convertie les coord. UV (dans [0, 1]) en coord. pixels (dans [0, width/height]).
static inline RectangleUint texturedims_pixelRegionFromUVrect(TextureDims texDims, Rectangle UVrect);
/// Convertie les coord. pixels (dans [0, width/height]) en coord. UV (dans [0, 1])
static inline Rectangle     texturedims_UVrectFromPixelRegion(TextureDims texDims, RectangleUint region);

// MARK: - Conversion en array de pixels
PixelBGRAArray* PixelBGRAArray_engine_createFromTextureOpt(Texture* tex);
PixelBGRAArray* PixelBGRAArray_engine_createFromTextureSubRegionOpt(Texture* tex, RectangleUint region);

// MARK: - Edition
void texture_engine_writePixelsAt(Texture* tex, PixelBGRAArray const* pixels, UintPair origin);
/// Copier une région d'une texture vers une autre texture.
/// Équivalent de `PixelBGRAArray_engine_createFromTextureSubRegionOpt`, `texture_engine_writePixelsAt`.
void texture_engine_copyRegionTo(const Texture* texSrc, RectangleUint srcRect,
                                 Texture* texDst, UintPair dstOrig);

// MARK: - Dessin (renderer)
void            texture_render_touchAndUpdate(Texture* tex);
/// Info pour dessiner par le renderer (et empêche de libérer)
TextureToDraw   texture_render_getTextureToDraw(Texture* tex);

// MARK: - Flags d'une texture -------------------------------------
enum {
// Flags utilisable pour init `Texture_createWithPixels`.
    tex_flag_shared           = 0x0001,  // Partagé (pas deallocated quand released)
    tex_flag_doubleBuffer     = 0x0004,  // Mutable avec double buffer pour édition "live".
    tex_flag_nearest          = 0x0008,  // Style pixélisé (pas de smoothing linéaire des pixels)
    
// Flags "privés"...
    tex_flags__initFlags      = 0x000f,
    tex_flag__dbEdited        = 0x0010,  // Double buffer édité, swaper pour le rendering.
    tex_flag__dbSecondLive    = 0x0040,
    tex_flag__png_coqlib       = 0x0080, // (png inclus par défaut)
    tex_flag__fullyDrawn      = 0x0100,
    tex_flag__tmpDrawn        = 0x0200,
    tex_flags__drawn          = 0x0300,
    tex_flag__static          = 0x0400, // Ne peut pas être dealloc.
};



// Définitions des inlines
#include "graph_texture.inl"

#endif /* graph_texture_h */
