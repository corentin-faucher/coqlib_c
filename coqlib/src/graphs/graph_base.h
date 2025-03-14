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

typedef struct coq_Font CoqFont;

/// MARK: - Per instance uniforms
/// Les informations graphiques d'un objet particulier à passer au vertex shader.
/// Doit correspondre avec la struct Metal/OpenGL dans les shaders.
/// (Struct doit être aligned avec size multiple de 16octets (4 floats) : 7 * 16 = 112)
/// (N'est utile que pour les drawables, les données sont libres pour les autres type de noeuds. -> voir `node_base.h`.)
typedef __attribute__((aligned(16))) struct InstanceUniforms {
    Matrix4   model;  // Commun à tout les noeuds
    uint32_t  flags;  // Flags pour renderer et shaders -> voir renderflag_...
    float     show;   // Flag analogique d'affichage du drawable.
    float     extra1; // Floats extra customizables, e.g. emphase, flip, frequency...
    float     extra2;
    Vector4   color;
    Rectangle uvRect; // Coord uv de la texture du drawable.
} InstanceUniforms;

// Piu par defaut : Matrice identite, blanc, tile (0,0)x(1,1), show == 1.
extern const InstanceUniforms InstanceUniforms_default;
extern const Rectangle        InstanceUniforms_defaultUVRect;

// MARK: - Structure des pixels 24 ou 32 bits.
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

/// Convertion d'un float en gris (et alpha = f)
static inline PixelBGRA float_toPixelBGRA(float const f);
/// Conversion de Vector4 vers un pixel en BGRA uint8,
/// e.g. (1, 0.25, 0, 0.5) -> 0x7F003FFF
static inline PixelBGRA vector4_color_toPixelBGRA(Vector4 const v);
// Quelques couleurs pratiques pour testing...
// Pour les autres couleurs voir `graph_color.h` et `PixelBGRA vector4_color_toPixelBGRA(Vector4 v)`
extern const PixelBGRA pixelbgra_black;
extern const PixelBGRA pixelbgra_white;
extern const PixelBGRA pixelbgra_red;
extern const PixelBGRA pixelbgra_yellow;

/// MARK: - Array 2D de pixels (pour mettre dans une texture)

typedef struct PixelBGRAArray {
    size_t const width, height; // Dimensions du "bitmap"
    double deltaX, deltaY; // Décalage du centre de l'image.
    double solidWidth, solidHeight; // Espace "occupé" (typiquement plus petit que width/height)
    PixelBGRA pixels[1];
} PixelBGRAArray;

PixelBGRAArray* Pixels_engine_createFromPngOpt(const char* pngPathOpt, bool showError);
PixelBGRAArray* Pixels_engine_createArrayFromCharacter(Character c, CoqFont const* coqFont);
PixelBGRAArray* Pixels_engine_test_createArrayFromString_(const char* c_str, CoqFont const* coqFont);

// MARK: - Couleurs (en Vector4) et conversion de couleurs
/// Transforme la couleur vers le gris de niveau `level`.
/// alpha : ratio de `grisification`.
/// e.g. si level = 0.5, alpha = 0 -> pas de changement.
/// level = 0.5, alpha = 1 -> completement gris (0.5, 0.5, 0.5).
static inline Vector4   vector4_color_toGray(Vector4 const v, float const level, float const alpha);

extern const Vector4 color4_black;
extern const Vector4 color4_black_back;
extern const Vector4 color4_white;
extern const Vector4 color4_white_beige;
extern const Vector4 color4_gray_25;
extern const Vector4 color4_gray_40;
extern const Vector4 color4_gray_60;
extern const Vector4 color4_gray_80;
extern const Vector4 color4_red;
extern const Vector4 color4_red_vermilion;
extern const Vector4 color4_red_coquelicot;
extern const Vector4 color4_red_orange2;
extern const Vector4 color4_red_coral;
extern const Vector4 color4_red_dark;
extern const Vector4 color4_orange;
extern const Vector4 color4_orange_amber;
extern const Vector4 color4_orange_bronze;
extern const Vector4 color4_orange_saffron;
extern const Vector4 color4_orange_saffron2;
extern const Vector4 color4_yellow_cadmium;
extern const Vector4 color4_yellow_amber;
extern const Vector4 color4_yellow_citrine;
extern const Vector4 color4_yellow_lemon;
extern const Vector4 color4_green_electric;
extern const Vector4 color4_green_electric2;
extern const Vector4 color4_green_fluo;
extern const Vector4 color4_green_ao;
extern const Vector4 color4_green_spring;
extern const Vector4 color4_green_avocado;
extern const Vector4 color4_green_dark_cyan;
extern const Vector4 color4_aqua;
extern const Vector4 color4_blue;
extern const Vector4 color4_blue_sky;
extern const Vector4 color4_blue_sky2;
extern const Vector4 color4_blue_pale;
extern const Vector4 color4_blue_azure;
extern const Vector4 color4_blue_strong;
extern const Vector4 color4_purple;
extern const Vector4 color4_purble_china_pink;
extern const Vector4 color4_purble_electric_indigo;
extern const Vector4 color4_purble_blue_violet;

// MARK: - Dessin de png
#define TEXTURE_PNG_NAME_SIZE 32
// Infos à passer pour initialiser les images png.
typedef struct {
    char     name[TEXTURE_PNG_NAME_SIZE];
    uint32_t m, n;
    bool     nearest;
    bool     isCoqlib;
} PngInfo;

/// Obtenir le chemin d'un png (le nom du fichier est sans l'extension ".png").
/// Les pngs devraient être dans le dossier resources à `pngs` et `pngs/minis`.
/// Inclure aussi si besoin les pngs de coqlib dans `pngs_coqlib` et `pngs_coqlib/minis`.
char*  FileManager_getPngPathOpt(const char* pngName, bool isCoqlib, bool isMini);

// MARK: - Fonts et glyphs pour les strings
typedef struct coq_Font CoqFont;
typedef struct CoqFontInit {
    char const*const nameOpt;     // Pour UIFont/NSFont
    char const*const fileNameOpt; // Pour Freetype
    double const     sizeOpt;
    PixelBGR         color;
    bool const       nearest;     // "Pixélisé"
} CoqFontInit;
// Infos sur les dimensions de la police.
typedef struct CoqFontDims {
    float const deltaY;
    float const fullHeight;
    float const solidHeight;
    float const relFullHeihgt; // Ratio `fullHeight / solidHeight`, pour scaling en y.
    float const relDeltaY; // Ratio `deltaY / solidHeight`.
    bool  const nearest;
} CoqFontDims;

CoqFont* CoqFont_engine_create(CoqFontInit info);
void     coqfont_engine_destroy(CoqFont** cf);
/// Décalage pour centrer les glyphes, relatif à `solidHeight`, i.e. `(ascender - capHeight)/ (2*solidHeight)`.
CoqFontDims coqfont_dims(CoqFont const* cf);

#define FONT_extra_margin(nearest) (nearest ? 1 : 2)
// Définie où sont les fonts et la font par défaut (avec extension)
void CoqFont_freetype_init(const char* fontsPath, const char* defaultFontFileName);
void CoqFont_freetype_quit(void);
void CoqFont_test_printAvailableFonts(void);

// MARK: - Effet spécial pour la second pass du fragment shader. -----------
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

#include "graph_base.inl"

#endif
