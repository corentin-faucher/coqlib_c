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

// MARK: - Drawable d'un seul caractère avec toutes les infos -
// (pour test, utiliser plutôt NodeString ou Drawable_createCharacter)
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
                               
// MARK: - Drawable simple (sans extra) d'un caractère ou string -
Drawable*  Drawable_createCharacter(Node* refOpt, Character c, GlyphMap* glyphMapOpt,
                               float x, float y, float twoDy, flag_t flags);
void       drawable_changeToCharacter(Drawable *d, Character c, GlyphMap *glyphMapOpt);
/// *test* Drawable de string avec sa propre texture (pixels array)
/// ( -> Utiliser de préférence NodeString) 
Drawable*  Drawable_test_createString(Node* refOpt, const char *c_str, CoqFont const* cf,
                                float x, float y, float twoDy, flag_t flags);
                               
// MARK: - Un caractère avec ses dimensions (Élément de StringGlyphed)
/// Info du glyph d'un char à garder en mémoire pour l'affichage (utile pour overrider `updateModel`)
typedef struct CharacterGlyphed {
    Character c;
    GlyphInfo info;
    bool      firstOfWord;
} CharacterGlyphed;

// MARK: - StringGlyphed (String avec info pour dessiner)
// Étapes pour dessiner une string.
// 1. On a une c_str (char[] en utf8) que l'on convertie en CharacterArray:
//     c_str -> charArray.
// 2. De l'array de chars, obtenir des chars avec des dimensions et associé à une FontGlyphMap.
//     charArray -> stringGlyphed.
// 3. (Optionnel) On peut scinder la stringGlyphed en plusieurs lignes avec une largeur de ligne définie.
//     stringGlyphed -> linesArray (array de StringGlyphed).
// 4. Création d'un NodeString (un DrawableMulti où chaque instance est le glyph d'un char). 
//     stringGlyphed -> NodeString
/// Les infos pour dessiner une string (sur une ligne)
typedef struct StringGlyphed StringGlyphed;
StringGlyphed* StringGlyphed_createEmpty(size_t maxCount, GlyphMap *glyphMapOpt, 
                                        float spacing, float x_margin);
StringGlyphed* StringGlyphed_create(const CharacterArray *ca, GlyphMap *glyphMapOpt,
                                    float spacing, float x_margin);
// (pas de stringglyphed_denit, on peut dealloc directement)
void stringglyphed_setChars(StringGlyphed* sg, const CharacterArray *ca);


// MARK: - NodeStringInit : info à fournir pour créer une string avec glyphs.
/// Structure temporaire pous initialiser un NodeString.
typedef struct NodeStringInit {
    /// La string à afficher (shared/unowned)
    const char*   c_str;
    /// Il faut chercher la version localisé avec `String_createLocalized`...
    bool          isLocalized;
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
} NodeStringInit;

// MARK: - NodeString : Noeud drawable affichant une string.
// La String affichée est constante. (Ne serait pas thread safe si editable...)
typedef struct NodeString {
    union {
        Node          n;
        Drawable      d;
        DrawableMulti dm;
    };
    StringGlyphed *const sg; // String avec infos pour dessiner.
    // Instant `ChronoRender` à l'ouverture.
    float const       openTimeSec;
    float const       twoDxOpt;
} NodeString;
NodeString* NodeString_create(Node* ref, NodeStringInit data,
                      float x, float y, float widthOpt, float height,
                      flag_t flags, uint8_t node_place);

NodeString* node_asNodeStringOpt(Node* nOpt);
void        nodestring_updateString(NodeString* ns, const char* newString);
                      
/// Custom updateModel pour NodeString : fait bouger les lettres.
/// n.float0 est le delai d'apparition des lettres.
/// n.float1 est la frequence d'oscillations (rotation) des lettres.
void nodestring_renderer_updateIUsMoving(Node* const n);


// MARK: - StringWithGlyphMulti, String scindé en plusieurs lignes.
typedef struct StringGlyphArr StringGlyphArr;
/// Fonction pour scinder un StringWithGlyph en plusieurs lignes.
StringGlyphArr* StringGlyphArr_create(const StringGlyphed* str, float lineWidth,
                                      Character trailingSpace);
void            stringglypharr_deinit(StringGlyphArr* strArr);


// MARK: - Multi String
/// Ajoute à un noeud la string scinder en plusieurs strings de largeur maximale lineWidth.
void  node_addMultiStrings(Node* n, NodeStringInit const data,
                           float const lineWidth, float const lineHeight,
                           uint32_t relativeFlags, Character trailingSpace);
                        

// MARK: - Multi String scrollable
void slidingmenu_addMultiStrings(SlidingMenu* sm, NodeStringInit data);

//typedef struct NodeStringSliding NodeStringSliding;
//
//NodeStringSliding* NodeStringSliding_create(Node* ref, StringDrawableInitData data,
//                        float x, float y, float lineWidth, float lineHeight, uint32_t displayedLinesCount,
//                        flag_t flags, uint8_t node_place);
//                        
//void nodestringsliding_next(NodeStringSliding* nss);
                                            
#endif /* node_string_h */
