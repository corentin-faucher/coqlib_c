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

// Temps maximale tolérable pour le chargement d'un png.
// Si trop long, warning d'ajouter une "mini".
#define TEXTURE_MAX_LOADTIME_MS 15

// MARK: - TextureDims, Dimensions d'une texture.
// Info pratique sur les dimensions d'une texture.
typedef struct TextureDims {
    // Dimensions en pixels et ratio d'une tile.
    size_t    width, height;
    // Découpage/subdivisions en x/y.
    union {
        UintPair mn;
        struct { uint32_t  m, n; };
    };
    union {
        Vector2 DuDv; // Du = 1/m.
        struct { float Du, Dv; };
    };
    
//    float     tileRatio;  // (w n) / (h m)
    // (ratio ordinaire est juste width/height, superflu...)
} TextureDims;

static inline void          texturedims_initConst(const TextureDims* texDims, TextureDims initValue);
static inline float         texturedims_tileRatio(TextureDims texDims);
// Obtenir la région en coord uv d'une "tile" de la texture.
static inline Rectangle     texturedims_uvRectOfTile(TextureDims texDims, uint32_t i, uint32_t j);
/// Convertie les coord. UV (dans [0, 1]) en coord. pixels (dans [0, width/height]).
static inline RectangleUint texturedims_pixelRegionFromUVrect(TextureDims texDims, Rectangle UVrect);
/// Convertie les coord. pixels (dans [0, width/height]) en coord. UV (dans [0, 1])
static inline Rectangle     texturedims_UVrectFromPixelRegion(TextureDims texDims, RectangleUint region);


// MARK: - Texture
typedef struct Texture Texture;
void     texture_initEmpty_(Texture* tex, size_t width, size_t height, uint32_t flags);
/// Texture avec array de pixels en mode brgra, i.e. en hexadecimal : 0xAARRGGBB.
/// (Les bit/bytes plus significatifs viennent après les bits moins significatifs dans un array.)
Texture* Texture_createWithPixels(const void* pixelsBGRA8Opt, 
                        uint32_t width, uint32_t height, uint32_t flags);
void     textureref_init(Texture*const* texref, Texture* initValue);
void     textureref_render_releaseAndNull(Texture*const* tex);
                        
/// Obtenir les dimensions de la texture.
TextureDims texture_dims(Texture const* tex);
/// Obtenir les pixels. Il faut une texture mutable ou avec `tex_flag_keepPixels`.
PixelArray const* texture_getPixelsOpt(Texture const* tex);
bool              texture_isShared(Texture const* tex);

enum {
// Flags utilisable pour init `Texture_createWithPixels`.
    tex_flag_shared           = 0x0001,  // Partagé (pas deallocated quand released)
                                        // Les "png" sont automatiquement `shared`.
    tex_flag_mutable          = 0x0002,  // Peut être édité.
    tex_flag_nearest          = 0x0004,  // Style pixélisé (pas de smoothing linéaire des pixels)
    tex_flag_keepPixels       = 0x0008,  // Garde les pixels en mémoire même si n'est pas mutable.
                     // Par défaut, on libère les pixels une fois la texture (Metal/OpenGL) loadé.
    tex_flag_png_coqlib       = 0x0010, // pngs de coqlib...
    tex_flag_doubleBuffer     = 0x0020,  // Mutable avec deux buffers en alternance pour l'affichage.
    
    tex_flags__initFlags      = 0x00FF,
};

// MARK: - Edition
typedef struct TextureToEdit {
    PixelArray*const pa;
    Texture*const    _tex;
} TextureToEdit;
//bool       texture_isReadyToEdit(Texture const* tex);
/// Obtenir la référence aux pixels pour édition (NULL si non mutable)
TextureToEdit texture_retainToEditOpt(Texture* tex);
/// Fini d'éditer, flager pour que la thread de rendering met à jour les pixels.
void       texturetoedit_release(TextureToEdit texEdit);
#define withTextureToEdit_beg(texEdit, tex) \
            { TextureToEdit texEdit = texture_retainToEditOpt(tex);  \
            if(texEdit.pa) {
#define withTextureToEdit_end(texEdit) texturetoedit_release(texEdit); } \
            else { printerror("Cannot edit texture."); } }


// MARK: - Dessin (renderer)
typedef struct TextureToDraw {
    union {
        const void* mtlTexture;
        uint32_t    glTextureId;
    };
    bool isNearest; // (Style pixélisé)
} TextureToDraw;

void          texture_render_checkTexture(Texture*tex);
TextureToDraw texture_render_getToDraw(Texture* tex);

//typedef struct TextureRef {
//    /// Référence à la texture.
//    Texture *const    tex;
//    /// Les infos pratique à propos de la texture.
//    TextureDims const dims;
//} TextureRef;
//void  textureref_init(TextureRef* texref, Texture *tex);
//void  textureref_render_releaseAndNull(TextureRef* texref);


// MARK: - Texture Global
// Texture par défaut (du blanc...), un peu comme la mesh par défaut (mesh_sprite, un carré)
extern Texture* Texture_white;
// Textures du frame buffer (résultats de la première passe du renderer)
extern Texture* Texture_frameBuffers[10];

/// (privé -> voir `CoqGraph_init...`)
void            Texture_init_(void);
/// Mise en pause (libère les pixels des textures)
void            Texture_suspend(void);
/// Sortie de pause... ne fait pas grand chose...
/// (ne redessine pas les textures, elles sont redessinées avec `Texture_checkToFullyDrawAndUnused`)
void            Texture_resume(void);
/// Nettoyage en fin de programme.
void            Texture_render_deinit(void);

// MARK: - Gestion des images png.
/// Charger les pngs de coqlib (dans `res/pngs_coqlib`)
void            Texture_loadCoqlibPngs(void);
/// Charger les pngs du projet (dans `res/pngs`).
void            Texture_loadProjectPngs(PngInfo const*const pngInfosOpt, const unsigned pngCount);
Texture*        Texture_getPng(uint32_t const pngId);
Texture*        Texture_getPngByName(const char* pngName);

/// A faire si `Texture_needToDrawPngs`.
/// Devrait être lancé dans une thread séparée 
/// pour ne pas bloquer le rendering ou les events.
void   Texture_drawMissingPngs(void);
bool   Texture_needToDrawPngs(void);
void   Texture_setNeedToBareDrawPng_(void);
void   Texture_setNeedToFullyDrawPng_(void);

/// Peut être fait de temps en temps pour libérer les textures non utilisées.
/// Lancer dans la thread de rendering (call fonctions OpenGL/Metal)
void   Texture_render_releaseUnusedPngs(void);

// Définitions des inlines
#include "graph_texture.inl"

#endif /* graph_texture_h */
