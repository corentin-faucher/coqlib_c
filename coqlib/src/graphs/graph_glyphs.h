//
//  graph_glyphs.h
//  Texture contenant les glyphs d'une police de caractères.
//
//  Created by Corentin Faucher on 2024-09-11.
//

#ifndef graph_glyphs_h
#define graph_glyphs_h

#include "graph_texture.h"

#pragma mark - Info d'un glyph (dessin d'un char)
/// Les infos pour placer les caractères les un après les autres (pour former une string).
/// La hauteur de référence est : hRef = capHeight + descender.
typedef struct GlyphInfo {
    Rectangle uvRect;        // Coordonées uv dans la texture.
    // Les dimension sont relative à la hauteur de ref `solidHeight`.
    float     relGlyphX;     // Décalage en x (en ratio à la hauteur de ref.) 
    float     relGlyphWidth; // Largeur du glyph/dessin (en ration à la hauteur de ref)
    float     relSolidWidth; // Ratio w/h de la "solid box", i.e. vrai espace occupé en x. 
                 // (Typiquement relSolidWidth < relGlyphWidth pour une glyph qui déborde, 
                 // e.g. des lettres attachées.)
} GlyphInfo;


#pragma mark - Map de glyphs (pour dessiner les strings), c'est une sous-struct de Texture...
typedef struct FontGlyphMap FontGlyphMap;
typedef struct FontGlyphMapCustomChars FontGlyphMapCustomChars;
void           fontglyphmap_deinit(FontGlyphMap* gm);
/// Une texture qui stocke les glyphs d'une police de caractères.
FontGlyphMap*  FontGlyphMap_create(const char* fontNameOpt, double fontSize, 
                      size_t textureWidthOpt, bool nearest, 
                      FontGlyphMapCustomChars* customCharsOpt);
void           fontglyphmapref_releaseAndNull(FontGlyphMap** fgmOptRef);


GlyphInfo      fontglyphmap_getGlyphInfoOfChar(FontGlyphMap* gm, Character c);
Texture*       fontglyphmap_getTexture(FontGlyphMap* gm);
float          fontglyphmap_getRelY(FontGlyphMap* gm);
float          fontglyphmap_getRelHeight(FontGlyphMap* gm);

#pragma mark - Sous structure de FontGlyphMap pour char customs (pris d'une texture)
typedef struct FontGlyphMapCustomChars {
    size_t     count;   // Nombre de chars customizés.
    Texture*   texture; // Texture où trouver les customs chars (shared).
    Character* chars;   // Liste des chars customizés.
    Rectangle* uvRects; // Liste des rectangles où trouver les chars customizés dans la texture.
} FontGlyphMapCustomChars;


#pragma mark - Un caractère avec ses dimensions.
/// Info du glyph d'un char à garder en mémoire pour l'affichage (utile pour overrider `updateModel`)
typedef struct CharacterGlyphed {
    Character c;
    GlyphInfo info;
    bool      firstOfWord;
} CharacterGlyphed;


#pragma mark - StringGlyphedInit : info à fournir pour créer une string avec glyphs.
/// Structure temporaire pous initialiser une StringGlyphed (et une NodeString).
typedef struct StringGlyphedInit {
    /// (Ici, la `c_str` est un pointeur unowned/shared, i.e. sera copié mais pas deallocated.)
    const char*   c_str;
    /// Il faut chercher la version localisé avec `String_createLocalized`...
    bool          isLocalized;
    /// Optionnel, la glyph map à utiliser (un glyph map par font). 
    /// Si absent une glyph map sera créée avec la police par défaut. 
    FontGlyphMap *glyphMapOpt;
    /// Espacement supplémentaire sur les côtés (en % de la hauteur, typiquement 0.5 est bien).
    float         x_margin;
    /// Espacement supplémentaire entre les caractères. (en % de la hauteur, 0 par défaut, peut être positif ou négatif)
    float         spacing;
} StringGlyphedInit;

#endif /* graph_glyphs_h */
