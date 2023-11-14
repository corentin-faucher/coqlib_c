//
//  graph_texture.h
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef graph_texture_h
#define graph_texture_h

#define TEXTURE_PNG_NAME_SIZE 32
#include "graph.h"
#include "string_utils.h"
#include "chronometers.h"

/*-- Textures --------------------------------------------*/

// Infos à passer pour initialiser les images png.
typedef struct {
    char     name[TEXTURE_PNG_NAME_SIZE];
    uint     m, n;
} PngInfo;

/*-- Infos d'une texture. (implementation partiellement cachée, depend de l'OS.) --*/
typedef struct Texture_ Texture;

void            Texture_suspend(void);
void            Texture_resume(void);
void            Texture_checkToFullyDraw(ChronoChecker* cc);
void            Texture_checkUnused(void);

void            Texture_setCurrentFont(const char* fontName);
void            Texture_setCurrentFontSize(double newSize);
/// Lors de la terminaison de l'app.
void            _Texture_deinit(void);
/// Init des images png. A caller avant de créer la structure de l'app.
void            Texture_loadPngs(PngInfo const pngInfos[], const uint pngCount);
/*-- Obtenir la texture de png ou string... --*/
Texture*        Texture_sharedImage(uint const pngId);
Texture*        Texture_sharedImageByName(const char* pngName);
Texture*        Texture_sharedConstantString(const char* c_str);
Texture*        Texture_createString(UnownedString str);
void            texture_updateString(Texture* tex, const char* newString);
/// A n'utiliser qu'avec les "owned" texture (pas les shared).
void            texture_destroy(Texture* tex);

// Accès aux données d'une texture.
const PerTextureUniforms* texture_ptu(Texture* tex);
/// Tiling en x, i.e. nombre de colonnes dans le png.
uint            texture_m(Texture *tex);
/// Tiling en y, i.e. nombre de lignes dans le png.
uint            texture_n(Texture *tex);
/// Nombre total de tiles dans l'image.
uint32_t        texture_mn(Texture *tex);
float           texture_ratio(Texture* tex);
float           texture_alpha(Texture* tex);
float           texture_beta(Texture* tex);
Bool            texture_isShared(Texture* tex);

// "private"/"internal"...
//void            _Texture_updateAllLocStrings(void);


#endif /* graph_texture_h */
