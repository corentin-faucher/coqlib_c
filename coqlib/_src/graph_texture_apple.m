//
//  graph_texture_apple.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include "graph.h"
#include "graph_texture_apple.h"
#include "utils.h"

typedef struct Texture_ {
    PerTextureUniforms ptu;
    /// Tiling en x, y. Meme chose que ptu -> m, n.
    uint32           m, n;
    /// Ratio sans les marges en x, y, e.g. (0.95, 0.95).
    /// Pour avoir les dimension utiles de la texture.
    float            alpha, beta;
    enum TextureType type;
    float            ratio;  // Ration w / h d'une tile. -> ratio = (w / h) * (n / m).
    char             name[TEXTURE_PNG_NAME_SIZE];
    char*            fontName;
    id<MTLTexture>   mtlTexture;
} Texture;

static PerTextureUniforms _ptu_default = {8, 8, 1, 1};

static MTKTextureLoader* _textureLoader = NULL;
/// Liste des pngs loadés.
static Texture*          _pngTexArray = NULL;
static uint              _pngTexCount = 0;

void  _texture_initAsPng(Texture* const tex, PngInfo* const info) {
    // Init des champs
    tex->ptu.m = (float)info->m;
    tex->ptu.n = (float)info->n;
    tex->m =     info->m;
    tex->n =     info->n;
    tex->alpha = 1.f;
    tex->beta =  1.f;
    tex->type =  texture_png;
    strncpy(tex->name, info->name, TEXTURE_PNG_NAME_SIZE);
    
    // Dessiner le png
    if(_textureLoader == NULL) { printerror("texture loader not init."); return; }
    NSString *pngName = [NSString stringWithUTF8String:tex->name];
    NSURL *pngUrl = [[NSBundle mainBundle]
                     URLForResource:pngName withExtension:@"png" subdirectory:@"pngs"];
    if(pngUrl == NULL) { printerror("cannot init url for %s.", tex->name); return; }
    NSError *error = nil;
    id<MTLTexture> mtlTexture = [_textureLoader newTextureWithContentsOfURL:pngUrl
                                       options:@{MTKTextureLoaderOptionSRGB: @false}
                                         error:&error];
    if(error != nil || mtlTexture == nil) { printerror("cannot load %s png.", tex->name); return; }
    float width = (float)[mtlTexture width];
    float height = (float)[mtlTexture height];
    tex->ptu.width = width;
    tex->ptu.height = height;
    tex->ratio = width / height * tex->ptu.n / tex->ptu.m;
    tex->mtlTexture = mtlTexture;
}

void Texture_init(id<MTLDevice> const device) {
    if(_textureLoader != NULL) {
        printerror("Texture already init.");
        return;
    }
    _textureLoader = [[MTKTextureLoader alloc] initWithDevice:device];
}

void Texture_loadPngs(PngInfo const pngInfos[], const uint pngCount) {
    if(_textureLoader == NULL) {
        printerror("Texture not init.");
        return;
    }
    _pngTexArray = calloc(pngCount, sizeof(Texture));
    _pngTexCount = pngCount;
    // Pour l'instant, on charge tout... (faire des minis... ?)
    Texture *tex = _pngTexArray;
    PngInfo *info = (PngInfo*)pngInfos;
    Texture *end = &tex[pngCount];
    while(tex < end) {
        _texture_initAsPng(tex, info);
        tex ++;
        info ++;
    }
}

Texture*        Texture_getPngTexture(uint const pngId) {
    if(pngId >= _pngTexCount) {
        printerror("Invalid png id %d. Max id is %d.", pngId, _pngTexCount);
        return NULL;
    }
    return &_pngTexArray[pngId];
}



const PerTextureUniforms* texture_ptu(Texture* tex) {
    if(!tex) return &_ptu_default;
    return &(tex->ptu);
}
uint            texture_m(Texture *tex) {
    if(!tex) return 1;
    return tex->m;
}
uint            texture_n(Texture *tex) {
    if(!tex) return 1;
    return tex->n;
}
float           texture_ratio(Texture* tex) {
    if(!tex) return 1.f;
    return tex->ratio;
}
float           texture_alpha(Texture* tex) {
    if(!tex) return 1.f;
    return tex->alpha;
}
float           texture_beta(Texture* tex) {
    if(!tex) return 1.f;
    return tex->beta;
}

id<MTLTexture>  texture_MTLTexture(Texture* tex) {
    if(!tex) return nil;
    return tex->mtlTexture;
}

