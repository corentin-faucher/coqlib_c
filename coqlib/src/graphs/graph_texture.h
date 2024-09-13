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
#include "../utils/util_char_and_keycode.h"

#define TEXTURE_PNG_NAME_SIZE 32

typedef struct Texture {
    uint32_t         m, n;  // Dimensions en "tuiles".
    union {
        Vector2        sizes; // Dimensions en pixels.
        struct { float width, height; };
    };
    uint32_t         flags;
    int64_t          touchTime;
    
    char*            string;  // Copie de la String ou nom du png, pour s'il faut redessiner la texture.
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
} PngInfo;

#pragma mark - Texture Global

// Texture par défaut (du blanc...), un peu comme la mesh par défaut (mesh_sprite, un carré)
extern Texture* Texture_white;
// Textures du frame buffer (résultats de la première passe du renderer)
extern Texture* Texture_frameBuffers[10];

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


#pragma mark - Texture de png --------
Texture*        Texture_sharedImage(uint32_t const pngId);
Texture*        Texture_sharedImageByName(const char* pngName);
/// Ration w/h d'une tile de la texture.
float           texture_tileRatio(const Texture* tex);
/// Espace u/v d'une tile de la texture (Delta u, Delta v).
Vector2         texture_tileDuDv(const Texture* tex);

#pragma mark - Texture de pixels ------
/// Texture avec array de pixels en mode brgra, i.e. en hexadecimal : 0xAARRGGBB.
/// (Les bit/bytes plus significatifs viennent après les bits moins significatifs dans un array.)
Texture*        Texture_createWithPixels(const void* pixelsBGRA8, uint32_t width, uint32_t height,
                                         bool shared, bool nearest);
/// Convertie les coord. UV (dans [0, 1]) en coord. pixels (dans [0, width/height]).
RectangleUint  texture_pixelRegionFromUVrect(const Texture* tex, Rectangle UVrect);
/// Convertie les coord. pixels (dans [0, width/height]) en coord. UV (dans [0, 1])
Rectangle      texture_UVrectFromPixelRegion(const Texture* tex, RectangleUint region);

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

#pragma mark - Libérer une texture.
/// Libérer (free) si n'est pas shared.
void            textureref_releaseAndNull_(Texture** const texRef);


// Privé...
#pragma mark - Privé, engine dependant methods (Implementer avec Metal ou OpenGL) --------------
void           texture_engine_tryToLoadAsPng_(Texture* tex, bool isMini);
// Editions des pixels d'une texture...
/// Crée une texture OpenGL/Metal vide. 
void           texture_engine_loadEmptyWithSize_(Texture* tex, size_t width, size_t height);

                                  
void           texture_engine_releaseAll_(Texture* tex);
void           texture_engine_releasePartly_(Texture* tex);

#pragma mark - Flags d'une texture -------------------------------------
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

// Il existe des textures pas encore dessinées...
extern bool     Texture_needToFullyDraw_;

void  texture_initEmpty_(Texture* tex);
void  texture_deinit_(void* tex_void);

#endif /* graph_texture_h */
