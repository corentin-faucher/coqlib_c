//
//  Number.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-29.
//

#include "node_number.h"

#include "../utils/util_base.h"

Texture* Number_defaultTex = NULL;

// MARK: - Number 2 -----------------------------
static Number* number_last_ = NULL;
void      number_open_(Node* n) {
    Number* nb = (Number*)n;
    number_setTo(nb, nb->value);
}
void number_renderer_updateIUs_(Node* const n) {
    Number* nb = (Number*)n;
    float const show = drawable_updateShow(&nb->d);
    if(!show) return;
    const Matrix4* const pm = node_parentModel(n);

    float const pop = (nb->n.flags & flag_drawablePoping) ? show : 1.f;
    float const pos_y = nb->n.y;
    float const pos_z = nb->n.z;
    Vector3 const scales = nb->n.scales;
    Vector3 const*      uvx =      nb->_U0V0Xs;
    Vector3 const*const uvx_end = &nb->_U0V0Xs[nb->_digitCount];
    IUsToEdit iusEdit = iusbuffer_rendering_retainIUsToEdit(nb->dm.iusBuffer);
    for(; uvx < uvx_end; iusEdit.iu++, uvx++) {
        iusEdit.iu->show = show;
        iusEdit.iu->uvRect.origin = uvx->xy;
        float const pos_x = nb->n.x + (uvx->z)*scales.x;
        Matrix4* m = &iusEdit.iu->model;
        // Petite translation sur la parent-matrix en fonction du digit.
        m->v0.v = pm->v0.v * scales.x * pop;
        m->v1.v = pm->v1.v * scales.y * pop;
        m->v2.v = pm->v2.v * scales.z * pop;
        m->v3.v = pm->v3.v + pm->v0.v * pos_x + pm->v1.v * pos_y + pm->v2.v * pos_z;
    }
    iustoedit_release(iusEdit);
}
void (*Number_renderer_defaultUpdateIUs)(Node*) = number_renderer_updateIUs_;
void number_and_super_init_(Number* const nb, Node* refOpt, int32_t value,
                      float x, float y, float height,
                      flag_t flags, uint8_t node_place) 
{
    if(Number_defaultTex == NULL)
        Number_defaultTex = Texture_sharedImageByName("coqlib_digits_black");
    node_init(&nb->n, refOpt, x, y, 1, 1, flags, node_place);
    drawable_init(&nb->d, Number_defaultTex, Mesh_drawable_sprite, 0, height);
    drawablemulti_init(&nb->dm, NUMBER_MAX_DIGITS_, &nb->n.renderIU);
    // Init as Number...
    nb->n._type |= node_type_number;
    nb->n.openOpt = number_open_;
    nb->n.renderer_updateInstanceUniforms = Number_renderer_defaultUpdateIUs;
    nb->value = value;
    nb->separator = digit_dot;
    nb->digit_x_margin =     -0.25;
    nb->separator_x_margin = -0.60;
    nb->extra_x_margin =      0.25;
    number_last_ = nb;
}
Number*   Number_create(Node* ref, int32_t value,
                      float x, float y, float height,
                      flag_t flags, uint8_t node_place) {
    Number* nb = coq_callocTyped(Number);
    number_and_super_init_(nb, ref, value, x, y, height, flags, node_place);

    return nb;
}
Number*   node_asNumberOpt(Node* n) {
    if(n->_type & node_type_number)
        return (Number*)n;
    return NULL;
}
typedef struct NumberIt_ {
    Vector3         *uvx;
    Vector3  *const  uvx_end;
    float            posX;
    uint32_t         digitCount;
    // Info de la textur pour set u0,v0.
    uint32_t const   tex_m;
    float            Du, Dv;
} NumberIt_;
void  numberit_setAndNext_(NumberIt_ *const nbit, uint32_t const digit, float const deltaX) {
    if(nbit->uvx >= nbit->uvx_end) {
        printerror("Number overflow, cannot add digit %d.", digit);
        return;
    }
    nbit->posX += deltaX;
    *nbit->uvx = (Vector3){{
        (float)(digit % nbit->tex_m) * nbit->Du,
        (float)(digit / nbit->tex_m) * nbit->Dv,
//        (float)(uint32_t)(digit / nbit->tex_m) * nbit->Dv, // Double casting superflu ?
        nbit->posX,
    }};
    nbit->digitCount++;
    nbit->posX += deltaX;
    
    nbit->uvx++;
}
void    number_setTo(Number* const nb, int32_t const newValue) {
    // Pas de changement ?
    if((newValue == nb->value) && nb->_digitCount)
        return;
    // 0. Init
    nb->value = newValue;
    uint32_t const displayedNumber = newValue < 0 ? -newValue : newValue;
    bool const isNegative = nb->value < 0;
    uint32_t const maxDigits = umaxu(uint_highestDecimal(displayedNumber), nb->unitDecimal);
    float const deltaX_def = 0.5*(1.f + nb->digit_x_margin);
    NumberIt_ it = {
        .uvx =      nb->_U0V0Xs,
        .uvx_end = &nb->_U0V0Xs[NUMBER_MAX_DIGITS_],
        .posX = 0,
        .digitCount = 0,
        .tex_m = nb->d.texr.dims.m,
        .Du = nb->d.texr.dims.Du,
        .Dv = nb->d.texr.dims.Dv,
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
    nb->_digitCount = it.digitCount;
    float deltaX = 0.5f*it.posX;
    nb->n.w = 2.f*deltaX;
    for(Vector3 *uvx = nb->_U0V0Xs; uvx < it.uvx; uvx++)
        uvx->z -= deltaX;  // (centrer)
}

void     number_last_setDigitTexture(Texture* const digitTexture) {
    if(!number_last_) { printerror("No last number."); return; }
    textureref2_releaseAndNull(&number_last_->d.texr);
    textureref2_init(&number_last_->d.texr, digitTexture);
    number_last_->n.sx = number_last_->n.sy * number_last_->d.texr.dims.tileRatio;
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
