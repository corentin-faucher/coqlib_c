//  graph_font.h
//
//  Police de caractères. Wrapper de freetype ou UIFont/NSFont.
//  Une "font" permet de dessiner des glyphes (dessins de caractères).
//  
// Étapes pour dessiner une string.
// 0.1. Optionel : créer une Font (infos pour dessiner des glyphes).
// 0.2. Optionel : créer une glyph map avec une font (texture où sont dessinés les glyphes).
// 1. On a une c_str (char[] en utf8) que l'on convertie en CharacterArray:
//     c_str -> charArray.
// 2. De l'array de chars, obtenir une `StringGlyphed` à l'aide d'une GlyphMap.
//     charArray -> stringGlyphed. (suite de glyphes)
// 3. Optionnel : on peut scinder la stringGlyphed en plusieurs lignes avec une largeur de ligne définie.
//     stringGlyphed -> linesArray (array de StringGlyphed).
// 4. Utiliser le StringGlyphed (ou StringGlyphArr) pour setter la texture et instances uniforms d'un objet "drawable". 
// -> Voir par exemple `node_string.h` et `nodestring_renderer_updateIUs_`.
//
//  Created by Corentin Faucher on 2025-04-19.
//
#ifndef COQ_GRAPH_FONT_H
#define COQ_GRAPH_FONT_H
#include "graph_base.h"
#include "../utils/util_chars.h"

#define FONT_extra_margin(nearest) (nearest ? 1 : 2)
#define FONT_defaultSize 32
#define FONT_maxSize 200
#define FONT_minSize 12

// MARK: - Fonts
// Structure "privé" avec lien vers font UIFont, NSFont ou freetype.
typedef struct coq_Font CoqFont;
// Info d'init d'une font. Tout est optionel (on peut laisser tout à zéro).
typedef struct CoqFontInit {
    union {
        char const*const    apple_fontNameOpt;    // Pour Apple UIFont/NSFont
        char const*const    freetype_fileNameOpt; // Pour Freetype
        wchar_t const*const dwrite_fontNameOpt; // Pour Dwrite / Microsoft.
    };
    double const     sizeOpt;     // Si 0 -> FONT_defaultSize
    PixelRGB         color;
    bool const       nearest;     // "Pixélisé"
} CoqFontInit;
CoqFont* CoqFont_engine_create(CoqFontInit info);
void     coqfont_engine_destroy(CoqFont** cf);

PixelArray* PixelsArray_engine_createFromCharacter(Character c, CoqFont const* coqFont);
PixelArray* PixelsArray_engine_test_createFromString_(const char* c_str, CoqFont const* coqFont);

// Infos sur les dimensions de la police.
typedef struct CoqFontDims {
    /// Décalage pour centrer les glyphes en y.
    float  deltaY;
    /// Hauteur total requise pour dessiner les glyphes en pixels *avec* les fioritures qui depassent,
    ///  i.e. `ascender - descender + extraYmargins`.
    float  fullHeight;
    /// Hauteur de la hitbox en pixels sans les fioritures.
    /// Choix : `0.5*(capheight+ascender) - descender` (on va de descender à entre capHeight et ascender).
    /// ** -> Notez : les dimensions "relatives" des glyphes le sont par rapport à `solidHeight` **,
    float  solidHeight;
    /// La "font size" donné lors de l'init.
    double fontSize;
    bool  nearest; // La font est en mode pixélisé.
} CoqFontDims;
CoqFontDims coqfont_dims(CoqFont const* cf);

// Définie où sont les fonts et la font par défaut (avec extension)
void  CoqFont_freetype_init_(const char* fontsPathOpt, 
                             const char* defaultFontFileNameOpt,
                             const char* defaultEmojiFileNameOpt);
void CoqFont_freetype_quit_(void);

void CoqFont_test_printAvailableFonts(void);

#endif
