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
// On utilisera le format RGBA, 
// Red -> bits moins signif., Blue -> bits plus signif.
// en hexa (unsigned de 32 bits) : 0xAABBGGRR,
// équivalent de `uint8 r, g, b, a;`
typedef struct PixelRGB {
    uint8_t r, g, b;
} PixelRGB;
typedef union PixelRGBA {
    uint32_t data;
    struct {
        union {
            PixelRGB rgb;
            struct { uint8_t r, g, b; };
        };
        uint8_t a;
    };
} PixelRGBA;
/// Convertion d'un float en gris (et alpha = f)
static inline PixelRGBA float_toPixelRGBA(float const f);
/// Conversion de Vector4 vers un pixel en BGRA uint8,
/// e.g. (1, 0.25, 0, 0.5) -> 0x7F003FFF
static inline PixelRGBA vector4_color_toPixelRGBA(Vector4 const v);
/// Transforme la couleur vers le gris de niveau `level`.
/// alpha : ratio de `grisification`.
/// e.g. si level = 0.5, alpha = 0 -> pas de changement.
/// level = 0.5, alpha = 1 -> completement gris (0.5, 0.5, 0.5).
static inline Vector4   vector4_color_toGray(Vector4 const v, float const level, float const alpha);

/// MARK: - Array 2D de pixels (pour mettre dans une texture)
typedef struct PixelArray {
    size_t const width, height; // Dimensions du "bitmap"
    double deltaX, deltaY; // Décalage du centre de l'image.
    double solidWidth, solidHeight; // Espace "occupé" (typiquement plus petit que width/height)
    PixelRGBA pixels[1];
} PixelArray;


PixelArray* PixelArray_createEmpty(size_t width, size_t height);
PixelArray* PixelArray_createSubRegion(PixelArray const* src, RectangleUint region);
PixelArray* PixelArray_createFromBitmapFileOpt(const char* path, bool flipY);
/// Chargement de pixels depuis une image png. Implémentation dépend de l'OS.
PixelArray* PixelArray_createFromPngFileOpt(const char* pngPathOpt, bool showError);
PixelArray* PixelArray_createFromSvgFileOpt(const char* svgPath,
                                 size_t height, bool showError);

/// Référence d'une région de pixels dans un PixelArray;
typedef struct PixelsRegion {
    PixelArray const* paRef;
    RectangleUint     region;
    // marge ?
} PixelsRegion;

void pixelarray_copyAt(PixelArray const* src, PixelArray* dst, UintPair dstOrigin);
void pixelarray_copyRegionAt(PixelArray const* src, RectangleUint srcRegion,
                                 PixelArray* dst, UintPair dstOrigin);

// MARK: - Dessin de png
#define TEXTURE_PNG_NAME_SIZE 32
// Infos à passer pour initialiser les images png.
typedef struct PngInfo {
    char     name[TEXTURE_PNG_NAME_SIZE];
    uint32_t m, n;    // Subdivisions (nombre de tiles) en x/y.
    uint32_t flags;
} PngInfo;

// MARK: - Liste de couleurs (en pixels et vector4)
// Quelques couleurs pratiques pour testing.
// Pour la struct Pixel, les couleurs sont dans l'ordre RGBA.
//  └-> *Attention : alpha -> plus significatifs, red -> moins significatifs.*
#define pixelbgra_black  (PixelRGBA) { 0xFF000000 }
#define pixelbgra_white  (PixelRGBA) { 0xFFFFFFFF }
#define pixelbgra_red    (PixelRGBA) { 0xFF0000FF }
#define pixelbgra_green  (PixelRGBA) { 0xFF00FF00 }
#define pixelbgra_blue   (PixelRGBA) { 0xFFFF0000 }
#define pixelbgra_yellow (PixelRGBA) { 0xFF00FFFF }

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
#define color4_blue_deepSea (Vector4) {{ 0.07, 0.20, 0.34 }}
#define color4_purple (Vector4) {{ 0.8, 0, 0.8, 1 }}
#define color4_purble_china_pink (Vector4) {{ 0.87, 0.44, 0.63, 1 }}
#define color4_purble_electric_indigo (Vector4) {{ 0.44, 0.00, 1, 1 }}
#define color4_purble_blue_violet (Vector4) {{ 0.54, 0.17, 0.89, 1 }}

#include "graph_base.inl"

#endif
