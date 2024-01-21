//
//  Number.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-29.
//

#include "nodes/node_number.h"
#include "nodes/node_squirrel.h"
#include "nodes/node_tree.h"

void  drawable_setAsDigit_(Drawable* d, uint32_t digit, float x_margin) {
    drawable_setTile(d, digit, 0);
    d->x_margin = x_margin;
    drawable_updateDimsWithDeltas(d, 0.f, 1.f);
}
Drawable* _sq_goRightToDrawable(Squirrel* sq, Texture* tex) {
    while(sq->pos->littleBro) {
        sq->pos = sq->pos->littleBro;
        Drawable* d = node_asDrawableOpt(sq->pos);
        if(d) return d;
        printerror("Non drawable in Number.");
    }
    // Pas trouvé de next drawable.
    Drawable* d = Drawable_create(sq->pos, tex, mesh_sprite, 0, node_place_asBro);
//    Drawable* d = Drawable_create(NULL, tex, mesh_sprite, 0, 0);
//    node_simpleMoveToBro(&d->n, sq->pos, 0);
    sq->pos = &d->n;
    return d;
}
Drawable* _sq_goDownToDrawable(Squirrel* sq, Texture* tex) {
    if(sq->pos->firstChild) {
        sq->pos = sq->pos->firstChild;
        Drawable* d = node_asDrawableOpt(sq->pos);
        if(d) return d;
        printerror("Non drawable in Number2.");
        return _sq_goRightToDrawable(sq, tex);
    }
    // Pas de child
    Drawable* d = Drawable_create(sq->pos, tex, mesh_sprite, 0, node_place_asElderBig);
//    Drawable* d = Drawable_create(NULL, tex, mesh_sprite, 0, 0);
//    node_simpleMoveToParent(&d->n, sq->pos, true);
    sq->pos = &d->n;
    return d;
}

void  number_open_(Node* n) {
    Number* nb = (Number*)n;
    number_setTo(nb, nb->value);
}
Texture* Number_defaultTex = NULL;
static Number* number_last_ = NULL;

Number* Number_create(Node* ref, int32_t value, float x, float y,
                      float height) {
    Number* nb = Node_createEmptyOfType_(node_type_n_number, sizeof(Number), 0, ref, 0);
    nb->n.x = x;       nb->n.y = y;
    nb->n.h = 1.f;     nb->n.w = 1.f;
    nb->n.sx = height; nb->n.sy = height;
    nb->n.openOpt = number_open_;
    nb->value = value;
    if(Number_defaultTex == NULL)
        Number_defaultTex = Texture_sharedImageByName("coqlib_digits_black");
    nb->digitTex = Number_defaultTex;
    nb->separator = digit_dot;
    nb->digit_x_margin = -0.15;
    nb->separator_x_margin = -0.25;
    number_last_ = nb;
    
    return nb;
}
Number* node_asNumberOpt(Node* n) {
    if(n->_type & node_type_flag_number)
        return (Number*)n;
    return NULL;
}
void    number_last_setNow(void) {
    if(!number_last_) { printerror("No last number."); return; }
    number_setTo(number_last_, number_last_->value);
}
void    number_setTo(Number* nb, int32_t newValue) {
    // Pas de changement ?
    if((newValue == nb->value) && nb->n.firstChild)
        return;
    // 0. Init
    nb->value = newValue;
    uint32_t displayedNumber = newValue < 0 ? -newValue : newValue;
    bool isNegative = nb->value < 0;
    uint32_t maxDigits = umaxu(uint_highestDecimal(displayedNumber), nb->unitDecimal);
    Squirrel sq;
    sq_init(&sq, &nb->n, sq_scale_ones);
    Drawable* d = _sq_goDownToDrawable(&sq, nb->digitTex);
    // 1. Signe "+/-"
    if(isNegative) {
        drawable_setAsDigit_(d, digit_minus, nb->digit_x_margin);
        d = _sq_goRightToDrawable(&sq, nb->digitTex);
    } else if(nb->showPlus) {
        drawable_setAsDigit_(d, digit_plus, nb->digit_x_margin);
        d = _sq_goRightToDrawable(&sq, nb->digitTex);
    }
    // 2. Chiffres avant le "separator"
    // (ici, attention au unsigned...
    for(uint32_t u = 0; u <= maxDigits - nb->unitDecimal; u++) {
        // (Premier deja pret)
        if(u != 0) d = _sq_goRightToDrawable(&sq, nb->digitTex);
        // i.e. decimal = maxDigit...unitDecimal.
        uint32_t digit = (nb->initAsBlank) ? digit_underscore : uint_digitAt(displayedNumber, maxDigits - u);
        drawable_setAsDigit_(d, digit, nb->digit_x_margin);
    }
    // 3. Separator et chiffres restants
    if(nb->unitDecimal > 0) {
        d = _sq_goRightToDrawable(&sq, nb->digitTex);
        drawable_setAsDigit_(d, nb->separator, nb->separator_x_margin);
        for(uint32_t u = 0; u < nb->unitDecimal; u++) {
            d = _sq_goRightToDrawable(&sq, nb->digitTex);
            // ici `decimal = nb->unitDecimal - 1 - u`.
            uint32_t digit = (nb->initAsBlank) ? digit_underscore :
                uint_digitAt(displayedNumber, nb->unitDecimal - 1 - u);
            drawable_setAsDigit_(d, digit, nb->digit_x_margin);
        }
    }
    // 4. Extra/"unit" digit
    if(nb->extraDigitOpt) {
        d = _sq_goRightToDrawable(&sq, nb->digitTex);
        drawable_setAsDigit_(d, nb->extraDigitOpt, nb->digit_x_margin);
    }
    // 5. Nettoyage de la queue.
    while(sq.pos->littleBro)
        node_tree_throwToGarbage(sq.pos->littleBro);
    // 6. Alignement
    node_tree_alignTheChildren(&nb->n, 0, 1.f, 1.f);
    // 7. Vérifier s'il faut afficher... (live update)}
    if(nb->n.flags & flag_show)
        node_tree_addFlags(&nb->n, flag_show);
    return;
}

/*
void    number_showDigit(Number* nb, uint32_t decimal) {
    uint32_t displayedNumber = nb->value < 0 ? -nb->value : nb->value;
    bool isNegative = nb->value < 0;
    uint32_t maxDigits = umaxu(uint_highestDecimal(displayedNumber), nb->unitDecimal);
    if(decimal > maxDigits) { printwarning("Decimal overflow %d, maxDigit %d.", decimal, maxDigits); return; }
    Squirrel sq;
    sq_init(&sq, &nb->n, sq_scale_ones);
    if(!sq_goDown(&sq)) { printwarning("Number not init."); return; }
    // 1. Signe "+/-"
    if(isNegative || nb->showPlus) sq_goRight(&sq);
    // 2. Chiffres avant le "separator"
    // (ici, attention au unsigned...
    for(uint32_t u = 0; u <= maxDigits - nb->unitDecimal; u++) {
        // (Premier deja pret)
        if(u != 0) sq_goRight(&sq);
        if(maxDigits - u == decimal) {
            Drawable* d = node_asDrawableOpt(sq.pos);
            if(!d) { printerror("Not a drawable."); return; }
            drawable_setAsDigit_(d, uint_digitAt(displayedNumber, decimal), nb->digit_x_margin);
            return;
        }
    }
    // 3. Separator et chiffres restants
    sq_goRight(&sq); // sep
    for(uint32_t u = 0; u < nb->unitDecimal; u++) {
        sq_goRight(&sq);
        if(nb->unitDecimal - 1 - u == decimal) {
            Drawable* d = node_asDrawableOpt(sq.pos);
            if(!d) { printerror("Not a drawable."); return; }
            drawable_setAsDigit_(d, uint_digitAt(displayedNumber, decimal), nb->digit_x_margin);
            return;
        }
    }
    printerror("No decimal found?");
}
Drawable*  number_getDigitDrawableOpt(Number* nb, uint32_t decimal) {
    uint32_t displayedNumber = nb->value < 0 ? -nb->value : nb->value;
    bool isNegative = nb->value < 0;
    uint32_t maxDigits = umaxu(uint_highestDecimal(displayedNumber), nb->unitDecimal);
    if(decimal > maxDigits) { return NULL; }
    Squirrel sq;
    sq_init(&sq, &nb->n, sq_scale_ones);
    if(!sq_goDown(&sq)) { printwarning("Number not init."); return NULL; }
    // 1. Signe "+/-"
    if(isNegative || nb->showPlus) sq_goRight(&sq);
    // 2. Chiffres avant le "separator"
    // (ici, attention au unsigned...
    for(uint32_t u = 0; u <= maxDigits - nb->unitDecimal; u++) {
        // (Premier deja pret)
        if(u != 0) sq_goRight(&sq);
        if(maxDigits - u == decimal) {
            Drawable* d = node_asDrawableOpt(sq.pos);
            if(!d) { printerror("Not a drawable."); return NULL; }
            return d;
        }
    }
    // 3. Separator et chiffres restants
    sq_goRight(&sq); // sep
    for(uint32_t u = 0; u < nb->unitDecimal; u++) {
        sq_goRight(&sq);
        if(nb->unitDecimal - 1 - u == decimal) {
            Drawable* d = node_asDrawableOpt(sq.pos);
            if(!d) { printerror("Not a drawable."); return NULL; }
            return d;
        }
    }
    printerror("No decimal found?");
    return NULL;
}

void    number_last_setTo(int32_t newValue) {
    if(!number_last_) { printerror("No last number."); return; }
    number_setTo(number_last_, newValue);
}
void    number_last_setDigitTex(Texture* digitTex) {
    if(!number_last_) { printerror("No last number."); return; }
    number_last_->digitTex = digitTex;
}
*/
