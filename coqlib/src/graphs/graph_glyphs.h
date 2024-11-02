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
typedef struct GlyphMap GlyphMap;
/// Infos pour init une FontGlyphMap.
typedef struct GlyphMapInit {
    const char* fontNameOpt;     // Si NULL, font par défaut du system.
    float       fontSizeOpt;     // Si 0 -> 32.
    size_t      textureWidthOpt; // Si 0, choix proportionnel à fontSize.
    bool        nearest;         // Style pixélisé
    // Liste de chars "custom" dessiner a partir d'une texture (optionnel)
    // (mais si count > 0 -> tout doit être défini) 
    Texture    *customChars_texOpt;   // Texture où trouver les customs chars
    Character  *customChars_charsOpt; // Liste des chars customizés
    Rectangle  *customChars_uvRectsOpt;// Liste des rectangles où trouver les chars customizés dans la texture
    size_t      customChars_count; // Nombre de chars customizés.
} GlyphMapInit;
void            glyphmap_deinit(GlyphMap* gm);
/// Une texture qui stocke les glyphs d'une police de caractères.
GlyphMap*       GlyphMap_create(GlyphMapInit info);
void            glyphmapref_releaseAndNull(GlyphMap** fgmOptRef);

GlyphInfo       glyphmap_getGlyphInfoOfChar(GlyphMap* gm, Character c);
Texture*        glyphmap_getTexture(GlyphMap* gm);
coq_Font const* glyphmap_getFont(GlyphMap const*const gm);

void            GlyphMap_setDefault(GlyphMapInit info);
GlyphMap*       GlyphMap_getDefault(void);
void            GlyphMap_unsetDefault(void);
coq_Font const* GlyphMap_getDefaultFont(void);
Texture*        GlyphMap_getDefaultTexture(void);

#endif /* graph_glyphs_h */
