//
//  graph.h
//  Liste des structures "uniforms",
//  i.e. les structures passées aux shaders.
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef COQ_GRAPH_BASE_H
#define COQ_GRAPH_BASE_H

#include "../maths/math_base.h"
#include "../utils/util_char_and_keycode.h"

#define TEXTURE_PNG_NAME_SIZE 32
// Infos à passer pour initialiser les images png.
typedef struct {
    char     name[TEXTURE_PNG_NAME_SIZE];
    uint32_t m, n;
    bool     nearest;
    bool     isCoqlib;
} PngInfo;

/// Obtenir le chemin d'un png (nom sans extension .png)
char*  FileManager_getPngPathOpt(const char* pngName, bool isCoqlib, bool isMini);

//typedef struct Texture Texture;
#pragma mark -- Per instance uniforms --------------------------------
/// Les informations graphiques d'un objet particulier
/// à passer au vertex shader.
/// (Struct doit être aligned avec size multiple de 16octets (4 floats) : 7 * 16 = 112)
/// (N'est utile que pour les drawables, les données sont libres pour les autres type de noeuds. -> voir `node_base.h`.)
typedef __attribute__((aligned(16))) struct InstanceUniforms {
    Matrix4   model;  // Commun à tout les noeuds
    uint32_t  render_flags;  // Flags pour renderer et shaders.
    float     show;   // Flag analogique d'affichage du drawable.
    float     extra1; // Floats extra customizables, e.g. emphase, flip, frequency...
    float     extra2;
    union {
        // Réserver aux drawables (8 floats / 32 bytes).
        struct {
            Vector4   draw_color;
            Rectangle draw_uvRect; // Coord uv de la texture du drawable.
        };
        // Data quelconques pour noeuds "non-drawables", e.g. les infos des boutons...
        union {
            struct {
                uint32_t nodraw_uint0, nodraw_uint1, nodraw_uint2, nodraw_uint3,
                         nodraw_uint4, nodraw_uint5, nodraw_uint6, _nodraw_uint7;
            };
            struct {
                float    nodraw_float0, nodraw_float1, nodraw_float2, nodraw_float3,
                         nodraw_float4, nodraw_float5, nodraw_float6, _nodraw_float7;
            };
            struct { struct coq_Node *nodraw_node0, *nodraw_node1; };
        };
    };
} InstanceUniforms;
// Piu par defaut : Matrice identite, blanc, tile (0,0)x(1,1), show == 1.
extern const InstanceUniforms InstanceUnifoms_drawableDefaultIU;
// Buffer vers les uniforms d'instance. Implémentation dépend de l'engine graphique.
typedef struct IUsBuffer {
    uint32_t const          max_count;
    uint32_t                actual_count;
    InstanceUniforms* const ius;    // Array des infos d'instances.
    const void* const       _mtlBuffer_cptr;  // Buffer Metal
} IUsBuffer;

/// Création du buffer.
void   iusbuffer_engine_init_(IUsBuffer* iusbuffer, uint32_t count);
/// Libère l'espace du buffer (et array de piu si nécessaire)
void   iusbuffer_engine_deinit_(IUsBuffer* iusbuffer);
void   iusbuffer_setAllTo(IUsBuffer* iusBuffer, InstanceUniforms iu);
void   iusbuffer_setAllActiveTo(IUsBuffer* iusBuffer, InstanceUniforms iu);

//#define for_iu_in_buffer(iu, iuBuffer) \
//    { InstanceUniforms* const end = &((iuBuffer).ius[nb->dm.iusBuffer.actual_count]);  \
//    for(InstanceUniforms* iu = (iuBuffer).ius; iu < end; iu++) {
//#define for_iu_in_buffer_end }}


#pragma mark - Structure des pixels 24 ou 32 bits.
// Blue -> bits moins signif., Red -> bits plus signif.
// i.e. 0xAARRGGBB. -> uint8 b, g, r, a;
typedef struct PixelBGR {
    uint8_t b, g, r;
} PixelBGR;
typedef union PixelBGRA {
    uint32_t data;
    struct {
        union {
            PixelBGR bgr;
            struct { uint8_t b, g, r; };
        };
        uint8_t a;
    };
} PixelBGRA;
/// Les pixels d'une texture.
typedef struct PixelBGRAArray {
    size_t width, height;
    double deltaX;
    double solidWidth;
    PixelBGRA pixels[1];
} PixelBGRAArray;

// Quelques couleurs pratiques pour testing...
// Pour les autres couleurs voir `graph_color.h` et `PixelBGRA vector4_color_toPixelBGRA(Vector4 v)`
extern const PixelBGRA pixelbgra_black;
extern const PixelBGRA pixelbgra_white;
extern const PixelBGRA pixelbgra_red;
extern const PixelBGRA pixelbgra_yellow;

#pragma mark - Dessin de png
PixelBGRAArray* Pixels_engine_createOptFromPng(const char* pngPathOpt);

#pragma mark - Dessin de fonts/glyphs -
typedef struct coq_Font {
    union {
        // Version Apple -> Dictionnaire d'attributes (dont la NSFont/UIFont)
        const void* const _nsdict_attributes;
        // Version SDL -> TTF_Font
        void const*const  _ttfFont;
    };
    bool const   nearest; // Pixelisé ou smooth ?
    /// Hauteur total requise pour dessiner les glyphes en pixels avec les fioritures qui depassent,
    ///  i.e. `ascender - descender`.
    size_t const glyphHeight;
    /// Hauteur de la hitbox en pixels sans le fioriture, i.e. `capHeight - descender`.
    /// (Les autres dimensions sont normalisé par rapport à `solidHeight`,
    ///  on a `solidHeight < glyphHeight`.)
    float const  solidHeight;
    float const  deltaY;  // Décalage pour centre les glyphes, i.e. (ascender - capHeight)/2.
    float const  drawDeltaY; // Décalage pour dessiner...
} coq_Font;
// Définie où sont les fonts (juste pour SDL_TTF).
void  CoqFont_sdlttf_setFontsPath(const char* fontsPath, const char* defaultFontName);
void  CoqFont_sdlttf_quit(void);
void  coqfont_engine_init(coq_Font const* cf, const char *fontNameOpt, double fontSizeOpt, bool nearest);
void  coqfont_engine_deinit(coq_Font const* cf);
/// Décalage pour centre les glyphes, i.e. (ascender - capHeight)/2.
/// `relY = deltaY / solidHeight`
float coqfont_getRelY(coq_Font const* cf);
/// `relHeight = glyphHeight / solidHeight > 1`
float coqfont_getRelHeight(coq_Font const* cf);

PixelBGRAArray* Pixels_engine_createArrayFromCharacter(Character c, coq_Font const coqFont);
PixelBGRAArray* Pixels_engine_createArrayFromString(const char* c_str, coq_Font const coqFont);

#pragma mark - Effet spécial pour la second pass du fragment shader. -----------
// e.g. Ondulation...
typedef struct AfterEffect {
    Vector2  pos;
    uint32_t flags;
    float    time;
    float    extra0;  // Paramètres de l'effet, e.g. amplitude, vitesse, delta, phase, ...
    float    extra1;
    float    extra2;
    float    extra3;
} AfterEffect;

#endif


// Garbage
/*-- Per texture uniforms --------------------------------*/
/*-- Les informations pour le shader d'une texture.     --*/
//typedef __attribute__((aligned(16)))struct {
//    float width, height; // largeur, hauteur en pixels.
//    float m, n;          // Tiling en x, y.
//} PerTextureUniforms;
//#define PTU_DEFAULT \
//{ 8, 8, 1, 1 }
//extern const PerTextureUniforms ptu_default;

/*-- Per frame uniforms --------------------------------*/
/*-- Les shader constantes  pour la frame courante    --*/
//typedef __attribute__((aligned(16)))struct {
//    Matrix4 projection;
//    float time;
//    float unused1, unused2, unused3;
//} PerFrameUniforms;
//extern PerFrameUniforms pfu_default;

/// Espace (tile) occupé par une sprite dans la grille d'une texture.
/// e.g. si ij0 = {1, 2} et Dij = {2, 2} pour un png avec un grille m = 10, n = 5,
/// on a uv0 = { 0.1, 0.4 }, Duv = { 0.2, 0.4 }
//typedef struct InstanceTile {
//    union {
//        uint32_t ij[2];
//        struct { uint32_t i, j; };
//    };
//    union {
//        uint32_t Dij[2];
//        struct { uint32_t Di, Dj; };
//    };
//} InstanceTile;
//Rectangle instancetile_toUVRectangle(InstanceTile const it, uint32_t m, uint32_t n);
