//
//  graph.h
//  Liste des structures "uniforms",
//  i.e. les structures passées aux shaders.
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef COQ_GRAPH_BASE_H
#define COQ_GRAPH_BASE_H

#include "maths/math_base.h"

/*-- Per instance uniforms --------------------------------*/
/// Les informations graphiques d'un objet particulier
/// à passer aux shaders/gpu.
/// (Struct doit être aligned avec size multiple de 16octets : 7 * 16 = 112)
typedef __attribute__((aligned(16)))struct {
    Matrix4 model;
    Vector4 color;
    union {
        float tile[2];
        struct { float i, j; };
    };
    float    emph;
    float    show;
    uint32_t flags;
    float    unused1, unused2, unused3;
} PerInstanceUniforms;

// Test... Wrapper d'un buffer d'uniform Metal...
typedef struct UniformBuffer UniformBuffer;
UniformBuffer* UniformBuffer_create(size_t size);
void           uniformbuffer_setDataAt(UniformBuffer* ub, const void *newData, size_t size, size_t offset);
void           uniformbufferref_destroyAndNull(UniformBuffer** const ubToDeleteRef);

const void* MTLBuffer_createAndGetCPointer(size_t size);
void        mtlbufferCptr_setDataAt(const void* mtlBufferCPtr, const void *newData, size_t size, size_t offset);
void        mtlbufferCPtrRef_releaseAndNull(const void** mtlBufferRef);

// Piu par defaut : Matrice identite, blanc, tile (0, 0), show == 1.
#define PIU_DEFAULT \
{{{ 1.f, 0.f, 0.f, 0.f, \
   0.f, 1.f, 0.f, 0.f, \
   0.f, 0.f, 1.f, 0.f, \
   0.f, 0.f, 0.f, 1.f }}, \
 {{ 1.f, 1.f, 1.f, 1.f }}, \
 {{ 0.f, 0.f }}, 0.f, 1.f, \
   0u,  0.f, 0.f, 0.f }
extern const PerInstanceUniforms piu_default;

/*-- Per texture uniforms --------------------------------*/
/*-- Les informations pour le shader d'une texture.     --*/
typedef __attribute__((aligned(16)))struct {
    float width, height; // largeur, hauteur en pixels.
    float m, n;          // Tiling en x, y.
} PerTextureUniforms;
#define PTU_DEFAULT \
{ 8, 8, 1, 1 }
extern const PerTextureUniforms ptu_default;

/*-- Per frame uniforms --------------------------------*/
/*-- Les shader constantes  pour la frame courante    --*/
typedef __attribute__((aligned(16)))struct {
    Matrix4 projection;
    float time;
    float unused1, unused2, unused3;
} PerFrameUniforms;
extern PerFrameUniforms pfu_default;

#endif
