//
//  graph_texture.h
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef COQ_GRAPH_TEXTURE_H
#define COQ_GRAPH_TEXTURE_H

#include "maths/math_chrono.h"
#include "graphs/graph_base.h"
#include "utils/utils_string.h"

#define TEXTURE_PNG_NAME_SIZE 32

/*-- Textures --------------------------------------------*/

// Infos à passer pour initialiser les images png.
typedef struct {
    char     name[TEXTURE_PNG_NAME_SIZE];
    uint32_t m, n;
    bool     nearest;
    bool     isCoqlib;
} PngInfo;

/*-- Infos d'une texture. (implementation partiellement cachée, depend de l'OS.) --*/
typedef struct Texture_ Texture;

#pragma mark - Texture Global Functions

/// Mise en pause (libère les pixels des textures)
void            Texture_suspend(void);
/// Sortie de pause... ne fait pas grand chose...
/// (ne redessine pas les textures, elles sont redessinées avec `Texture_checkToFullyDrawAndUnused`)
void            Texture_resume(void);
/// Nettoyage en fin de programme.
void            Texture_deinit(void);
/// A faire de temps en temps pour dessiner pleinement les textures.
void            Texture_checkToFullyDrawAndUnused(ChronoChecker* cc, int64_t timesUp);

#pragma mark - Font des strings

void            Texture_setCurrentFont(const char* fontName);
void            Texture_setCurrentFontSize(double newSize);
double          Texture_currentFontSize(void);

#pragma mark - Constructeurs de textures

/*-- Obtenir la texture de png ou string... --*/
Texture*        Texture_sharedImage(uint32_t const pngId);
Texture*        Texture_sharedImageByName(const char* pngName);
Texture*        Texture_retainString(StringDrawable str);
/// Libérer (si besoin) la texture qui n'est plus utilisé.
void            textureref_releaseAndNull_(Texture** const texRef);

#pragma mark - Getters, mise à jour...

/// Change la texture constante/shared pour une autre.
void            textureref_exchangeSharedStringFor(Texture** const texRef, StringDrawable str);
/// Mise à jour d'une texture de type "string mutable", i.e. non constant et non localisée.
void            texture_updateMutableString(Texture* tex, const char* newString, bool forceRedraw);
// Accès aux données d'une texture.
const PerTextureUniforms* texture_ptu(Texture* tex);
/// Style pixelise (pas linear/flou)
bool            texture_isNearest(Texture* tex);
/// Texture d'un png (shared, pas besoin de release)
bool            texture_isSharedPng(Texture* tex);
/// Tiling en x, i.e. nombre de colonnes dans le png.
uint32_t        texture_m(Texture *tex);
/// Tiling en y, i.e. nombre de lignes dans le png.
uint32_t        texture_n(Texture *tex);
/// Nombre total de tiles dans l'image.
uint32_t        texture_mn(Texture *tex);
float           texture_ratio(Texture* tex);
float           texture_alpha(Texture* tex);
float           texture_beta(Texture* tex);
const char*     texture_string(Texture* tex);


#endif /* graph_texture_h */
