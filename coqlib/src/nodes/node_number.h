//
//  Number.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-29.
//

#ifndef _coq_node_number_h
#define _coq_node_number_h

#include "node_base.h"
#include "node_drawable.h"
#include "node_drawable_multi.h"
#include "../maths/math_flpos.h"

#define NUMBER_MAX_DIGITS_ 14

typedef struct Number {
    union {
        Node          n;
        Drawable      d;
        DrawableMulti dm;
    };
    /// Le nombre affiché
    int32_t  value;
    ///  Où placer le point des décimal. e.g. si value = 128 et unitDecimal = 2 => 1.28.
    uint32_t unitDecimal;
    /// `digit` utilisé pour séparer units et décimales.
    uint32_t separator;
    /// `digit` ajouté à la fin, e.g. `digit_percent`.
    uint32_t extraDigitOpt;
    /// Espacement entre digits.
    float    digit_x_margin;
    /// Espacement avec séparateur.
    float    separator_x_margin;
    /// Espacement du `digit` ajouté à la fin.
    float    extra_x_margin;
    /// Afficher le `+` pour un nombre positif.
    bool     showPlus;
    /// Laissé l'espace "blanc".
    bool     initAsBlank;
    uint32_t _digitCount;
    Vector3  _U0V0Xs[NUMBER_MAX_DIGITS_];
} Number;

extern Texture*    Number_defaultTex;
extern void      (*Number_renderer_defaultUpdateIUs)(Node*); 

/// Init node as number (avec supers: Node, Drawable, DrawableMulti)
void number_and_super_init_(Number* const nb, Node* refOpt, int32_t value,
                      float x, float y, float height,
                      flag_t flags, uint8_t node_place);
/// Convenience constructor: alloc + init.
Number*  Number_create(Node* ref, int32_t value,
                      float x, float y, float height,
                      flag_t flags, uint8_t node_place);
/// Downcasting Node to Number.
Number*  node_asNumberOpt(Node* n);
/// Créer tout de suite la structure des chiffres pour avoir les vrais dimensions (width depend des chiffres)
void     number_setTo(Number* nb, int32_t newValue);
void     number_last_setDigitTexture(Texture* digitTexture);
void     number_last_setExtraDigit(uint32_t extraDigit);
void     number_last_setSeparator(uint32_t separatorDigit);
void     number_last_setunitDecimal(uint32_t unitDecimal);
/// Créer tout de suite la structure des chiffres pour avoir les vrais dimensions (width depend des chiffres)
void     number_last_setNow(void);

#endif /* Number_h */
