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
typedef struct DrawableChar {
    union {
        Node     n;
        Drawable d;
    };
    GlyphInfo glyph;
} DrawableChar;
DrawableChar*   DrawableChar_create(Node* refOpt, Character c,
                               float x, float y, float twoDy, flag_t flags);
// Espace occupé par le glyph (complet avec fioritures) dans le référentiel du parent.
Box             drawablechar_getGlyphBox(DrawableChar const* dc);
void            drawablechar_updateToChar(DrawableChar* dc, Character newChar);
void            drawablechar_renderer_updateIU_(Node* n);

// MARK: - NodeString : Noeud drawable affichant une string.
// La String affichée est constante. (Ne serait pas thread safe si editable...)
typedef struct NodeString {
    union {
        Node          n;
        Drawable      d;
        DrawableMulti dm;
    };
    StringGlyphed *const sg; // String avec infos pour dessiner.
//    GlyphSimple*      gsArr;
//    size_t            gsArrCount;
//    size_t            gsArrMaxCount;
    // Instant `ChronoRender` à l'ouverture.
    float const       openTimeSec;
    float const       twoDxOpt;
} NodeString;
NodeString* NodeString_create(Node* ref, StringGlyphedInit data,
                      float x, float y, float widthOpt, float height,
                      flag_t flags, uint8_t node_place);
void        nodestring_updateString(NodeString* ns, const char* newString);
static inline NodeString* node_asNodeStringOpt(Node* nOpt) {
    return (nOpt && (nOpt->_type & node_type_string)) ? (NodeString*)nOpt : NULL;
}
                      
/// Exemple de custom updateIU pour NodeString : fait bouger les lettres.
/// n.float0 est le delai d'apparition des lettres.
/// n.float1 est la frequence d'oscillations (rotation) des lettres.
void nodestring_renderer_updateIUsMoving(Node* const n);


// MARK: - Multi String
/// Ajoute à un noeud la string scinder en plusieurs strings de largeur maximale lineWidth.
void  node_addMultiStrings(Node* n, StringGlyphedInit const data,
                           float const lineWidth, float const lineHeight,
                           uint32_t relativeFlags, Character trailingSpace);
                        

// MARK: - Multi String scrollable
void slidingmenu_addMultiStrings(SlidingMenu* sm, StringGlyphedInit data);

/// *test* Drawable de string avec sa propre texture (pixels array)
/// ( -> Utiliser de préférence NodeString) 
Drawable*  Drawable_test_createString(Node* refOpt, const char *c_str, CoqFont const* cf,
                                float x, float y, float twoDy, flag_t flags);

                                            
#endif /* node_string_h */
