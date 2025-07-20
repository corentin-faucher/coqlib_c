//  graph_base.h
//
//  Structures de base pour l'affichage :
//  - Instance uniforms,
//  - Pixel,
//  - Png info,
//  - liste de couleurs.
//
//  Created by Corentin Faucher on 2023-10-12.
//
#ifndef COQ_GRAPH_BASE_H
#define COQ_GRAPH_BASE_H
#include "../maths/math_base.h"

/// MARK: - Per instance uniforms
/// Les informations graphiques d'un objet particulier à passer au vertex shader.
/// Doit correspondre avec la struct Metal/OpenGL dans les shaders.
/// (Struct doit être aligned avec size multiple de 16octets (4 floats) : 7 * 16 = 112)
/// (N'est utile que pour les drawables, les données sont libres pour les autres type de noeuds. -> voir `node_base.h`.)
typedef __attribute__((aligned(16))) struct InstanceUniforms {
    Matrix4   model;  // Matrice modèle. Commun à tout les noeuds.
    uint32_t  flags;  // Flags pour renderer et shaders -> voir renderflag_...
    float     show;   // Flag analogique d'affichage du drawable : 0 invisible, 1 visible.
    float     extra1; // Floats extra customizables, e.g. emphase, flip, frequency...
    float     extra2;
    Vector4   color;  // Teinte de couleurs à appliquer (blanc par défaut).
    Rectangle uvRect; // Range de coord. uv à utiliser avec la texture présente.
} InstanceUniforms;
// Flags d'instance uniforms utilsés par renderer et shaders,
// e.g. renderflag_isLight -> l'objet agit comme une source lumière.
enum {
    renderflag_toDraw =           1,
    renderflag_firstCustom =   1<<1,
};

// Exemple d'instance uniforms par défaut :
// Matrice identité, couleur blanc et region uv [0, 1] x [0, 1].
#define InstanceUniforms_default (InstanceUniforms) { \
    .model = {{ 1.f, 0.f, 0.f, 0.f, \
                0.f, 1.f, 0.f, 0.f, \
                0.f, 0.f, 1.f, 0.f, \
                0.f, 0.f, 0.f, 1.f }}, \
    .color = {{ 1.f, 1.f, 1.f, 1.f }}, \
    .uvRect = {{ 0.f, 0.f, 1.f, 1.f }}, \
};

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
/// Transforme la couleur vers le gris de niveau `level`.
/// alpha : ratio de `grisification`.
/// e.g. si level = 0.5, alpha = 0 -> pas de changement.
/// level = 0.5, alpha = 1 -> completement gris (0.5, 0.5, 0.5).
static inline Vector4   vector4_color_toGray(Vector4 const v, float const level, float const alpha);

/// MARK: - Array 2D de pixels (pour mettre dans une texture)
typedef struct PixelBGRAArray {
    size_t const width, height; // Dimensions du "bitmap"
    double deltaX, deltaY; // Décalage du centre de l'image.
    double solidWidth, solidHeight; // Espace "occupé" (typiquement plus petit que width/height)
    PixelBGRA pixels[1];
} PixelBGRAArray;

PixelBGRAArray* PixelBGRAArray_createEmpty(size_t width, size_t height);
PixelBGRAArray* PixelBGRAArray_createSubRegion(PixelBGRAArray const* src, RectangleUint region);
PixelBGRAArray* PixelBGRAArray_createFromBitmapFile(const char* path, bool flipY);
PixelBGRAArray* PixelBGRAArray_engine_createFromPngFileOpt(const char* pngPath, bool showError);

void pixelbgraarray_copyAt(PixelBGRAArray const* src, PixelBGRAArray* dst, UintPair dstOrigin);
void pixelbgraarray_copyRegionAt(PixelBGRAArray const* src, RectangleUint srcRegion,
                                 PixelBGRAArray* dst, UintPair dstOrigin);

// MARK: - Dessin de png
#define TEXTURE_PNG_NAME_SIZE 32
// Infos à passer pour initialiser les images png.
typedef struct PngInfo {
    char     name[TEXTURE_PNG_NAME_SIZE];
    uint32_t m, n;    // Subdivisions (nombre de tiles) en x/y.
    bool     nearest; // Pixélisé ou interp. linéaire.
    
    /// "privé", image par défaut de coqlib, voir liste `coqlib_pngInfos_` dans `graph_texture.c`.
    bool     _isCoqlib; 
} PngInfo;

/// Obtenir le chemin d'un png (le nom du fichier est sans l'extension ".png").
/// Les pngs devraient être dans le dossier resources à `pngs` et `pngs/minis`.
/// Inclure aussi si besoin les pngs de coqlib dans `pngs_coqlib` et `pngs_coqlib/minis`.
char*  FileManager_getPngPathOpt(const char* pngName, bool isCoqlib, bool isMini);


// MARK: - Effet spécial pour la second pass du fragment shader. -----------
// e.g. Ondulation...
// TODO: Enlever... cas particulier à faire pour un projet spécifique.
typedef struct AfterEffect {
    Vector2  pos;
    uint32_t flags;
    float    time;
    float    extra0;  // Paramètres de l'effet, e.g. amplitude, vitesse, delta, phase, ...
    float    extra1;
    float    extra2;
    float    extra3;
} AfterEffect;


// MARK: - Liste de couleurs (en pixels et vector4)
// Quelques couleurs pratiques pour testing.
// Pour la struct Pixel, les couleurs sont dans l'ordre BGRA.
//  └-> *Attention : alpha -> plus significatifs, blue -> moins significatifs.*
#define pixelbgra_black  (PixelBGRA) { 0xFF000000 }
#define pixelbgra_white  (PixelBGRA) { 0xFFFFFFFF }
#define pixelbgra_red    (PixelBGRA) { 0xFFFF0000 }
#define pixelbgra_green  (PixelBGRA) { 0xFF00FF00 }
#define pixelbgra_blue   (PixelBGRA) { 0xFF0000FF }
#define pixelbgra_yellow (PixelBGRA) { 0xFFFFFF00 }

// Sous forme 4 floats, on utilise l'ordre RGBA...
#define color4_black (Vector4) {{0, 0, 0, 1 }}
#define color4_black_back (Vector4) {{0.1, 0.1, 0.05, 1 }}
#define color4_white (Vector4) {{1, 1, 1, 1 }}
#define color4_white_beige (Vector4) {{0.95, 0.92, 0.85, 1 }}
#define color4_gray_25 (Vector4) {{0.25, 0.25, 0.25, 1 }}
#define color4_gray_40 (Vector4) {{0.40, 0.40, 0.40, 1 }}
#define color4_gray_50 (Vector4) {{0.50, 0.50, 0.50, 1 }}
#define color4_gray_60 (Vector4) {{0.60, 0.60, 0.60, 1 }}
#define color4_gray_80 (Vector4) {{0.80, 0.80, 0.80, 1 }}
#define color4_red (Vector4) {{1, 0, 0, 1 }}
#define color4_red_vermilion (Vector4) {{ 1, 0.3, 0.1, 1 }}
#define color4_red_coquelicot (Vector4) {{ 1, 0.2, 0, 1 }}
#define color4_red_orange2 (Vector4) {{ 1, 0.4, 0.4, 1 }}
#define color4_red_coral (Vector4) {{ 1, 0.5, 0.3, 1 }}
#define color4_red_dark (Vector4) {{ 0.2, 0.1, 0.1, 1 }}
#define color4_orange (Vector4) {{ 1, 0.6, 0, 1 }}
#define color4_orange_amber (Vector4) {{ 1, 0.5, 0, 1 }}
#define color4_orange_bronze (Vector4) {{ 0.8, 0.5, 0.2, 1 }}
#define color4_orange_saffron (Vector4) {{ 1.0, 0.6, 0.2, 1 }}
#define color4_orange_saffron2 (Vector4) {{ 1.0, 0.7, 0.4, 1 }}
#define color4_yellow_cadmium (Vector4) {{ 1, 1, 0, 1 }}
#define color4_yellow_amber (Vector4) {{ 1, 0.75, 0, 1 }}
#define color4_yellow_citrine (Vector4) {{ 0.90, 0.82, 0.04, 1 }}
#define color4_yellow_lemon (Vector4) {{ 1, 0.95, 0.05, 1 }}
#define color4_green_electric (Vector4) {{ 0, 1, 0, 1 }}
#define color4_green_electric2 (Vector4) {{ 0.25, 1, 0.25, 1 }}
#define color4_green_fluo (Vector4) {{ 0.5, 1, 0.5, 1 }}
#define color4_green_ao (Vector4) {{ 0.0, 0.55, 0.0, 1 }}
#define color4_green_spring (Vector4) {{ 0.2, 1, 0.5, 1 }}
#define color4_green_avocado (Vector4) {{ 0.34, 0.51, 0.01, 1 }}
#define color4_green_dark_cyan (Vector4) {{ 0.0, 0.55, 0.55, 1 }}
#define color4_aqua (Vector4) {{ 0, 1, 1, 1 }}
#define color4_blue (Vector4) {{ 0, 0.25, 1, 1 }}
#define color4_blue_sky (Vector4) {{ 0.40, 0.70, 1, 1 }}
#define color4_blue_sky2 (Vector4) {{ 0.55, 0.77, 1, 1 }}
#define color4_blue_pale (Vector4) {{ 0.8, 0.9, 1, 1 }}
#define color4_blue_azure (Vector4) {{ 0.00, 0.50, 1, 1 }}
#define color4_blue_strong (Vector4) {{ 0, 0, 1, 1 }}
#define color4_purple (Vector4) {{ 0.8, 0, 0.8, 1 }}
#define color4_purble_china_pink (Vector4) {{ 0.87, 0.44, 0.63, 1 }}
#define color4_purble_electric_indigo (Vector4) {{ 0.44, 0.00, 1, 1 }}
#define color4_purble_blue_violet (Vector4) {{ 0.54, 0.17, 0.89, 1 }}

// Liste des couleurs
/*
enum {
    colorenum_black,
    colorenum_black_back,
    colorenum_white,
    colorenum_white_beige,
    colorenum_gray_25,
    colorenum_gray_40,
    colorenum_gray_50,
    colorenum_gray_60,
    colorenum_gray_80,
    colorenum_red,
    colorenum_red_vermilion,
    colorenum_red_coquelicot,
    colorenum_red_orange2,
    colorenum_red_coral,
    colorenum_red_dark,
    colorenum_orange,
    colorenum_orange_amber,
    colorenum_orange_bronze,
    colorenum_orange_saffron,
    colorenum_orange_saffron2,
    colorenum_yellow_cadmium,
    colorenum_yellow_amber,
    colorenum_yellow_citrine,
    colorenum_yellow_lemon,
    colorenum_green_electric,
    colorenum_green_electric2,
    colorenum_green_fluo,
    colorenum_green_ao,
    colorenum_green_spring,
    colorenum_green_avocado,
    colorenum_green_dark_cyan,
    colorenum_aqua,
    colorenum_blue,
    colorenum_blue_sky,
    colorenum_blue_sky2,
    colorenum_blue_pale,
    colorenum_blue_azure,
    colorenum_blue_strong,
    colorenum_purple,
    colorenum_purble_china_pink,
    colorenum_purble_electric_indigo,
    colorenum_purble_blue_violet,
};

extern const Vector4 color4_ofEnum[];
*/

#include "graph_base.inl"

#endif
