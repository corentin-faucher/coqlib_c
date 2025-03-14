//
//  graph_glyphs.h
//  Texture contenant les glyphs d'une police de caractères.
//
//  Created by Corentin Faucher on 2024-09-11.
//

#ifndef graph_glyphs_h
#define graph_glyphs_h

#include "graph_texture.h"

// MARK: - Info d'un glyph (dessin d'un char)
/// Les infos pour placer les caractères les un après les autres (pour former une string).
/// La hauteur de référence est : hRef = capHeight + descender.
/// Pour le décalage en x `relGlyphX`, c'est le déplacement à faire pour positionner le glyph en x,
/// e.g. pour le 'j' qu'il faut décaler vers la gauche, on pourrait avoir `relGlyphX = -0.2`.
typedef struct GlyphInfo {
    Rectangle uvRect;        // Coordonées uv dans la texture.
    // Les dimension sont relative à la hauteur de ref `solidHeight`.
    float     relGlyphX;     // Décalage en x (en ratio à la hauteur de ref.) typiquement ~ 0.
    float     relGlyphWidth; // Largeur du glyph/dessin (en ration à la hauteur de ref)
    float     relSolidWidth; // Ratio w/h de la "solid box", i.e. vrai espace occupé en x. 
                 // (Typiquement relSolidWidth < relGlyphWidth pour une glyph qui déborde, 
                 // e.g. des lettres attachées.)
} GlyphInfo;

// MARK: - Map de glyphs (pour dessiner les strings), c'est une sous-struct de Texture...
typedef struct GlyphMap GlyphMap;
/// Infos pour init une FontGlyphMap (tout est optionel... on peut juste tout laisser à zéro)
typedef struct GlyphMapInit {
    CoqFontInit fontInit;
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
Texture*        glyphmap_texture(GlyphMap* gm);

/// Init la glyphmap utilisée par défaut.
void            GlyphMap_default_init(GlyphMapInit info);
void            GlyphMap_default_deinit(void);
GlyphMap*       GlyphMap_default(void);
bool            GlyphMap_default_isInit(void);
CoqFont*        GlyphMap_default_font(void);
Texture*        GlyphMap_default_texture(void);

#endif /* graph_glyphs_h */
