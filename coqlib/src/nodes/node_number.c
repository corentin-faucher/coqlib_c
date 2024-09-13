//
//  Number.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-29.
//

#include "node_number.h"

#include "node_squirrel.h"
#include "node_tree.h"
#include "../utils/util_base.h"


Texture* Number_defaultTex = NULL;

#pragma mark - Number 2 -----------------------------

static Number* number_last_ = NULL;
void      number_open_(Node* n) {
    Number* nb = (Number*)n;
    number_setTo(nb, nb->value);
}
void number_updateModels_(Node* const n) {
    Number* nb = (Number*)n;
    float const show = smtrans_setAndGetValue(&nb->d.trShow, (n->flags & flag_show) != 0);
    if(show < 0.001f) {
        n->flags &= ~flag_drawableActive;
        return;
    }
    n->flags |= flag_drawableActive;
    const Matrix4* const pm = node_parentModel(n);
    
    float const pop = (nb->n.flags & flag_poping) ? show : 1.f;
    float const pos_y = nb->n.y;
    float const pos_z = nb->n.z;
    Vector2 const scales = nb->n.scales;
    const float* x = nb->_xs;
    InstanceUniforms* const end = &nb->dm.iusBuffer.ius[nb->dm.iusBuffer.actual_count];
    for(InstanceUniforms* iu = nb->dm.iusBuffer.ius; iu < end; iu++, x++) {
        iu->show = show;
        float const pos_x = nb->n.x + (*x) * scales.x;
        Matrix4* m = &iu->model;
        // Petite translation sur la parent-matrix en fonction du digit.
        m->v0.v = pm->v0.v * scales.x * pop;
        m->v1.v = pm->v1.v * scales.y * pop;
        m->v2 =   pm->v2;
        m->v3 = (Vector4) {{
            pm->v3.x + pm->v0.x * pos_x + pm->v1.x * pos_y + pm->v2.x * pos_z,
            pm->v3.y + pm->v0.y * pos_x + pm->v1.y * pos_y + pm->v2.y * pos_z,
            pm->v3.z + pm->v0.z * pos_x + pm->v1.z * pos_y + pm->v2.z * pos_z,
            pm->v3.w,
        }};
    }
}
void (*Number_defaultUpdateModel)(Node*) = number_updateModels_; 
Number*   Number_create(Node* ref, int32_t value,
                      float x, float y, float height,
                      flag_t flags, uint8_t node_place) {
    Number* nb = coq_callocTyped(Number);
    node_init(&nb->n, ref, x, y, 1, 1, node_type_ndm_number, flags, node_place);
    nb->n.sx = height; nb->n.sy = height;
    if(Number_defaultTex == NULL)
        Number_defaultTex = Texture_sharedImageByName("coqlib_digits_black");
    drawable_init(&nb->d, Number_defaultTex, mesh_sprite, 0, 1);
    drawablemulti_init_(&nb->dm, NUMBER_MAX_DIGITS_);
    InstanceUniforms iu = InstanceUnifoms_default;
    iu.uvRect.size = texture_tileDuDv(Number_defaultTex);
    piusbuffer_setAllTo(&nb->dm.iusBuffer, iu);
    nb->dm.iusBuffer.actual_count = 0;
    // Init as number
    nb->n.openOpt =     number_open_;
    nb->n.updateModel = Number_defaultUpdateModel;
    nb->value = value;
    nb->separator = digit_dot;
    nb->digit_x_margin =     -0.25;
    nb->separator_x_margin = -0.60;
    nb->extra_x_margin =      0.25;
    nb->n.sx = height * texture_tileRatio(nb->d._tex);
    nb->n.sy = height;
    number_last_ = nb;
    
    return nb;
}
Number*   node_asNumberOpt(Node* n) {
    if(n->_type & node_type_flag_number)
        return (Number*)n;
    return NULL;
}
typedef struct NumberIt_ {
    float*                  xit;
    float                   x1;
    InstanceUniforms*       iu;
    InstanceUniforms* const end;
    uint32_t const          m;
    uint32_t                i;
    Vector2 const           Duv;
} NumberIt_; 
void  numberit_setAndNext_(NumberIt_* nbit, uint32_t digit, float deltaX) {
    if(nbit->iu >= nbit->end) {
        printerror("Number overflow, cannot add digit %d.", digit);
        return;
    }
    nbit->x1 += deltaX;
    *nbit->xit = nbit->x1;
    nbit->iu->uvRect.o_x = (digit % nbit->m) * nbit->Duv.w;
    nbit->iu->uvRect.o_y = (digit / nbit->m) * nbit->Duv.h;
    nbit->iu++; nbit->xit++; nbit->i++;
    nbit->x1 += deltaX;
}
void    number_setTo(Number* const nb, int32_t const newValue) {
    // Pas de changement ?
    if((newValue == nb->value) && nb->dm.iusBuffer.actual_count)
        return;
    // 0. Init
    nb->value = newValue;
    uint32_t const displayedNumber = newValue < 0 ? -newValue : newValue;
    bool const isNegative = nb->value < 0;
    uint32_t const maxDigits = umaxu(uint_highestDecimal(displayedNumber), nb->unitDecimal);
    float const deltaX_def = 0.5*(1.f + nb->digit_x_margin);
    NumberIt_ it = {
        nb->_xs, 0, 
        nb->dm.iusBuffer.ius, &nb->dm.iusBuffer.ius[NUMBER_MAX_DIGITS_], 
        nb->d._tex->m, 0,
        texture_tileDuDv(nb->d._tex),
    };
    // 1. Signe "+/-"
    if(isNegative) {
        numberit_setAndNext_(&it, digit_minus, deltaX_def);
    } else if(nb->showPlus) {
        numberit_setAndNext_(&it, digit_plus, deltaX_def);
    }
    // 2. Chiffres avant le "separator"
    // (ici, attention au unsigned...)
    for(uint32_t u = 0; u <= maxDigits - nb->unitDecimal; u++) {
        uint32_t digit = (nb->initAsBlank) ? digit_underscore : uint_digitAt(displayedNumber, maxDigits - u);
        numberit_setAndNext_(&it, digit, deltaX_def);
    }
    // 3. Separator et chiffres restants
    if(nb->unitDecimal > 0) {
        float const deltaX_sep = 0.5*(1.f + nb->separator_x_margin);
        numberit_setAndNext_(&it, nb->separator, deltaX_sep);
        for(uint32_t u = 0; u < nb->unitDecimal; u++) {
            // ici `decimal = nb->unitDecimal - 1 - u`.
            uint32_t digit = (nb->initAsBlank) ? digit_underscore :
                             uint_digitAt(displayedNumber, nb->unitDecimal - 1 - u);
            numberit_setAndNext_(&it, digit, deltaX_def);
        }
    }
    // 4. Extra/"unit" digit
    if(nb->extraDigitOpt) {
        float const deltaX_ext = 0.5*(1.f + nb->extra_x_margin);
        numberit_setAndNext_(&it, nb->extraDigitOpt, deltaX_ext);
    }
    // 5. Finaliser...
    nb->dm.iusBuffer.actual_count = it.i;
    float deltaX = 0.5f*it.x1;
    nb->n.w = 2.f*deltaX;
    for(float *x = nb->_xs; x < it.xit; x++)
        *x -= deltaX;  // (centrer)
}

void     number_last_setDigitTexture(Texture* const digitTexture) {
    if(!number_last_) { printerror("No last number."); return; }
    number_last_->d._tex = digitTexture;
    number_last_->n.sx = number_last_->n.sy * texture_tileRatio(digitTexture);
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
