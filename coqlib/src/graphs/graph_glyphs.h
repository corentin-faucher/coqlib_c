//
//  graph_glyphs.h
//  Les glyphes : caractères dessinés avec une police.
//
//  Created by Corentin Faucher on 2024-09-11.
//

#ifndef graph_glyphs_h
#define graph_glyphs_h

#include "graph_font.h"
#include "graph_texture.h"

// MARK: - Info d'un glyph (dessin d'un char)
/// Les infos pour placer les caractères les un après les autres (pour former une string).
/// Pour le décalage en x `relGlyphX`, c'est le déplacement à faire pour positionner le glyph en x,
/// e.g. pour le 'j' qu'il faut décaler vers la gauche, on pourrait avoir `relGlyphX = -0.2`.
typedef struct GlyphInfo {
    Rectangle uvRect;        // Coordonées uv dans la texture GlyphMap (vois plus bas pour GlyphMap).
    // **Les dimension sont relative à la hauteur de ref `solidHeight` de la font.**
    float     relGlyphX;     // Décalage en x pour centrer (en ratio à la hauteur de ref.) typiquement ~ 0.
    float     relGlyphY;     // Décalage en y pour centrer.
    float     relGlyphWidth; // Largeur du glyph (avec fioritures).
    float     relGlyphHeight;
    // Ratio w/h de la "solid box", i.e. vrai espace occupé en x. 
    // (Typiquement relSolidWidth < relGlyphWidth pour une glyph qui déborde, 
    // e.g. des lettres attachées.)
    float     relSolidWidth; 
    // float  relSolidHeight == 1, par définition.
} GlyphInfo;

/// CharacterGlyphed : un caractère avec ses dimensions -> Élément de StringGlyphed.
/// Info du glyph d'un char à garder en mémoire pour le rendering.
typedef struct CharacterGlyphed {
    Character c;
    GlyphInfo glyph;
    // Position (relative à solidHeight) du glyph par rapport au début de la StringGlyphed.
    // Tient compte du decalage deltaX des glyphs. Si en arabe, les xRel < 0.
    // Ne tient pas compte de la marge x_margin.
    float     xRel; 
    // Début d'un mot. Utile pour scinder une longue string de glyphs.
    bool      firstOfWord;
} CharacterGlyphed;

// MARK: - Map de glyphs (pour dessiner les strings), 
// La GlyphMap est une texture où on dessine les glyphs de caractères à l'aide d'une Font de caractères. 
typedef struct GlyphMap GlyphMap;
/// Infos pour init une GlyphMap (tout est optionel... on peut juste tout laisser à zéro)
typedef struct GlyphMapInit {
    CoqFontInit fontInit;        // Font à utiliser pour dessiner les glyphes.
    size_t      textureWidthOpt; // Si 0, choix proportionnel à fontSize.
    // Liste de chars "custom" dessiner a partir d'une texture (optionnel)
    // (mais si count > 0 -> tout doit être défini) 
    Texture    *customChars_texOpt;   // Texture où trouver les customs chars
    Character  *customChars_charsOpt; // Liste des chars customizés
    Rectangle  *customChars_uvRectsOpt;// Liste des rectangles où trouver les chars customizés dans la texture
    size_t      customChars_count; // Nombre de chars customizés.
} GlyphMapInit;

/// Une texture qui stocke les glyphs d'une police de caractères.
GlyphMap*       GlyphMap_create(GlyphMapInit info);
void            glyphmapref_releaseAndNull(GlyphMap *const* fgmOptRef);

GlyphInfo       glyphmap_glyphInfoOfChar(GlyphMap *gm, Character c);
CoqFont const*  glyphmap_font(GlyphMap const* gm);
CoqFontDims     glyphmap_fontDims(GlyphMap const* gm);
Texture*        glyphmap_texture(GlyphMap* gm);

/// Init la glyphmap utilisée par défaut.
void            GlyphMap_default_init(GlyphMapInit info);
void            GlyphMap_default_deinit(void);
GlyphMap*       GlyphMap_default(void);
bool            GlyphMap_default_isInit(void);
CoqFont*        GlyphMap_default_font(void);
Texture*        GlyphMap_default_texture(void);

// MARK: - StringGlyphed : String avec info pour dessiner
typedef struct StringGlyphed StringGlyphed;
// Étapes pour dessiner une string.
// 1. On a une c_str (char[] en utf8) que l'on convertie en CharacterArray:
//     c_str -> charArray.
// 2. De l'array de chars, obtenir des glyphes avec leurs GlyphMap :
//     charArray -> stringGlyphed.
// 3. (Optionnel) On peut scinder la stringGlyphed en plusieurs lignes avec une largeur de ligne définie.
//     stringGlyphed -> linesArray (array de StringGlyphed).
// 4. Utiliser le StringGlyphed (ou StringGlyphArr) pour setter les instances uniforms et texture du rendering. 
//    Voir par exemple NodeString (un DrawableMulti où chaque instance est le glyph d'un char) et `nodestring_renderer_updateIUs_`.

/// Info pour initialiser une string devant être dessinée, i.e. StringGlyphed.
/// (Tout les arguments sont "optionnels"...)
typedef struct StringGlyphedInit {
    /// La string à afficher (shared/unowned). Si NULL, init "empty" à éditer.
    const char*   c_str;
    /// Il faut chercher la version localisé avec `String_createLocalized`...
    bool          isLocalized;
    /// Lecture de droite à gauche (arabe). ** Si isLocalized, isRightToLeft est définie par la langue. **
    bool          isRightToLeft;
    /// Optionnel, la glyph map à utiliser (un glyph map par font). 
    /// Si absent une glyph map sera créée avec la police par défaut. 
    /// Doit rester disponible (static/shared) durant la durée de vie de la StringGlyphed.
    GlyphMap     *glyphMapOpt;
    /// Espacement supplémentaire sur les côtés (en % de la hauteur, typiquement 0.5 est bien).
    float         x_margin;
    /// Espacement supplémentaire entre les caractères. (en % de la hauteur, 0 par défaut, peut être positif ou négatif)
    float         spacing;
    /// Pour strings éditables, le nombre maximal de chars.
    size_t        maxCountOpt;
} StringGlyphedInit;

StringGlyphed* StringGlyphed_create(StringGlyphedInit init);
StringGlyphed* StringGlyphed_createCopy(StringGlyphed const* src);
// (pas de stringglyphed_denit, on peut dealloc directement)
void           stringglyphed_setChars(StringGlyphed* sg, const CharacterArray *ca);
/// Les infos pour dessiner une string
typedef struct StringGlyphedToDraw {
    CharacterGlyphed const*      c; // Itérateur de char glypled.
    CharacterGlyphed const*const beg;
    CharacterGlyphed const*const end;
    // Position de la fin de la string. (négatif si arabe/rightToLeft)
    // Puisque `xBegRel == 0` par définition, abs(xEndRel) est la largeur de la string et 0.5*xEndRel le centre.
    float const xEndRel; 
    float const x_margin;
} StringGlyphedToDraw;
StringGlyphedToDraw stringglyphed_getToDraw(StringGlyphed const* sg);
Texture* stringglyphed_glyphMapTexture(StringGlyphed* sg);
size_t stringglyphed_maxCount(StringGlyphed const* sg);

// MARK: - StringGlyphArr, String scindé en plusieurs lignes.
typedef struct StringGlyphArr {
    size_t          lineCount;
    size_t          _maxLineCount;
    StringGlyphed*  strOpts[1]; // (Array de pointeurs StringGlyphed optionels)
} StringGlyphArr;
/// Fonction pour scinder un StringWithGlyph en plusieurs lignes.
/// `trailingSpace` est le char d'"affichage" de l'espace en bout de ligne, e.g. `⎵`.
/// Si trailingSpace == `\0`, l'espace de bout de ligne est retiré.
StringGlyphArr* StringGlyphArr_create(const StringGlyphed* str, float lineWidth,
                                      Character trailingSpaceOpt);
void stringglypharr_destroyAndNull(StringGlyphArr*const*const strArrRef);

#endif /* graph_glyphs_h */


//typedef struct {
//    StringGlyphed const* sg;
//    Vector2 center;
//    Vector2 scalesOpt;
//    bool    centered;
//} GlyphSimpleArrayInit;
/// Conversion d'une StringGlyphed en un array de GlyphSimple (version minimaliste de StringGlyphed).
//GlyphSimple* GlyphSimpleArray_createFromStringGlyphed(GlyphSimpleArrayInit init); 
