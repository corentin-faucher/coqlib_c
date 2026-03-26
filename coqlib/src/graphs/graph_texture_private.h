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
    // Metal/OpenGl
    union {
        struct {
            const void*    mtlTex0Opt;
            const void*    mtlTex1Opt;
            const void*    mtlTexMiniOpt;
        };
        struct {
            uint32_t glTex0Id;
            uint32_t glTex1Id;
            uint32_t glTexMiniId;
        };
    };
    char*            string;  // Copie de la String ou nom du png, pour s'il faut redessiner la texture.
    PixelArray*  miniPixelsOpt;
    PixelArray*  pixelsOpt;
    
    int64_t          touchTime;
    uint32_t         flags;
    uint32_t         rendFlags;
    // Pour suivre la version de textures éditables.
    uint16_t         version_pixels; // Version édité
    uint16_t         version_rend;   // Version "live"
} Texture;

void    texture_render_deinit_(void* tex_void);
void    texture_render_releaseBuffers_(void* tex_void);

enum {
// Flags "privés"...
    tex_flag__png             = 0x0100,
    tex_flag__editing         = 0x0400,
    
    tex_flag_rend_bufferSet    = 0x1000,
    tex_flag_rend_bufferMiniSet = 0x4000,
    tex_flags_rend_buffersSet = 0x7000,
    tex_flag_rend_readBuff1   = 0x8000,
};


#endif

