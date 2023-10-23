//
//  Mesh.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef Mesh_h
#define Mesh_h

#include "maths.h"

/*-- Vertex --------------------*/
typedef struct {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
} Vertex;

/*-- Mesh ----------------------*/
// Type de primitives -> Metal, voir MTLRenderCommandEncoder.h.
enum MeshPrimitiveType {
    mesh_primitive_point = 0,
    mesh_primitive_line = 1,
    mesh_primitive_lineStrip = 2,
    mesh_primitive_triangle = 3,
    mesh_primitive_triangleStrip = 4,
};
enum MeshCullMode {
    mesh_cullMode_none = 0,
    mesh_cullMode_front = 1,
    mesh_cullMode_back = 2,
};
#warning TODO : check type des indices.
typedef uint32_t mesh_index_t;
typedef struct {
    int vertex_count;       // Le nombre de vertex.
    int vertices_size;      // La taille en bytes de l'array vertices.
    int index_count;        // 0 si triangle strip (pas besoin d'indices de vertex).
//    float width;
//    float height;
    enum MeshPrimitiveType primitive_type;
    enum MeshCullMode cull_mode;
    mesh_index_t *indices;
    Vertex vertices[1];
} Mesh;
/// Mesh partagé par la plupart des surfaces. Init lors du Mesh_init().
extern Mesh* mesh_sprite;
void   Mesh_init(void);
Mesh*  Mesh_createEmpty(uint vertexCount, uint indexCount,
                        enum MeshPrimitiveType primitive_type,
                        enum MeshCullMode cull_mode);
Mesh*  Mesh_createBar(void);
Mesh*  Mesh_createFrame(void);
void   mesh_destroy(Mesh* meshToDelete);
void   mesh_fillVertices(Mesh* mesh, Vertex* verticesSrc, uint count);

/*-- Per instance uniforms --------------------------------*/
/// Les informations graphiques d'un objet particulier.
/// Size est multiple de 16octets (6 * 16 = 96)
typedef __attribute__((aligned(16)))struct {
    Matrix4 model;
    Vector4 color;
    union {
        float tile[2];
        struct { float i, j; };
    };
    float emph;
    float show;
} PerInstanceUniforms;
// Piu par defaut : Matrice identite et afficher (show = 1).
#define PIU_DEFAULT {{ 1.f, 0.f, 0.f, 0.f, \
0.f, 1.f, 0.f, 0.f, \
0.f, 0.f, 1.f, 0.f, \
0.f, 0.f, 0.f, 1.f }, \
{ 1.f, 1.f, 1.f, 1.f }, { 0.f, 0.f }, 0.f, 1.f,}
extern const PerInstanceUniforms piu_default;


/*-- Per frame uniforms --------------------------------*/
/*-- Les graphique pour la frame courante             --*/
typedef struct {
    Matrix4 projection;
    float time;
    float unused1, unused2, unused3;
} PerFrameUniforms;
extern PerFrameUniforms pfu_default;


/*-- Textures --------------------------------------------*/
/*-- Per texture uniforms --------------------------------*/
/*-- Les informations pour le shader d'une texture.     --*/
typedef struct {
    float width, height; // largeur, hauteur en pixels.
    float m, n;          // Tiling en x, y.
} PerTextureUniforms;

enum TextureType {
    texture_png,
    texture_constantString,
    texture_mutableString,
    texture_localizedString,
};
#define TEXTURE_PNG_NAME_SIZE 32
typedef struct {
    char     name[TEXTURE_PNG_NAME_SIZE];
    uint     m, n;
} PngInfo;

/*-- Infos d'une texture. (implementation cachée, depend de l'OS.) --*/
typedef struct Texture_ Texture;

void            Texture_loadPngs(PngInfo const pngInfos[], const uint pngCount);
Texture*        Texture_getPngTexture(uint const pngId);

const PerTextureUniforms* texture_ptu(Texture* texOpt);
uint            texture_m(Texture *texOpt);
uint            texture_n(Texture *texOpt);
float           texture_ratio(Texture* texOpt);
float           texture_alpha(Texture* texOpt);
float           texture_beta(Texture* texOpt);

#endif /* Mesh_h */
