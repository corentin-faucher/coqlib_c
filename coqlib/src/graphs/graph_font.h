//  graph_font.h
//  Police de caractères. Wrapper de freetype ou UIFont/NSFont.
//
//  Created by Corentin Faucher on 2025-04-19.
//
#ifndef COQ_GRAPH_FONT_H
#define COQ_GRAPH_FONT_H
#include "graph_base.h"
#include "../utils/util_char_and_keycode.h"

#define FONT_extra_margin(nearest) (nearest ? 1 : 2)

// MARK: - Fonts
// Structure "privé" avec lien vers font UIFont, NSFont ou freetype.
typedef struct coq_Font CoqFont;

typedef struct CoqFontInit {
    char const*const nameOpt;     // Pour UIFont/NSFont
    char const*const fileNameOpt; // Pour Freetype
    double const     sizeOpt;
    PixelBGR         color;
    bool const       nearest;     // "Pixélisé"
} CoqFontInit;
CoqFont* CoqFont_engine_create(CoqFontInit info);
void     coqfont_engine_destroy(CoqFont** cf);

PixelBGRAArray* Pixels_engine_createArrayFromCharacter(Character c, CoqFont const* coqFont);
PixelBGRAArray* Pixels_engine_test_createArrayFromString_(const char* c_str, CoqFont const* coqFont);

// Infos sur les dimensions de la police.
typedef struct CoqFontDims {
    /// Décalage pour centrer les glyphes en y.
    float deltaY;
    /// Hauteur total requise pour dessiner les glyphes en pixels *avec* les fioritures qui depassent,
    ///  i.e. `ascender - descender + extraYmargins`.
    float fullHeight;
    /// Hauteur de la hitbox en pixels sans les fioritures.
    /// Choix : `0.5*(capheight+ascender) - descender` (on va de descender à entre capHeight et ascender).
    /// ** -> Notez : les dimensions "relatives" des glyphes le sont par rapport à `solidHeight` **,
    float solidHeight;
    bool  nearest; // La font est en mode pixélisé.
} CoqFontDims;
CoqFontDims coqfont_dims(CoqFont const* cf);

// Définie où sont les fonts et la font par défaut (avec extension)
void CoqFont_freetype_init(const char* fontsPath, const char* defaultFontFileName);
void CoqFont_freetype_quit(void);
void CoqFont_test_printAvailableFonts(void);

#endif
