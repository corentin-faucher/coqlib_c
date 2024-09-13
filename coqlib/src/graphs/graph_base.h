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

typedef struct Texture Texture;

#pragma mark -- Per instance uniforms --------------------------------
/// Les informations graphiques d'un objet particulier
/// à passer au vertex shader.
/// (Struct doit être aligned avec size multiple de 16octets (4 floats) : 7 * 16 = 112)
/// (N'est utile que pour les drawables, les données sont libres pour les autres type de noeuds. -> voir `node_base.h`.)
typedef __attribute__((aligned(16))) struct InstanceUniforms {
    Matrix4   model;  // Commun à tout les noeuds
    
    Vector4   color;  // Que pour les drawables (12 floats / 48 bytes)
    Rectangle uvRect; // Coord uv de la texture du drawable.
    uint32_t  flags;  // Flags custom pour shader.
    float     show;   // Flag analogique d'affichage du drawable.
    float     extra1; // Floats extra customizables, e.g. emphase, flip, frequency...
    float     extra2;
} InstanceUniforms;

// Piu par defaut : Matrice identite, blanc, tile (0,0)x(1,1), show == 1.
extern const InstanceUniforms InstanceUnifoms_default;

uint32_t piu_getTileI(const InstanceUniforms* piu);

// Buffer vers les uniforms d'instance. Implémentation dépend de l'engine graphique.
typedef struct IUsBuffer {
    uint32_t const          max_count;
    uint32_t                actual_count;
    InstanceUniforms* const ius;    // Array des infos d'instances.
    const void* const       _mtlBuffer_cptr;  // Buffer Metal
} IUsBuffer;

/// Création du buffer. 
void   piusbuffer_init_(IUsBuffer* piusbuffer, uint32_t count);
/// Libère l'espace du buffer (et array de piu si nécessaire)
void   piusbuffer_deinit_(IUsBuffer* piusbuffer);
void   piusbuffer_setAllTo(IUsBuffer* iusBuffer, InstanceUniforms iu);
void   piusbuffer_setAllActiveTo(IUsBuffer* iusBuffer, InstanceUniforms iu);

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

// Quelques couleurs pratiques pour testing...
// Pour les autres couleurs voir `graph_color.h` et `PixelBGRA vector4_color_toPixelBGRA(Vector4 v)`
extern const PixelBGRA pixelbgra_black;
extern const PixelBGRA pixelbgra_white;
extern const PixelBGRA pixelbgra_red;
extern const PixelBGRA pixelbgra_yellow;


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
