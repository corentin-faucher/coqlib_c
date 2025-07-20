//
//  graph_texture_private.h
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-17.
//
#ifndef COQ_GRAPH_TEXTURE_PRIVATE_H
#define COQ_GRAPH_TEXTURE_PRIVATE_H

#include "graph_texture.h"

typedef struct Texture {
    TextureDims      dims;
    
    uint32_t         flags;
    int64_t          touchTime;
    
    char*            string;  // Copie de la String ou nom du png, pour s'il faut redessiner la texture.
//    PixelBGRA*       pixelsOpt; // Copie des pixels pour l'édition.
    // Metal/OpenGl
    union {
        struct {
            const void*    mtlTex_cptr;
            const void*    mtlTex2_cptr;
            const void*    mtlTexTmp_cptr;
        };
        struct {
            uint32_t glTexId;
            uint32_t glTex2Id;
            uint32_t glTexTmpId;
        };
    };
} Texture;

// Il existe des textures pas encore dessinées...
extern bool     Texture_needToFullyDraw_;

void            texture_initEmpty_(Texture* tex);
void            texture_deinit_(void* tex_void);

// MARK: - Engine dependant methods (Implementer avec Metal ou OpenGL)
/// Init de la texture avec des pixels et dimensions.
void           texture_engine_load_(Texture* tex, size_t width, size_t height, bool asMini, PixelBGRA const* pixelsOpt);
/// Cas particulier de load. Equivalent de `Pixels_engine_createOptFromPng` + `texture_engine_load_`.
void           texture_engine_tryToLoadAsPng_(Texture* tex, bool isMini);

void           texture_engine_releaseAll_(Texture* tex);
void           texture_engine_releasePartly_(Texture* tex);



#endif

