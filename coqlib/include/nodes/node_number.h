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
#include "maths/math_flpos.h"

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

typedef struct _Number {
    Node     n;
    
    int32_t  value;
    Texture* digitTex;
    uint32_t unitDecimal;
    uint32_t separator;
    uint32_t extraDigitOpt;
    float    digit_x_margin;
    float    separator_x_margin;
    // Faire un uint de state flags?
    bool     showPlus;
    bool     initAsBlank;
} Number;


extern Texture* Number_defaultTex;
/// Noeud avec des drawable de digit comme descendants.
/// La structure (Number->{ 1, 2 }) est créée lors du `open` (call to `number_setTo`).
/// On peut donc customizer les parametres avant que la structure soit créée.
Number*  Number_create(Node* ref, int32_t value,
                      float x, float y, float height);
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

// superflu ?
//void     number_showDigit(Number* nb, uint32_t decimal);
//void     number_last_setTo(int32_t newValue);
//void     number_last_setDigitTex(Texture* digitTex);


// Utile ? Nombre qui change de facon fluide. (pas d'un seul coup)
//typedef struct _NumberFluid {
//    union {
//        Node   n;
//        Number nb;
//    };
//    FluidPos fl_value;
//    Timer*   timer;
//} NumberFluid;

#endif /* Number_h */
