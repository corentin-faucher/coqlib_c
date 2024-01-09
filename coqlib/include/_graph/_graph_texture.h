//
//  graph_texture.h
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef _coq_graph_texture_h
#define _coq_graph_texture_h

#include "coq_utils.h"
#include "_math/_math_chrono.h"
#include "_graph/_graph_.h"

#define TEXTURE_PNG_NAME_SIZE 32

/*-- Textures --------------------------------------------*/

// Infos à passer pour initialiser les images png.
typedef struct {
    char     name[TEXTURE_PNG_NAME_SIZE];
    uint32_t m, n;
    bool     nearest;
} PngInfo;

/*-- Infos d'une texture. (implementation partiellement cachée, depend de l'OS.) --*/
typedef struct Texture_ Texture;

void            Texture_setCurrentFont(const char* fontName);
void            Texture_setCurrentFontSize(double newSize);
double          Texture_currentFontSize(void);

/// Init des images png. A caller avant de créer la structure de l'app.
void            Texture_loadPngs(PngInfo const pngInfos[], const uint32_t pngCount);

/*-- Obtenir la texture de png ou string... --*/
Texture*        Texture_sharedImage(uint32_t const pngId);
Texture*        Texture_sharedImageByName(const char* pngName);
Texture*        Texture_sharedConstantString(const char* c_str);
Texture*        Texture_createString(UnownedString str, bool isShared);

/*-- Fonctions sur une texture ---------------*/
/// A n'utiliser qu'avec les "owned" texture (pas les shared).
void            texture_destroy(Texture* tex);
/// Mise à jour d'une texture de type "string mutable", i.e. non constant et non localisée.
void            texture_updateString(Texture* tex, const char* newString);

// Accès aux données d'une texture.
const PerTextureUniforms* texture_ptu(Texture* tex);
/// Style pixelise (pas linear/flou)
bool            texture_isNearest(Texture* tex);
/// Tiling en x, i.e. nombre de colonnes dans le png.
uint32_t        texture_m(Texture *tex);
/// Tiling en y, i.e. nombre de lignes dans le png.
uint32_t        texture_n(Texture *tex);
/// Nombre total de tiles dans l'image.
uint32_t        texture_mn(Texture *tex);
float           texture_ratio(Texture* tex);
float           texture_alpha(Texture* tex);
float           texture_beta(Texture* tex);
bool            texture_isShared(Texture* tex);
const char*     texture_string(Texture* tex);

// "private"/"internal"...
void            _Texture_suspend(void);
void            _Texture_resume(void);
void            _Texture_deinit(void);
void            _Texture_checkToFullyDrawAndUnused(ChronoChecker* cc, int64_t timesUp);


#endif /* graph_texture_h */
