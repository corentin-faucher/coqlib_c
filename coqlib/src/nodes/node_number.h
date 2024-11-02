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

typedef enum _Digit {
    digit_zero,
    digit_one,
    digit_two,
    digit_three,
    digit_four,
    digit_five,
    digit_six,
    digit_seven,
    digit_eight,
    digit_nine,
    digit_space,
    digit_unused1,
    digit_underscore,
    digit_plus,
    digit_minus,
    digit_mult,
    digit_div,
    digit_dot,
    digit_comma,
    digit_second,
    digit_percent,
    digit_equal,
    digit_question,
    digit_unused2,
} Digit;

typedef struct Number {
    union {
        Node          n;
        Drawable      d;
        DrawableMulti dm;
    };
    int32_t  value;
    uint32_t unitDecimal;
    uint32_t separator;
    uint32_t extraDigitOpt;
    float    digit_x_margin;
    float    separator_x_margin;
    float    extra_x_margin;
    bool     showPlus;
    bool     initAsBlank;
    float    _xs[NUMBER_MAX_DIGITS_];
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
