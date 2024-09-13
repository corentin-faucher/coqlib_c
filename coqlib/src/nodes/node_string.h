//
//  node_string.h
//  Un drawable multi avec les glyph des char pour créer une string.
//
//  Created by Corentin Faucher on 17/6/24.
//

#ifndef coq_node_string_h
#define coq_node_string_h

#include "node_drawable_multi.h"
#include "node_sliding_menu.h"
#include "../graphs/graph_glyphs.h"

#pragma mark - Drawable d'un seul caractère. (pour test, utiliser plutôt NodeString) -
typedef struct DrawableChar {
    union {
        Node     n;
        Drawable d;
    };
    // (Pour testing... Pas toutes les infos d'un glyph sont vraiment utile...)
    // Décalage de la position glyph vs sprite.
    float     glyphX;
    float     glyphY;
    // Espace occupé par la sprite du glyph.
    float     glyphWidth; // Largeur du glyph/dessin (en ratio à la hauteur de ref)
    float     glyphHeight;
    // Hitbox du char (sans les fioritureg qui dépassent)
    float     solidWidth;
    float     solidHeight;
} DrawableChar;
DrawableChar*   DrawableChar_create(Node* refOpt, Character c,
                               float x, float y, float twoDy, flag_t flags);

#pragma mark - StringGlyphed (String avec info pour dessiner)
// Étapes pour dessiner une string.
// 1. On a une c_str (char[] en utf8) que l'on convertie en CharacterArray:
//     c_str -> charArray.
// 2. Obtenir les dimensions des chars pour savoir l'espace occupé.
//     charArray -> stringGlyphed.
// 3. (Optionnel) On peut scinder la stringGlyphed en plusieurs lignes avec une largeur de ligne définie.
//     stringGlyphed -> linesArray (array de StringGlyphed).
// 4. Création d'un NodeString (un DrawableMulti où chaque instance est le glyph d'un char). 
//     stringGlyphed -> NodeString
/// Les infos pour dessiner une string (sur une ligne)
typedef struct StringGlyphed StringGlyphed;
StringGlyphed* StringGlyphed_create(StringGlyphedInit initData);

/// Setter la Glyph map a utiliser par défaut pour les StringGlyphed.
/// L'ownership est transféré (fonction avec `give...`).
void           StringGlyphed_giveDefaultFontGlyphMap(FontGlyphMap** defaultFontGlyphMapGivenRef);
void           StringGlyphed_deinit(void);
Texture*       StringGlyphed_defaultGlyphMapTexture(void);

#pragma mark - Noeud drawable d'une string.
typedef struct NodeString {
    union {
        Node          n;
        Drawable      d;
        DrawableMulti dm;
    };
    StringGlyphed*   _strGlyph; // String avec infos pour dessiner.
    /// Un chrono qui commence à l'ouverture (pour animation de la string)
    Chrono           chrono;
} NodeString;

NodeString* NodeString_create(Node* ref, StringGlyphedInit data,
                      float x, float y, float widthOpt, float height,
                      flag_t flags, uint8_t node_place);

/// Espace occupé par la string (relativement à height).
/// `o_y` est le shift relatif en y. `o_x` est la marge en x (x_margin).
/// h est la hauteur relative : gm->glyphHeight / gm->solidHeight.
/// w inclus le spacing et les marges (+2*x_margin).
Rectangle         nodestring_getRelativeBox(NodeString* ns);
/// Espace relatif entre les caractères.
float             nodestring_getSpacing(NodeString* ns);
/// Accès à la liste de caractère avec dimensions de glyph
CharacterGlyphed* nodestring_firstCharacterGlyphed(NodeString* ns);
/// Fonction utile pour avoir un planché sur la largeur d'un `GlyphInfo`.
float SG_charWidth_(float width, float spacing);

#pragma mark - StringWithGlyphMulti, String scindé en plusieurs lignes.
typedef struct StringGlyphArr StringGlyphArr;
/// Fonction pour scinder un StringWithGlyph en plusieurs lignes.
StringGlyphArr* StringGlyphArr_create(const StringGlyphed* str, float lineWidth,
                                      Character trailingSpace);
void            stringglypharr_deinit(StringGlyphArr* strArr);


#pragma mark - Multi String
/// Ajoute à un noeud la string scinder en plusieurs strings de largeur maximale lineWidth.
void  node_addMultiStrings(Node* n, StringGlyphedInit const data,
                           float const lineWidth, float const lineHeight,
                           uint32_t relativeFlags, Character trailingSpace);
                        

#pragma mark - Multi String scrollable
void slidingmenu_addMultiStrings(SlidingMenu* sm, StringGlyphedInit data);

//typedef struct NodeStringSliding NodeStringSliding;
//
//NodeStringSliding* NodeStringSliding_create(Node* ref, StringDrawableInitData data,
//                        float x, float y, float lineWidth, float lineHeight, uint32_t displayedLinesCount,
//                        flag_t flags, uint8_t node_place);
//                        
//void nodestringsliding_next(NodeStringSliding* nss);
                                            
#endif /* node_string_h */
