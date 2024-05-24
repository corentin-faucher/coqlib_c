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

/*-- Per instance uniforms --------------------------------*/
/// Les informations graphiques d'un objet particulier
/// à passer aux shaders/gpu.
/// (Struct doit être aligned avec size multiple de 16octets : 7 * 16 = 112)
typedef __attribute__((aligned(16))) struct PerInstanceUniforms {
    Matrix4 model;
    Vector4 color;
    Rectangle uvRect;
    float    emph;
    float    show;
    uint32_t flags;
    float    unused1;
} PerInstanceUniforms;

uint32_t piu_getTileI(const PerInstanceUniforms* piu);

/// Espace (tile) occupé par une sprite dans la grille d'une texture.
/// e.g. si ij0 = {1, 2} et Dij = {2, 2} pour un png avec un grille m = 10, n = 5,
/// on a uv0 = { 0.1, 0.4 }, Duv = { 0.2, 0.4 } 
typedef struct InstanceTile {
    union {
        uint32_t ij[2];
        struct { uint32_t i, j; };
    };
    union {
        uint32_t Dij[2];
        struct { uint32_t Di, Dj; };
    };
} InstanceTile;

Rectangle instancetile_toUVRectangle(InstanceTile const it, uint32_t m, uint32_t n);

// Buffer vers les uniforms d'instance. Implémentation dépend de l'engine graphique.
typedef struct PIUsBuffer {
    uint32_t             max_count;
    uint32_t             actual_count;
    size_t               _size; // = piu_count * sizeof(PerInstanceUniforms)
    PerInstanceUniforms* pius;
    const void*          _mtlBuffer_cptr;  // Buffer Metal
} PIUsBuffer;

/// Création du buffer. 
void   piusbuffer_init_(PIUsBuffer* piusbuffer, uint32_t count);
/// Libère l'espace du buffer (et array de piu si nécessaire)
void   piusbuffer_deinit_(PIUsBuffer* piusbuffer);

// Piu par defaut : Matrice identite, blanc, tile (0, 0), show == 1.
#define PIU_DEFAULT \
{{{ 1.f, 0.f, 0.f, 0.f, \
   0.f, 1.f, 0.f, 0.f, \
   0.f, 0.f, 1.f, 0.f, \
   0.f, 0.f, 0.f, 1.f }}, \
 {{ 1.f, 1.f, 1.f, 1.f }}, \
 {{ 0.f, 0.f, 1.f, 1.f }}, \
   0.f, 1.f, 0u, 0.f }
extern const PerInstanceUniforms piu_default;

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
typedef __attribute__((aligned(16)))struct {
    Matrix4 projection;
    float time;
    float unused1, unused2, unused3;
} PerFrameUniforms;
extern PerFrameUniforms pfu_default;

#pragma mark - Structure des pixels 24 ou 32 bits.
typedef struct PixelBGR {
    uint8_t b, g, r;
} PixelBGR;
typedef union PixelBGRA {
    uint32_t data;
//    float    c_arr[4];
    struct { uint8_t b, g, r, a; };
    struct { PixelBGR bgr; /* uint8_t a; */ };
} PixelBGRA;

#endif
