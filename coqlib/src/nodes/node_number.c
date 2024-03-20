//
//  Number.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-29.
//

#include "nodes/node_number.h"
#include "nodes/node_squirrel.h"
#include "nodes/node_tree.h"
#include "utils/utils_base.h"

void sq_goToDrawable_(Squirrel* sq, Node* const ref, Texture* tex, float x_margin, uint32_t digit) {
    Drawable* d = NULL;
    if(sq->pos == ref) {
        Node* child = sq->pos->_firstChild;
        if(!child) { // Creation premier child.
            d = Drawable_createImageGeneral(ref, tex, mesh_sprite, 0, 0, 0, 1, x_margin, 0, 0);
            drawable_setTile(d, digit, 0);
            sq->pos = &d->n;
            return;
        }
        sq->pos = child;
        d = node_asDrawableOpt(child);
        if(!d) {printerror("Non drawable in number."); }
    } else {
        do {
            Node* littlebro = sq->pos->_littleBro;
            if(!littlebro) break;
            sq->pos = littlebro;
            d = node_asDrawableOpt(littlebro);
            if(d) break;
            printerror("Non drawable in number.");
        } while(true);
    }
    if(!d) { // Pas trouver de drawable, le créer et se placer dessus.
        d = Drawable_createImageGeneral(sq->pos, tex, mesh_sprite, 0, 0, 0, 1, x_margin, 0, node_place_asBro);
        drawable_setTile(d, digit, 0);
        sq->pos = &d->n;
        return;
    }
    // Si trouver, mettre à jour.
    d->x_margin = x_margin;
    drawable_updateDims_(d);
    drawable_setTile(d, digit, 0);
}

void  number_open_(Node* n) {
    Number* nb = (Number*)n;
    number_setTo(nb, nb->value);
}
Texture* Number_defaultTex = NULL;
static Number* number_last_ = NULL;

Number* Number_create(Node* ref, int32_t value, float x, float y,
                      float height) {
    Number* nb = coq_calloc(1, sizeof(Number));
    node_init_(&nb->n, ref, x, y, 1, 1, node_type_n_number, 0, 0);
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
void     number_last_setDigitTexture(Texture* digitTexture) {
    if(!number_last_) { printerror("No last number."); return; }
    number_last_->digitTex = digitTexture;
}
void   number_last_setExtraDigit(uint32_t extraDigit) {
    if(!number_last_) { printerror("No last number."); return; }
    number_last_->extraDigitOpt = extraDigit;
}
void     number_last_setSeparator(uint32_t separatorDigit) {
    if(!number_last_) { printerror("No last number."); return; }
    number_last_->separator = separatorDigit;
}
void     number_last_setunitDecimal(uint32_t unitDecimal) {
    if(!number_last_) { printerror("No last number."); return; }
    number_last_->unitDecimal = unitDecimal;
}
void   number_last_setNow(void) {
    if(!number_last_) { printerror("No last number."); return; }
    number_setTo(number_last_, number_last_->value);
}
void    number_setTo(Number* nb, int32_t newValue) {
    // Pas de changement ?
    if((newValue == nb->value) && nb->n._firstChild)
        return;
    // 0. Init
    nb->value = newValue;
    uint32_t displayedNumber = newValue < 0 ? -newValue : newValue;
    bool isNegative = nb->value < 0;
    uint32_t maxDigits = umaxu(uint_highestDecimal(displayedNumber), nb->unitDecimal);
    Node* const ref = &nb->n;
    Texture* tex = nb->digitTex;
    Squirrel sq;
    sq_init(&sq, ref, sq_scale_ones);
    // 1. Signe "+/-"
    if(isNegative) {
        sq_goToDrawable_(&sq, ref, tex, nb->digit_x_margin, digit_minus);
    } else if(nb->showPlus) {
        sq_goToDrawable_(&sq, ref, tex, nb->digit_x_margin, digit_plus);
    }
    // 2. Chiffres avant le "separator"
    // (ici, attention au unsigned...
    for(uint32_t u = 0; u <= maxDigits - nb->unitDecimal; u++) {
        uint32_t digit = (nb->initAsBlank) ? digit_underscore : uint_digitAt(displayedNumber, maxDigits - u);
        sq_goToDrawable_(&sq, ref, tex, nb->digit_x_margin, digit);
    }
    // 3. Separator et chiffres restants
    if(nb->unitDecimal > 0) {
        sq_goToDrawable_(&sq, ref, tex, nb->separator_x_margin, nb->separator);
        for(uint32_t u = 0; u < nb->unitDecimal; u++) {
            // ici `decimal = nb->unitDecimal - 1 - u`.
            uint32_t digit = (nb->initAsBlank) ? digit_underscore :
                             uint_digitAt(displayedNumber, nb->unitDecimal - 1 - u);
            sq_goToDrawable_(&sq, ref, tex, nb->digit_x_margin, digit);
        }
    }
    // 4. Extra/"unit" digit
    if(nb->extraDigitOpt)
        sq_goToDrawable_(&sq, ref, tex, nb->digit_x_margin, nb->extraDigitOpt);
    // 5. Nettoyage de la queue.
    while(sq.pos->_littleBro)
        node_tree_throwToGarbage(sq.pos->_littleBro);
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
     Node* child = sq->pos->firstChild;
     if(child) {
         sq->pos = child;
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
*/
